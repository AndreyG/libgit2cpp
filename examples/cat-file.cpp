#include <stdio.h>
#include <git2.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "git2cpp/threads_initializer.h"
#include "git2cpp/repo.h"
#include "git2cpp/id_to_str.h"

static void check(int error, const char *message)
{
	if (error) {
		fprintf(stderr, "%s (%d)\n", message, error);
		exit(1);
	}
}

static void usage(const char *message, const char *arg)
{
	if (message && arg)
		fprintf(stderr, "%s: %s\n", message, arg);
	else if (message)
		fprintf(stderr, "%s\n", message);
	fprintf(stderr, "usage: cat-file (-t | -s | -e | -p) [<options>] <object>\n");
	exit(1);
}

static int check_str_param(
	const char *arg, const char *pattern, const char **val)
{
	size_t len = strlen(pattern);
	if (strncmp(arg, pattern, len))
		return 0;
	*val = (const char *)(arg + len);
	return 1;
}

static void print_signature(const char *header, const git_signature *sig)
{
	char sign;
	int offset, hours, minutes;

	if (!sig)
		return;

	offset = sig->when.offset;
	if (offset < 0) {
		sign = '-';
		offset = -offset;
	} else {
		sign = '+';
	}

	hours   = offset / 60;
	minutes = offset % 60;

	printf("%s %s <%s> %ld %c%02d%02d\n",
		   header, sig->name, sig->email, (long)sig->when.time,
		   sign, hours, minutes);
}

static void show_blob(const git_blob *blob)
{
	/* ? Does this need crlf filtering? */
	fwrite(git_blob_rawcontent(blob), git_blob_rawsize(blob), 1, stdout);
}

static void show_tree(const git_tree *tree)
{
	size_t i, max_i = (int)git_tree_entrycount(tree);
	char oidstr[GIT_OID_HEXSZ + 1];
	const git_tree_entry *te;

	for (i = 0; i < max_i; ++i) {
		te = git_tree_entry_byindex(tree, i);

		git_oid_tostr(oidstr, sizeof(oidstr), git_tree_entry_id(te));

		printf("%06o %s %s\t%s\n",
			git_tree_entry_filemode(te),
			git_object_type2string(git_tree_entry_type(te)),
			oidstr, git_tree_entry_name(te));
	}
}

static void show_commit(const git_commit *commit)
{
	unsigned int i, max_i;
	char oidstr[GIT_OID_HEXSZ + 1];

	git_oid_tostr(oidstr, sizeof(oidstr), git_commit_tree_id(commit));
	printf("tree %s\n", oidstr);

	max_i = (unsigned int)git_commit_parentcount(commit);
	for (i = 0; i < max_i; ++i) {
		git_oid_tostr(oidstr, sizeof(oidstr), git_commit_parent_id(commit, i));
		printf("parent %s\n", oidstr);
	}

	print_signature("author", git_commit_author(commit));
	print_signature("committer", git_commit_committer(commit));

	if (git_commit_message(commit))
		printf("\n%s\n", git_commit_message(commit));
}

static void show_tag(const git_tag *tag)
{
	char oidstr[GIT_OID_HEXSZ + 1];

	git_oid_tostr(oidstr, sizeof(oidstr), git_tag_target_id(tag));;
	printf("object %s\n", oidstr);
	printf("type %s\n", git_object_type2string(git_tag_target_type(tag)));
	printf("tag %s\n", git_tag_name(tag));
	print_signature("tagger", git_tag_tagger(tag));

	if (git_tag_message(tag))
		printf("\n%s\n", git_tag_message(tag));
}

enum {
	SHOW_TYPE = 1,
	SHOW_SIZE = 2,
	SHOW_NONE = 3,
	SHOW_PRETTY = 4
};

int main(int argc, char *argv[])
{
	const char *dir = ".", *rev = NULL;
	int i, action = 0, verbose = 0;
	char oidstr[GIT_OID_HEXSZ + 1];

	for (i = 1; i < argc; ++i) {
		char *a = argv[i];

		if (a[0] != '-') {
			if (rev != NULL)
				usage("Only one rev should be provided", NULL);
			else
				rev = a;
		}
		else if (!strcmp(a, "-t"))
			action = SHOW_TYPE;
		else if (!strcmp(a, "-s"))
			action = SHOW_SIZE;
		else if (!strcmp(a, "-e"))
			action = SHOW_NONE;
		else if (!strcmp(a, "-p"))
			action = SHOW_PRETTY;
		else if (!strcmp(a, "-q"))
			verbose = 0;
		else if (!strcmp(a, "-v"))
			verbose = 1;
		else if (!strcmp(a, "--help") || !strcmp(a, "-h"))
			usage(NULL, NULL);
		else if (!check_str_param(a, "--git-dir=", &dir))
			usage("Unknown option", a);
	}

	if (!action || !rev)
		usage(NULL, NULL);

    git::ThreadsInitializer threads_initializer;

    git::Repository repo(dir);
	git::Object obj = revparse_single(repo, rev);

	if (verbose) {
        std::cout 
            << git_object_type2string(obj.type()) 
            << " " 
            << git::id_to_str(obj.id()) 
            << "\n--\n"; 
	}

	switch (action) {
	case SHOW_TYPE:
		printf("%s\n", git_object_type2string(obj.type()));
		break;
	case SHOW_SIZE: 
        {
            git::Odb odb = repo.odb();
            git::OdbObject odbobj = odb.read(obj.id()); 

            printf("%ld\n", (long)odbobj.size());
		}
		break;
	case SHOW_NONE:
		/* just want return result */
		break;
	case SHOW_PRETTY:

		switch (obj.type()) {
		case GIT_OBJ_BLOB:
			show_blob(obj.as_blob());
			break;
		case GIT_OBJ_COMMIT:
			show_commit(obj.as_commit());
			break;
		case GIT_OBJ_TREE:
			show_tree(obj.as_tree());
			break;
		case GIT_OBJ_TAG:
			show_tag(obj.as_tag());
			break;
		default:
			printf("unknown %s\n", oidstr);
			break;
		}
		break;
	}

	return 0;
}
