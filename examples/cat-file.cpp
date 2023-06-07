#include "git2cpp/id_to_str.h"
#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

#include <git2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <iostream>

namespace {

[[noreturn]] void usage(const char * message, const char * arg)
{
    if (message && arg)
        fprintf(stderr, "%s: %s\n", message, arg);
    else if (message)
        fprintf(stderr, "%s\n", message);
    fprintf(stderr, "usage: cat-file (-t | -s | -e | -p) [<options>] <object>\n");
    exit(1);
}

bool check_str_param(const char * arg, const char * pattern, const char ** val)
{
    size_t len = strlen(pattern);
    if (strncmp(arg, pattern, len))
        return false;
    *val = arg + len;
    return true;
}

void print_signature(const char * header, const git_signature * sig)
{
    if (!sig)
        return;

    int offset = sig->when.offset;
    char sign;
    if (offset < 0)
    {
        sign = '-';
        offset = -offset;
    }
    else
    {
        sign = '+';
    }

    const int hours = offset / 60;
    const int minutes = offset % 60;

    printf("%s %s <%s> %ld %c%02d%02d\n",
           header, sig->name, sig->email, (long)sig->when.time,
           sign, hours, minutes);
}

void show_blob(git::Blob const & blob)
{
    /* ? Does this need crlf filtering? */
    fwrite(blob.content(), blob.size(), 1, stdout);
}

void show_tree(git::Tree const & tree)
{
    char oidstr[GIT_OID_SHA1_HEXSIZE + 1];

    for (size_t i = 0, n = tree.entrycount(); i < n; ++i)
    {
        auto te = tree[i];

        git_oid_tostr(oidstr, sizeof(oidstr), &te.id());

        printf("%06o %s %s\t%s\n",
               te.filemode(),
               git_object_type2string(te.type()),
               oidstr, te.name());
    }
}

void show_commit(git::Commit const & commit)
{
    char oidstr[GIT_OID_SHA1_HEXSIZE + 1];

    git_oid_tostr(oidstr, sizeof(oidstr), &commit.tree_id());
    printf("tree %s\n", oidstr);

    for (size_t i = 0, n = commit.parents_num(); i != n; ++i)
    {
        git_oid_tostr(oidstr, sizeof(oidstr), &commit.parent_id(i));
        printf("parent %s\n", oidstr);
    }

    print_signature("author", commit.author());
    print_signature("committer", commit.commiter());

    if (auto message = commit.message())
        printf("\n%s\n", message);
}

void show_tag(git::Tag const & tag)
{
    std::cout   << "object " << git::id_to_str(tag.target_id()) << "\n"
                << "\ntype " << git_object_type2string(tag.target_type())
                << "\ntag "  << tag.name()
                << std::endl;
    print_signature("tagger", tag.tagger());

    if (auto message = tag.message())
        printf("\n%s\n", message);
}

enum class Action
{
    NONE = 0,
    SHOW_TYPE = 1,
    SHOW_SIZE = 2,
    SHOW_NONE = 3,
    SHOW_PRETTY = 4
};

}

int main(int argc, char * argv[])
{
    const char *dir = ".", *rev = nullptr;
    int i, verbose = 0;
    Action action = Action::NONE;
    char oidstr[GIT_OID_SHA1_HEXSIZE + 1];

    for (i = 1; i < argc; ++i)
    {
        char * a = argv[i];

        if (a[0] != '-')
        {
            if (rev)
                usage("Only one rev should be provided", nullptr);
            else
                rev = a;
        }
        else if (!strcmp(a, "-t"))
            action = Action::SHOW_TYPE;
        else if (!strcmp(a, "-s"))
            action = Action::SHOW_SIZE;
        else if (!strcmp(a, "-e"))
            action = Action::SHOW_NONE;
        else if (!strcmp(a, "-p"))
            action = Action::SHOW_PRETTY;
        else if (!strcmp(a, "-q"))
            verbose = 0;
        else if (!strcmp(a, "-v"))
            verbose = 1;
        else if (!strcmp(a, "--help") || !strcmp(a, "-h"))
            usage(nullptr, nullptr);
        else if (!check_str_param(a, "--git-dir=", &dir))
            usage("Unknown option", a);
    }

    if (action == Action::NONE || !rev)
        usage(nullptr, nullptr);

    git::Initializer threads_initializer;

    git::Repository repo(dir);
    git::Object obj = revparse_single(repo, rev);

    if (verbose)
    {
        std::cout
            << git_object_type2string(obj.type())
            << " "
            << git::id_to_str(obj.id())
            << "\n--\n";
    }

    switch (action)
    {
    case Action::SHOW_TYPE:
        printf("%s\n", git_object_type2string(obj.type()));
        break;
    case Action::SHOW_SIZE:
    {
        git::Odb odb = repo.odb();
        git::OdbObject odbobj = odb.read(obj.id());

        printf("%zu\n", odbobj.size());
        break;
    }
    case Action::SHOW_NONE:
        /* just want return result */
        break;
    case Action::SHOW_PRETTY:

        switch (obj.type())
        {
        case GIT_OBJ_BLOB:
            show_blob(obj.to_blob());
            break;
        case GIT_OBJ_COMMIT:
            show_commit(obj.to_commit());
            break;
        case GIT_OBJ_TREE:
            show_tree(obj.to_tree());
            break;
        case GIT_OBJ_TAG:
            show_tag(obj.to_tag());
            break;
        default:
            printf("unknown %s\n", oidstr);
            break;
        }
        break;
    default:
        assert("unexpected action" && static_cast<int>(action));
    }

    return 0;
}
