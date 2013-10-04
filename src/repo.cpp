extern "C"
{
#include <git2/repository.h>
#include <git2/branch.h>
#include <git2/types.h>
#include <git2/merge.h>
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

    Repository::~Repository()
    {
        git_repository_free(repo_);
    }

    Commit Repository::commit_lookup(git_oid const * oid) const
    {
        git_commit * commit;
        if (git_commit_lookup(&commit, repo_, oid))
            throw commit_lookup_error(oid);
        return Commit(commit);
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

    Object revparse_single(Repository const & repo, const char * spec)
    {
        return std::move(*repo.revparse_single(spec).single());
    }
}
