#pragma once

extern "C"
{
#include <git2/diff.h>
}

namespace git
{
    struct DiffList
    {
        size_t deltas_num() const
        {
            return git_diff_num_deltas(diff_list_);
        }

        void print_patch(git_diff_data_cb cb) const
        {
            auto res = git_diff_print_patch(diff_list_, cb, NULL);  
        }

        explicit DiffList(git_diff_list * diff_list)
            : diff_list_(diff_list)
        {}

        ~DiffList() { git_diff_list_free(diff_list_); }

        DiffList              (DiffList const &) = delete;
        DiffList& operator =  (DiffList const &) = delete; 

        DiffList(DiffList && other)
            : diff_list_(other.diff_list_)
        {
            other.diff_list_ = nullptr;
        }

    private:
        git_diff_list * diff_list_;
    };

    struct Tree;

    DiffList diff(git_repository * repo, Tree const & a, Tree const & b, git_diff_options const & opts);
}

