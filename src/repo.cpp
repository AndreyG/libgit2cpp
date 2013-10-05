extern "C"
{
#include <git2/branch.h>
#include <git2/types.h>
#include <git2/merge.h>
#include <git2/submodule.h>
#include <git2/errors.h>
}

#include "git2cpp/repo.h"
#include "git2cpp/error.h"
#include "git2cpp/revwalker.h"

namespace git
{
    namespace
    {
        int write_branch_name(const char * name, git_branch_t, void * payload)
        {
            std::vector<std::string> * out = reinterpret_cast<std::vector<std::string> *>(payload);
            out->emplace_back(name);
            return 0;
        }
    }

    Repository::Repository(std::string const & dir)
    {
        if (git_repository_open_ext(&repo_, dir.c_str(), 0, NULL))
            throw repository_open_error();
    }

    Repository::Repository(std::string const & dir, init_tag)
    {
        if (git_repository_init(&repo_, dir.c_str(), 0) < 0)
            throw repository_init_error(dir);
    }

    Repository::Repository(std::string const & dir, init_tag, 
                           git_repository_init_options opts)
    {
        if (git_repository_init_ext(&repo_, dir.c_str(), &opts) < 0)
            throw repository_init_error(dir);
    }

    Repository::~Repository()
    {
        git_repository_free(repo_);
    }

    bool Repository::is_bare() const
    {
        return git_repository_is_bare(repo_);
    }

    Signature Repository::signature() const
    {
        return Signature(repo_);
    }

    Commit Repository::commit_lookup(git_oid const * oid) const
    {
        git_commit * commit;
        if (git_commit_lookup(&commit, repo_, oid))
            throw commit_lookup_error(oid);
        return Commit(commit);
    }

    Tree Repository::tree_lookup(git_oid const * oid) const
    {
        git_tree * tree;
        if (git_tree_lookup(&tree, repo_, oid))
            throw tree_lookup_error(oid);
        return Tree(tree);
    }

    Revspec Repository::revparse(const char * spec) const
    {
        git_revspec revspec;
        if (git_revparse(&revspec, repo_, spec))
            throw revparse_error(spec);
        return Revspec(revspec);
    }

    Revspec Repository::revparse_single(const char * spec) const
    {
        git_object * obj;
        if (git_revparse_single(&obj, repo_, spec) < 0)
            throw revparse_error(spec);
        return Revspec(obj);
    }

    Index Repository::index() const
    {
        return Index(repo_);
    }

    Odb Repository::odb() const
    {
        return Odb(repo_);
    }

    git_status_t Repository::file_status(const char * filepath) const
    {
        git_status_t res;
        switch (git_status_file(reinterpret_cast<unsigned int *>(&res), repo_, filepath))
        {
        case GIT_ENOTFOUND:
            throw file_not_found_error(filepath);
        case GIT_EAMBIGUOUS:
            throw ambiguous_path_error(filepath);
        case -1:
            throw unknown_file_status_error(filepath);
        }

        return res;
    }

    std::vector<std::string> Repository::branches() const
    {
        std::vector<std::string> res;
        git_branch_foreach  ( repo_
                            , GIT_BRANCH_LOCAL | GIT_BRANCH_REMOTE
                            , write_branch_name
                            , &res
                            );
        return res;
    }

    std::unique_ptr<RevWalker> Repository::rev_walker() const
    {
        return std::unique_ptr<RevWalker>(new RevWalker(repo_));
    }

    int Repository::merge_base(git_oid & out, git_oid const * one, git_oid const * two) const
    {
        return git_merge_base(&out, repo_, one, two);
    }

    Reference Repository::head() const
    {
        git_reference * head;
        auto error = git_repository_head(&head, repo_);
        if (error == GIT_EUNBORNBRANCH)
            throw non_existing_branch_error();
        else if (error == GIT_ENOTFOUND)
            throw missing_head_error();
        else if (error)
            throw unknown_get_current_branch_error();
        else
            return Reference(head);
    }

    Status Repository::status(git_status_options const & opts) const
    {
        return Status(repo_, opts);
    }

    int Repository::submodule_lookup(git_submodule *& sm, const char * name) const
    {
        return git_submodule_lookup(&sm, repo_, name);
    }

    const char * Repository::path() const
    {
        return git_repository_path(repo_);
    }

    const char * Repository::workdir() const
    {
        return git_repository_workdir(repo_);
    }

    git_oid Repository::create_commit(const char * update_ref,
                                      Signature const & author,
                                      Signature const & commiter,
                                      const char * message_encoding,
                                      const char * message,
                                      Tree const & tree,
                                      int parent_count)
    {
        git_oid res;
        if (git_commit_create_v(&res, repo_, update_ref, 
                                author.ptr(), commiter.ptr(),
                                message_encoding, message,
                                tree.ptr(), parent_count))
        {
            throw commit_create_error();
        }
        return res;
    }

    Object revparse_single(Repository const & repo, const char * spec)
    {
        return std::move(*repo.revparse_single(spec).single());
    }
}
