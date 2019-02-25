#include "git2cpp/tree.h"
#include "git2cpp/error.h"
#include "git2cpp/repo.h"

#include <git2/errors.h>

namespace git
{
    Tree::Tree(git_tree * tree, Repository const & repo)
        : tree_(tree)
        , repo_(&repo)
    {
    }

    void Tree::Destroy::operator()(git_tree* tree) const
    {
        git_tree_free(tree);
    }

    int Tree::pathspec_match(uint32_t flags, Pathspec const & ps)
    {
        return git_pathspec_match_tree(nullptr, ptr(), flags, ps.ptr());
    }

    size_t Tree::entrycount() const
    {
        return git_tree_entrycount(ptr());
    }

    Tree::BorrowedEntry Tree::operator[](size_t i) const
    {
        return BorrowedEntry(git_tree_entry_byindex(ptr(), i));
    }

    Tree::BorrowedEntry Tree::operator[](std::string const & filename) const
    {
        if (auto entry = git_tree_entry_byname(ptr(), filename.c_str()))
            return BorrowedEntry(entry);
        else
            throw file_not_found_error(filename.c_str());
    }

    Tree::OwnedEntry Tree::find(const char * path) const
    {
        git_tree_entry * res;
        const auto status = git_tree_entry_bypath(&res, ptr(), path);
        switch (status)
        {
        case GIT_OK:
            return OwnedEntry(res, *repo_);
        case GIT_ENOTFOUND:
            throw file_not_found_error(path);
        default:
            throw error_t(internal::format("unknown error inside function: 'git_tree_entry_bypath': %d", status));
        }
    }

    Tree::OwnedEntry::OwnedEntry(git_tree_entry * entry, Repository const & repo)
        : entry_(entry)
        , repo_(&repo)
    {
    }

    void Tree::OwnedEntry::Destroy::operator()(git_tree_entry* entry) const
    {
        git_tree_entry_free(entry);
    }

    Tree Tree::OwnedEntry::to_tree() /* && */
    {
        auto const & repo = *repo_;
        return repo.entry_to_object(std::move(*this)).to_tree();
    }

    const char * Tree::BorrowedEntry::name() const
    {
        return git_tree_entry_name(entry_);
    }

    git_oid const & Tree::BorrowedEntry::id() const
    {
        return *git_tree_entry_id(entry_);
    }

    git_otype Tree::BorrowedEntry::type() const
    {
        return git_tree_entry_type(entry_);
    }

    git_filemode_t Tree::BorrowedEntry::filemode() const
    {
        return git_tree_entry_filemode(entry_);
    }
}
