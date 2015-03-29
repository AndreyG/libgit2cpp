extern "C"
{
#include <git2/commit.h>
#include <git2/branch.h>
#include <git2/types.h>
#include <git2/merge.h>
#include <git2/submodule.h>
#include <git2/errors.h>
}

#include <boost/optional.hpp>

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
        return Commit(oid, *this);
    }

    Tree Repository::tree_lookup(git_oid const * oid) const
    {
        return Tree(oid, repo_);
    }

    Tag Repository::tag_lookup(git_oid const * oid) const
    {
        return Tag(oid, *this);
    }

    Blob Repository::blob_lookup(git_oid const * oid) const
    {
        return Blob(oid, repo_);
    }

    Revspec Repository::revparse(const char * spec) const
    {
        git_revspec revspec;
        if (git_revparse(&revspec, repo_, spec))
            throw revparse_error(spec);
        return Revspec(revspec, *this);
    }

    Revspec Repository::revparse_single(const char * spec) const
    {
        git_object * obj;
        if (git_revparse_single(&obj, repo_, spec) < 0)
            throw revparse_error(spec);
        return Revspec(obj, *this);
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

    struct branch_iterator
    {
        branch_iterator(Repository const & repo, branch_type type)
            : type_(convert(type))
        {
            git_branch_iterator_new(&base_, repo.ptr(), type_);
            ++(*this);
        }

        ~branch_iterator()
        {
            git_branch_iterator_free(base_);
        }

        explicit operator bool () const { return ref_.is_initialized(); }

        void operator ++ ()
        {
            git_branch_t type;
            git_reference * ref;
            if (git_branch_next(&ref, &type, base_) == 0)
            {
                assert(type == type_);
                ref_ = boost::in_place(ref);
            }
        }

        Reference const * operator -> () const
        {
            return ref_.get_ptr();
        }

    private:
        git_branch_t convert(branch_type t) const
        {
            switch (t)
            {
            case branch_type::LOCAL :   return GIT_BRANCH_LOCAL;
            case branch_type::REMOTE:   return GIT_BRANCH_REMOTE;
            case branch_type::ALL:      return GIT_BRANCH_REMOTE;
            default:
                throw std::logic_error("invalid branch type");
            }
        }

    private:
        git_branch_t type_;
        git_branch_iterator * base_;
        boost::optional<Reference> ref_;
    };

    std::vector<std::string> Repository::branches(branch_type type) const
    {
        std::vector<std::string> res;
        for (branch_iterator it(*this, type); it; ++it)
            it->name();
        return res;
    }

    RevWalker Repository::rev_walker() const
    {
        return RevWalker(repo_);
    }

    git_oid Repository::merge_base(Revspec::Range const & range) const
    {
        git_oid res;
        if (git_merge_base(&res, repo_, range.from.id(), range.to.id()))
            throw merge_base_error();
        return res;
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

    Reference Repository::ref(const char * name) const
    {
        git_reference * ref;
        git_reference_lookup(&ref, repo_, name);
        return Reference(ref);
    }

    Status Repository::status(git_status_options const & opts) const
    {
        return Status(repo_, opts);
    }

    StrArray Repository::reference_list() const
    {
        git_strarray str_array;
        git_reference_list(&str_array, repo_);
        return StrArray(str_array);
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
                                      Tree const & tree)
    {
        git_oid res;
        if (git_commit_create_v(&res, repo_, update_ref, 
                                author.ptr(), commiter.ptr(),
                                message_encoding, message,
                                tree.ptr(), 0))
        {
            throw commit_create_error();
        }
        return res;
    }

    git_oid Repository::create_commit(const char * update_ref,
                                      Signature const & author,
                                      Signature const & commiter,
                                      const char * message_encoding,
                                      const char * message,
                                      Tree const & tree,
                                      Commit const & parent)
    {
        git_oid res;
        if (git_commit_create_v(&res, repo_, update_ref,
                                author.ptr(), commiter.ptr(),
                                message_encoding, message,
                                tree.ptr(), 1, parent.ptr()))
        {
            throw commit_create_error();
        }
        return res;
    }

    Object Repository::entry_to_object(git_tree_entry const * entry) const
    {
        git_object * obj;
        git_tree_entry_to_object(&obj, repo_, entry);
        return Object(obj, *this);
    }

    Object revparse_single(Repository const & repo, const char * spec)
    {
        return std::move(*repo.revparse_single(spec).single());
    }
}
