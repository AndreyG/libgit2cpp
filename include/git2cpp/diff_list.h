#pragma once

#include <functional>

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

        typedef std::function<int   ( git_diff_delta const *
                                    , git_diff_range const *
                                    , char usage
                                    , const char * line
                                    , size_t line_len
                                    ) > data_callback_t;

        void print_patch    (data_callback_t cb) const;
        void print_compact  (data_callback_t cb) const;
        void print_raw      (data_callback_t cb) const;

        void find_similar(git_diff_find_options & findopts)
        {
            git_diff_find_similar(diff_list_, &findopts);
        }

        DiffList& merge(DiffList const & other);

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

    DiffList diff           ( git_repository * repo
                            , Tree & a, Tree & b
                            , git_diff_options const & opts
                            );
    DiffList diff_to_index  ( git_repository * repo
                            , Tree & 
                            , git_diff_options const & opts
                            ); 

    DiffList diff_index_to_workdir(git_repository * repo, git_diff_options const & opts);
}

