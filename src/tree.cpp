#include "git2cpp/tree.h"
#include "git2cpp/error.h"
#include "git2cpp/repo.h"

#include <git2/errors.h>

namespace git
{
    Tree::Tree(git_tree * tree, Repository const & repo)
        : tree_(tree)
        , repo_(&repo)
    {}

    Tree::Tree()
       : tree_(nullptr)
       , repo_(nullptr)
    {}

    Tree::Tree(Tree && other)
        : tree_(other.tree_)
        , repo_(other.repo_)
    {
        other.tree_ = nullptr;
        other.repo_ = nullptr;
    }

    Tree& Tree::operator =(Tree && other)
    {
        tree_ = other.tree_;
        repo_ = other.repo_;
        other.tree_ = nullptr;
        other.repo_ = nullptr;
        return *this;
    }

    Tree::~Tree()
    {
        git_tree_free(tree_);
    }

    int Tree::pathspec_match(uint32_t flags, Pathspec const & ps)
    {
        return git_pathspec_match_tree(NULL, tree_, flags, ps.ptr());
    }

    size_t Tree::entrycount() const
    {
        return git_tree_entrycount(tree_);
    }

    Tree::BorrowedEntry Tree::operator[] (size_t i) const
    {
        return BorrowedEntry(git_tree_entry_byindex(tree_, i));
    }

    Tree::BorrowedEntry Tree::operator[] (std::string const & filename) const
    {
        if (auto entry = git_tree_entry_byname(tree_, filename.c_str()))
            return BorrowedEntry(entry);
        else
            throw file_not_found_error(filename.c_str());
    }

    Tree::OwnedEntry Tree::find(const char * path) const
    {
       git_tree_entry * res;
       const auto status = git_tree_entry_bypath(&res, tree_, path);
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
    {}

    Tree::OwnedEntry::~OwnedEntry()
    {
       git_tree_entry_free(entry_);
    }

    Tree::OwnedEntry::OwnedEntry(OwnedEntry && other)
        : entry_(other.entry_)
        , repo_(other.repo_)
    {
        other.entry_    = nullptr;
        other.repo_     = nullptr;
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
}
