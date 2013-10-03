extern "C" 
{
    #include <git2.h>
}

#include <cassert>

#include "git2cpp/diff_list.h"
#include "git2cpp/tree.h"

namespace git
{
    DiffList diff(git_repository * repo, Tree const & a, Tree const & b, git_diff_options const & opts)
    {
        git_diff_list * diff_list;
        auto op_res = git_diff_tree_to_tree(&diff_list, repo, a.tree_, b.tree_, &opts); 
        assert(op_res == 0);
        return DiffList(diff_list);
    }
}

