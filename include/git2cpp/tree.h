#pragma once

#include "pathspec.h"

namespace git
{
    struct Tree
    {
        git_tree const  * ptr() const   { return tree_; }
        git_tree        * ptr()         { return tree_; }

        int pathspec_match(uint32_t flags, Pathspec const & ps)
        {
            return git_pathspec_match_tree(NULL, tree_, flags, ps.ptr());
        }

        size_t entrycount() const
        {
            return git_tree_entrycount(tree_);
        }

        git_tree_entry const * operator[] (size_t i) const
        {
            return git_tree_entry_byindex(tree_, i);
        }

        git_tree_entry const * operator[] (std::string const & filename) const
        {
            return git_tree_entry_byname(tree_, filename.c_str());
        }

        Tree(git_oid const * oid, git_repository * repo);

        explicit Tree(git_tree * tree = nullptr)
            : tree_(tree)
        {}

        Tree(Tree && other)
            : tree_(other.tree_)
        {
            other.tree_ = nullptr;
        }

        Tree& operator =(Tree && other)
        {
            tree_ = other.tree_;
            other.tree_ = nullptr;
        }

        Tree                (Tree const &) = delete;
        Tree& operator =    (Tree const &) = delete; 

        ~Tree()
        {
            git_tree_free(tree_);
        }

        explicit operator bool() const { return tree_; }

    private:
        git_tree * tree_;
    };
}

