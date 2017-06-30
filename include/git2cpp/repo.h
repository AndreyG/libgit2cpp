#pragma once

#include <string>
#include <vector>

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
#include "diff.h"
#include "submodule.h"

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
        Commit commit_lookup(git_oid const & oid) const;
        Tree   tree_lookup  (git_oid const & oid) const;
        Tag    tag_lookup   (git_oid const & oid) const;
        Blob   blob_lookup  (git_oid const & oid) const;

        git_oid merge_base(Revspec::Range const & range)       const;
        git_oid merge_base(git_oid const &, git_oid const &)   const;

        Revspec revparse        (const char * spec) const;
        Revspec revparse_single (const char * spec) const;

        RevWalker rev_walker() const;

        git_status_t file_status(const char * filepath) const;

        Object entry_to_object(Tree::OwnedEntry)    const;
        Object entry_to_object(Tree::BorrowedEntry) const;

        Index   index() const;
        Odb     odb()   const;

        Diff diff                       (Tree &, Tree &,    git_diff_options const &) const;
        Diff diff_to_index              (Tree &,            git_diff_options const &) const;
        Diff diff_to_workdir            (Tree &,            git_diff_options const &) const;
        Diff diff_to_workdir_with_index (Tree &,            git_diff_options const &) const;
        Diff diff_index_to_workdir      (                   git_diff_options const &) const;

        Signature signature() const;

        Status status(Status::Options const &) const;

        StrArray reference_list() const;

        std::vector<std::string> branches(branch_type) const;

        bool is_bare() const;

        Reference head()                    const;
        Reference ref(const char * name)    const;

        Submodule submodule_lookup(const char * name) const;
        
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

        void reset_default(Commit const &, git_strarray const & pathspecs);

        explicit Repository(const char * dir);
        explicit Repository(std::string const & dir);

        struct init_tag {};
        static const init_tag init;
        Repository(const char * dir, init_tag);
        Repository(const char * dir, init_tag, git_repository_init_options opts);

        Repository              (Repository const &) = delete;
        Repository& operator =  (Repository const &) = delete; 

        Repository              (Repository &&) noexcept;
        Repository& operator =  (Repository &&) noexcept;

        ~Repository();

    private:
        git_repository * repo_;
    };

    Object revparse_single(Repository const & repo, const char * spec);
}

