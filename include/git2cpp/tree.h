#pragma once

#include "diff_list.h"
#include "pathspec.h"

namespace git
{
    struct Tree
    {
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

        Tree                (Tree && other)
            : Tree(other.tree_)
        {
            other.tree_ = nullptr;
        }

        Tree& operator =    (Tree && other)
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

        friend DiffList diff(git_repository *, Tree const &, Tree const &, git_diff_options const & opts);

    private:
        git_tree * tree_;
    };
}

