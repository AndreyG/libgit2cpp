#pragma once

#include "diff_list.h"
#include "pathspec.h"

namespace git
{
    struct Tree
    {
        git_tree const  * ptr() const   { return tree_; }
        git_tree        * ptr()         { return tree_; }

        int pathspec_match(uint32_t flags, Pathspec const & ps)
        {
            return git_pathspec_match_tree(NULL, tree_, flags, ps.get());
        }

        explicit Tree(git_tree * tree)
            : tree_(tree)
        {}

        Tree()
            : Tree(nullptr)
        {}

        Tree(Tree && other)
            : Tree(other.tree_)
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

