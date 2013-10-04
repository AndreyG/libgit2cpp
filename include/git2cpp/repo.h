#pragma once

#include <string>
#include <vector>
#include <memory>

#include "commit.h"
#include "index.h"
#include "odb.h"
#include "revspec.h"
#include "status.h"
#include "reference.h"

struct git_repository;

namespace git
{
    struct non_existing_branch_error    {};
    struct missing_head_error           {};

    struct Repository
    {
        git_repository * ptr() { return repo_; }

        Commit commit_lookup(git_oid const * oid) const;

        int merge_base(git_oid & out, git_oid const * one, git_oid const * two) const;

        Revspec revparse        (const char * spec) const;
        Revspec revparse_single (const char * spec) const;

        std::unique_ptr<struct RevWalker> rev_walker() const;

        git_status_t file_status(const char * filepath) const;

        Index   index() const;
        Odb     odb()   const;

        Status status(git_status_options const &) const;

        std::vector<std::string> branches() const;

        bool is_bare() const;

        Reference head() const;

        int submodule_lookup(git_submodule *&, const char * name) const;
        
        explicit Repository(std::string const & dir);

        Repository              (Repository const &) = delete;
        Repository& operator =  (Repository const &) = delete; 

        ~Repository();

    private:
        git_repository * repo_;
    };

    Object revparse_single(Repository const & repo, const char * spec);
}

