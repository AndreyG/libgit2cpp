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
#include "tag.h"
#include "blob.h"
#include "str_array.h"

namespace git
{
    struct non_existing_branch_error    {};
    struct missing_head_error           {};

    enum class branch_type
    {
        LOCAL, REMOTE, ALL
    };

    struct Repository
    {
        git_repository * ptr() const { return repo_; }

        Commit commit_lookup(git_oid const & oid) const;
        Tree   tree_lookup  (git_oid const & oid) const;
        Tag    tag_lookup   (git_oid const & oid) const;
        Blob   blob_lookup  (git_oid const & oid) const;

        git_oid merge_base(Revspec::Range const & range) const;

        Revspec revparse        (const char * spec) const;
        Revspec revparse_single (const char * spec) const;

        RevWalker rev_walker() const;

        git_status_t file_status(const char * filepath) const;

        Object entry_to_object(git_tree_entry const * entry) const;

        Index   index() const;
        Odb     odb()   const;

        Signature signature() const;

        Status status(git_status_options const &) const;

        StrArray reference_list() const;

        std::vector<std::string> branches(branch_type) const;

        bool is_bare() const;

        Reference head()                    const;
        Reference ref(const char * name)    const;

        int submodule_lookup(git_submodule *&, const char * name) const;
        
        const char * path()     const;
        const char * workdir()  const;

        git_oid create_commit(const char * update_ref,
                              Signature const & author,
                              Signature const & commiter,
                              const char * message_encoding,
                              const char * message,
                              Tree const & tree);

        git_oid create_commit(const char * update_ref,
                              Signature const & author,
                              Signature const & commiter,
                              const char * message_encoding,
                              const char * message,
                              Tree const & tree,
                              Commit const & parent);

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

