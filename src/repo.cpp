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

    int Repository::revparse(git_revspec & out, const char * spec) const
    {
        return git_revparse(&out, repo_, spec);
    }

    int Repository::revparse_single(git_object *& out, const char * spec) const
    {
        return git_revparse_single(&out, repo_, spec);
    }

    Index Repository::index() const
    {
        return Index(repo_);
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

    int Repository::merge_base(git_oid & out, git_oid const * one, git_oid const * two) const
    {
        return git_merge_base(&out, repo_, one, two);
    }
}
