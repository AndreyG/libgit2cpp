#include <cassert>

#include "git2cpp/diff_list.h"
#include "git2cpp/tree.h"

namespace git
{
    DiffList diff           (git_repository * repo
                            , Tree & a, Tree & b
                            , git_diff_options const & opts
                            )
    {
        git_diff_list * diff_list;
        auto op_res = git_diff_tree_to_tree(&diff_list, repo, a.ptr(), b.ptr(), &opts); 
        assert(op_res == 0);
        return DiffList(diff_list);
    }

    DiffList diff_to_index  ( git_repository * repo
                            , Tree & t 
                            , git_diff_options const & opts
                            ) 
    {
        git_diff_list * diff_list;
        auto op_res = git_diff_tree_to_index(&diff_list, repo, t.ptr(), nullptr, &opts); 
        assert(op_res == 0);
        return DiffList(diff_list);
    }

    DiffList diff_index_to_workdir(git_repository * repo, git_diff_options const & opts)
    {
        git_diff_list * diff_list;
        auto op_res = git_diff_index_to_workdir(&diff_list, repo, nullptr, &opts); 
        assert(op_res == 0);
        return DiffList(diff_list);
    }

    namespace 
    {
        int apply_callback  ( git_diff_delta const * delta
                            , git_diff_range const * range
                            , char usage
                            , const char * line
                            , size_t line_len
                            , void * payload ) 
        {
            auto cb = reinterpret_cast<DiffList::data_callback_t *>(payload);
            return (*cb)(delta, range, usage, line, line_len);
        }
    }

    void DiffList::print_patch(data_callback_t cb) const
    {
        git_diff_print_patch(diff_list_, &apply_callback, &cb);  
    }

    void DiffList::print_compact(data_callback_t cb) const
    {
        git_diff_print_compact(diff_list_, &apply_callback, &cb);
    }

    void DiffList::print_raw(data_callback_t cb) const
    {
        git_diff_print_raw(diff_list_, &apply_callback, &cb);
    }

    DiffList& DiffList::merge(DiffList const & other)
    {
        git_diff_merge(diff_list_, other.diff_list_);
        return *this;
    }
}

