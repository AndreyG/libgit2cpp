#include "git2cpp/repo.h"

#include "git2cpp/annotated_commit.h"
#include "git2cpp/error.h"
#include "git2cpp/internal/optional.h"

#include <git2/blame.h>
#include <git2/blob.h>
#include <git2/branch.h>
#include <git2/commit.h>
#include <git2/errors.h>
#include <git2/merge.h>
#include <git2/reset.h>
#include <git2/revwalk.h>
#include <git2/submodule.h>
#include <git2/tag.h>
#include <git2/types.h>

#include <cassert>

namespace git
{
    const Repository::init_tag Repository::init;

    Repository::Repository(const char * dir)
    {
        git_repository * repo;
        if (git_repository_open_ext(&repo, dir, 0, nullptr))
            throw repository_open_error(dir);
        repo_.reset(repo);
    }

    Repository::Repository(std::string const & dir)
        : Repository(dir.c_str())
    {
    }

    Repository::Repository(const char * dir, init_tag)
    {
        git_repository * repo;
        if (git_repository_init(&repo, dir, false) < 0)
            throw repository_init_error(dir);
        repo_.reset(repo);
    }

    Repository::Repository(std::string const & dir, init_tag tag)
        : Repository(dir.c_str(), tag)
    {}

    Repository::Repository(const char * dir, init_tag, git_repository_init_options opts)
    {
        git_repository * repo;
        if (git_repository_init_ext(&repo, dir, &opts) < 0)
            throw repository_init_error(dir);
        repo_.reset(repo);
    }

    bool Repository::is_bare() const
    {
        return git_repository_is_bare(repo_.get()) != 0;
    }

    Signature Repository::signature() const
    {
        return Signature(repo_.get());
    }

    Commit Repository::commit_lookup(git_oid const & oid) const
    {
        git_commit * commit;
        if (git_commit_lookup(&commit, repo_.get(), &oid))
            throw commit_lookup_error(oid);
        else
            return {commit, *this};
    }

    Tree Repository::tree_lookup(git_oid const & oid) const
    {
        git_tree * tree;
        if (git_tree_lookup(&tree, repo_.get(), &oid))
            throw tree_lookup_error(oid);
        else
            return {tree, *this};
    }

    Tag Repository::tag_lookup(git_oid const & oid) const
    {
        git_tag * tag;
        if (git_tag_lookup(&tag, repo_.get(), &oid))
            throw tag_lookup_error(oid);
        else
            return Tag(tag);
    }

    Blob Repository::blob_lookup(git_oid const & oid) const
    {
        git_blob * blob;
        if (git_blob_lookup(&blob, repo_.get(), &oid))
            throw blob_lookup_error(oid);
        else
            return Blob(blob);
    }

    Revspec Repository::revparse(const char * spec) const
    {
        git_revspec revspec;
        if (git_revparse(&revspec, repo_.get(), spec))
            throw revparse_error(spec);
        else
            return {revspec, *this};
    }

    Revspec Repository::revparse_single(const char * spec) const
    {
        git_object * obj;
        if (git_revparse_single(&obj, repo_.get(), spec) < 0)
            throw revparse_error(spec);
        return Revspec(obj, *this);
    }

    Index Repository::index() const
    {
        return Index(repo_.get());
    }

    Odb Repository::odb() const
    {
        return Odb(repo_.get());
    }

    git_status_t Repository::file_status(const char * filepath) const
    {
        static_assert(sizeof(git_status_t) == sizeof(unsigned int), "sizeof(git_status_t) != sizeof(unsigned int)");
        git_status_t res;
        switch (git_status_file(reinterpret_cast<unsigned int *>(&res), repo_.get(), filepath))
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
        branch_iterator(git_repository * repo, branch_type type)
            : type_(convert(type))
        {
            git_branch_iterator_new(&base_, repo, type_);
            ++(*this);
        }

        ~branch_iterator()
        {
            git_branch_iterator_free(base_);
        }

        explicit operator bool() const { return internal::has_value(ref_); }

        void operator++()
        {
            git_branch_t type;
            git_reference * ref;
            switch (git_branch_next(&ref, &type, base_))
            {
            case GIT_OK:
                assert(type & type_);
                internal::emplace(ref_, ref);
                break;
            case GIT_ITEROVER:
                ref_ = internal::none;
                break;
            default:
                ref_ = internal::none;
                throw std::logic_error("unknown git_branch_next error");
            }
        }

        Reference & operator*()
        {
            return *ref_;
        }

    private:
        static git_branch_t convert(branch_type t)
        {
            switch (t)
            {
            case branch_type::LOCAL:
                return GIT_BRANCH_LOCAL;
            case branch_type::REMOTE:
                return GIT_BRANCH_REMOTE;
            case branch_type::ALL:
                return GIT_BRANCH_ALL;
            }
            throw std::logic_error("invalid branch type");
        }

    private:
        git_branch_t type_;
        git_branch_iterator * base_;
        internal::optional<Reference> ref_;
    };

    std::vector<Reference> Repository::branches(branch_type type) const
    {
        std::vector<Reference> res;
        for (branch_iterator it(repo_.get(), type); it; ++it)
            res.emplace_back(std::move(*it));
        return res;
    }

    Reference Repository::create_branch(const char * name, Commit const & target, bool force)
    {
        git_reference * ref;
        const auto err = git_branch_create(&ref, repo_.get(), name, target.ptr(), force);
        switch (err)
        {
        case GIT_OK:
            return Reference(ref);
        case GIT_EEXISTS:
            assert(!force);
            throw branch_create_error(branch_create_error::already_exists);
        case GIT_EINVALIDSPEC:
            throw branch_create_error(branch_create_error::invalid_spec);
        default:
            throw branch_create_error(branch_create_error::unknown);
        }
    }

    Reference Repository::dwim(const char* shorthand) const
    {
        git_reference * ref;
        if (git_reference_dwim(&ref, repo_.get(), shorthand) == GIT_OK)
            return Reference(ref);
        else
            return Reference();
    }

    AnnotatedCommit Repository::annotated_commit_from_ref(Reference const& ref) const
    {
        git_annotated_commit * commit;
        const auto err = git_annotated_commit_from_ref(&commit, repo_.get(), ref.ptr());
        assert(err == GIT_OK);
        return AnnotatedCommit(commit);
    }

    AnnotatedCommit Repository::annotated_commit_lookup(git_oid const& id) const
    {
        git_annotated_commit * commit;
        const auto err = git_annotated_commit_lookup(&commit, repo_.get(), &id);
        assert(err == GIT_OK);
        return AnnotatedCommit(commit);
    }

    RevWalker Repository::rev_walker() const
    {
        git_revwalk * walker;
        if (git_revwalk_new(&walker, repo_.get()))
            throw revwalk_new_error();
        else
            return {walker, *this};
    }

    git_oid Repository::merge_base(git_oid const & a, git_oid const & b) const
    {
        git_oid res;
        if (git_merge_base(&res, repo_.get(), &a, &b))
            throw merge_base_error(a, b);
        return res;
    }

    git_oid Repository::merge_base(Revspec::Range const & range) const
    {
        return merge_base(range.from.id(), range.to.id());
    }

    Reference Repository::head() const
    {
        git_reference * head;
        switch (git_repository_head(&head, repo_.get()))
        {
        case GIT_OK:
            return Reference(head);
        case GIT_EUNBORNBRANCH:
            throw non_existing_branch_error();
        case GIT_ENOTFOUND:
            throw missing_head_error();
        default:
            throw unknown_get_current_branch_error();
        }
    }

    Reference Repository::ref(const char * name) const
    {
        git_reference * ref;
        git_reference_lookup(&ref, repo_.get(), name);
        return Reference(ref);
    }

    Status Repository::status(Status::Options const & opts) const
    {
        return Status(repo_.get(), opts);
    }

    git_repository_state_t Repository::state() const
    {
        return static_cast<git_repository_state_t>(git_repository_state(repo_.get()));
    }

    StrArray Repository::reference_list() const
    {
        git_strarray str_array;
        git_reference_list(&str_array, repo_.get());
        return StrArray(str_array);
    }

    Submodule Repository::submodule_lookup(const char * name) const
    {
        git_submodule * sm;
        if (git_submodule_lookup(&sm, repo_.get(), name))
            throw submodule_lookup_error(name);
        else
            return {sm, repo_.get()};
    }

    const char * Repository::path() const
    {
        return git_repository_path(repo_.get());
    }

    const char * Repository::workdir() const
    {
        return git_repository_workdir(repo_.get());
    }

    git_oid Repository::create_commit(const char * update_ref,
                                      Signature const & author,
                                      Signature const & commiter,
                                      const char * message,
                                      Tree const & tree,
                                      const char * message_encoding)
    {
        git_oid res;
        if (git_commit_create_v(&res, repo_.get(), update_ref,
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
                                      const char * message,
                                      Tree const & tree,
                                      Commit const & parent,
                                      const char * message_encoding)
    {
        git_oid res;
        if (git_commit_create_v(&res, repo_.get(), update_ref,
                                author.ptr(), commiter.ptr(),
                                message_encoding, message,
                                tree.ptr(), 1, parent.ptr()))
        {
            throw commit_create_error();
        }
        return res;
    }

    namespace
    {
        Object tree_entry_to_object(git_tree_entry const * entry, Repository const & repo, git_repository * repo_ptr)
        {
            git_object * obj;
            git_tree_entry_to_object(&obj, repo_ptr, entry);
            return Object(obj, repo);
        }
    }

    Object Repository::entry_to_object(Tree::BorrowedEntry entry) const
    {
        return tree_entry_to_object(entry.ptr(), *this, repo_.get());
    }

    Object Repository::entry_to_object(Tree::OwnedEntry entry) const
    {
        return tree_entry_to_object(entry.ptr(), *this, repo_.get());
    }

    Object revparse_single(Repository const & repo, const char * spec)
    {
        return std::move(*repo.revparse_single(spec).single());
    }

    Diff Repository::diff(Tree & a, Tree & b, git_diff_options const & opts) const
    {
        git_diff * diff;
        auto op_res = git_diff_tree_to_tree(&diff, repo_.get(), a.ptr(), b.ptr(), &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    Diff Repository::diff_to_index(Tree & t, git_diff_options const & opts) const
    {
        git_diff * diff;
        auto op_res = git_diff_tree_to_index(&diff, repo_.get(), t.ptr(), nullptr, &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    Diff Repository::diff_to_workdir(Tree & t, git_diff_options const & opts) const
    {
        git_diff * diff;
        auto op_res = git_diff_tree_to_workdir(&diff, repo_.get(), t.ptr(), &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    Diff Repository::diff_to_workdir_with_index(Tree & t, git_diff_options const & opts) const
    {
        git_diff * diff;
        auto op_res = git_diff_tree_to_workdir_with_index(&diff, repo_.get(), t.ptr(), &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    Diff Repository::diff_index_to_workdir(git_diff_options const & opts) const
    {
        git_diff * diff;
        auto op_res = git_diff_index_to_workdir(&diff, repo_.get(), nullptr, &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    void Repository::reset_default(Commit const & commit, git_strarray const & pathspecs)
    {
        auto op_res = git_reset_default(repo_.get(), reinterpret_cast<git_object *>(const_cast<git_commit *>(commit.ptr())), const_cast<git_strarray *>(&pathspecs));
        assert(op_res == GIT_OK);
    }

    void Repository::file_diff(std::string const & old_path, git_oid const & old_id,
                               std::string const & new_path, git_oid const & new_id,
                               FileDiffHandler & diff_handler) const
    {
        auto old_file = blob_lookup(old_id);
        auto new_file = blob_lookup(new_id);
        git_diff_options options = GIT_DIFF_OPTIONS_INIT;
        auto data_callback = [](git_diff_delta const *, git_diff_hunk const *, git_diff_line const * line, void * payload) {
            auto handler = reinterpret_cast<FileDiffHandler *>(payload);
            handler->line(*line);
            return 0;
        };
        auto op_res = git_diff_blobs(old_file.ptr(), old_path.c_str(), new_file.ptr(), new_path.c_str(),
                                     &options, nullptr, nullptr, nullptr, data_callback, &diff_handler);
        assert(op_res == GIT_OK);
    }

    StrArray Repository::remotes() const
    {
        git_strarray result;
        auto op_res = git_remote_list(&result, repo_.get());
        assert(op_res == GIT_OK);
        return StrArray(result);
    }

    Remote Repository::remote(const char * name) const
    {
        git_remote * remote;
        if (git_remote_lookup(&remote, repo_.get(), name))
            throw remote_lookup_error(name);
        else
            return Remote(remote);
    }

    Remote Repository::create_remote(const char * name, const char * url)
    {
        git_remote * remote;
        if (git_remote_create(&remote, repo_.get(), name, url))
            throw remote_create_error(name, url);
        else
            return Remote(remote);
    }

    void Repository::delete_remote(const char * name)
    {
        if (git_remote_delete(repo_.get(), name))
            throw remote_delete_error(name);
    }

    internal::optional<StrArray> Repository::rename_remote(const char * old_name, const char * new_name)
    {
        git_strarray problems {};
        if (git_remote_rename(&problems, repo_.get(), old_name, new_name))
            return StrArray(problems);
        else
            return internal::none;
    }

    void Repository::set_url(const char * name, const char * url)
    {
        if (git_remote_set_url(repo_.get(), name, url))
            throw remote_set_url_error(name, url);
    }

    void Repository::set_pushurl(const char * name, const char * url)
    {
        if (git_remote_set_pushurl(repo_.get(), name, url))
            throw remote_set_pushurl_error(name, url);
    }

    int Repository::checkout_tree(Commit const& commit, git_checkout_options const& options)
    {
        return git_checkout_tree(repo_.get(), reinterpret_cast<git_object const *>(commit.ptr()), &options);
    }

    int Repository::set_head(char const* ref)
    {
        return git_repository_set_head(repo_.get(), ref);
    }

    int Repository::set_head_detached(AnnotatedCommit const& commit)
    {
        return git_repository_set_head_detached_from_annotated(repo_.get(), commit.ptr());
    }

    Blame Repository::blame_file(const char* path, git_blame_options const& options)
    {
        git_blame * blame;
        const auto err = git_blame_file(
            &blame, repo_.get(), path,
            /*IMO `options` can be const, git_blame_file doesn't change it*/ const_cast<git_blame_options*>(&options)
            );
        if (err != GIT_OK)
            throw blame_file_error(path);
        return Blame(blame);
    }

    internal::optional<std::string> Repository::discover(const char * start_path)
    {
        git_buf buf = GIT_BUF_INIT_CONST(nullptr, 0);
        if (git_repository_discover(&buf, start_path, 0, nullptr))
            return internal::none;
        return std::string(buf.ptr, buf.size);
    }

    void Repository::Destroy::operator() (git_repository* repo) const
    {
        git_repository_free(repo);
    }
}
