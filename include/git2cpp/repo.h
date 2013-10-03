#pragma once

#include <string>
#include <cassert>
#include <memory>

#include "commit.h"
#include "revwalker.h"
#include "index.h"

extern "C"
{
#include <git2/revparse.h>
#include <git2/status.h>
}

namespace git
{
    struct Repository
    {
        void commit_lookup_c_style(git_commit *& commit, git_oid const & oid) const
        {
            assert(git_commit_lookup(&commit, repo_, &oid) == 0);
        }

        Commit commit_lookup(git_oid const & oid) const
        {
            git_commit * commit;
            commit_lookup_c_style(commit, oid);
            return Commit(commit);
        }

        int object_lookup(git_object *& object, git_oid const & oid, git_otype type) const 
        {
            return git_object_lookup(&object, repo_, &oid, type);
        }

        int merge_base(git_oid & out, git_oid const * one, git_oid const * two) const;

        int revparse(git_revspec & out, const char * spec) const;
        int revparse_single(git_object *& out, const char * spec) const;

        std::shared_ptr<RevWalker> rev_walker() const
        {
            git_revwalk * out;
            git_revwalk_new(&out, repo_);
            return std::make_shared<RevWalker>(out);
        }

        git_status_t file_status(const char * filepath) const;

        Index index() const;

        std::vector<std::string> branches() const;
        
        explicit Repository(std::string const & dir);

        Repository              (Repository const &) = delete;
        Repository& operator =  (Repository const &) = delete; 

        ~Repository();

    private:
        struct git_repository * repo_;
    };
}

