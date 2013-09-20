#include "repo.h"

extern "C"
{
#include <git2/branch.h>
#include <git2/types.h>
#include <git2/merge.h>
}

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
