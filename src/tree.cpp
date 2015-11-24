#include "git2cpp/tree.h"
#include "git2cpp/error.h"
#include "git2cpp/repo.h"

#include <git2/errors.h>

namespace git
{
    Tree::Tree(git_oid const & oid, Repository const & repo)
       : repo_(&repo)
    {
        if (git_tree_lookup(&tree_, repo.ptr(), &oid))
            throw tree_lookup_error(oid);
    }

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
        return BorrowedEntry(git_tree_entry_byname(tree_, filename.c_str()));
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
          throw error_t(boost::format("unknown error inside function: 'git_tree_entry_bypath': %1%") % status);
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

    Tree Tree::OwnedEntry::as_tree()
    {
       git_object * obj;
       git_tree_entry_to_object(&obj, repo_->ptr(), entry_);
       return Object(obj, *repo_).to_tree();
    }

    const char * Tree::BorrowedEntry::name() const
    {
       return git_tree_entry_name(entry_);
    }
}
