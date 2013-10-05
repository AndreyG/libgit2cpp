#pragma once

#include <string>
#include <vector>

extern "C"
{
#include <git2/repository.h>
}

#include "commit.h"
#include "index.h"
#include "odb.h"
#include "revspec.h"
#include "status.h"
#include "reference.h"
#include "signature.h"
#include "revwalker.h"

namespace git
{
    struct non_existing_branch_error    {};
    struct missing_head_error           {};

    struct Repository
    {
        git_repository * ptr() { return repo_; }

        Commit commit_lookup(git_oid const * oid) const;
        Tree   tree_lookup  (git_oid const * oid) const;

        git_oid merge_base(Revspec::Range const & range) const;

        Revspec revparse        (const char * spec) const;
        Revspec revparse_single (const char * spec) const;

        RevWalker rev_walker() const;

        git_status_t file_status(const char * filepath) const;

        Index   index() const;
        Odb     odb()   const;

        Signature signature() const;

        Status status(git_status_options const &) const;

        std::vector<std::string> branches() const;

        bool is_bare() const;

        Reference head() const;

        int submodule_lookup(git_submodule *&, const char * name) const;
        
        const char * path()     const;
        const char * workdir()  const;

        git_oid create_commit(const char * update_ref,
                              Signature const & author,
                              Signature const & commiter,
                              const char * message_encoding,
                              const char * message,
                              Tree const & tree,
                              int parent_count);

        explicit Repository(std::string const & dir);

        struct init_tag {};
        static init_tag init;
        Repository(std::string const & dir, init_tag);
        Repository(std::string const & dir, init_tag, 
                   git_repository_init_options opts);

        Repository              (Repository const &) = delete;
        Repository& operator =  (Repository const &) = delete; 

        Repository(Repository &&);

        ~Repository();

    private:
        git_repository * repo_;
    };

    Object revparse_single(Repository const & repo, const char * spec);
}

