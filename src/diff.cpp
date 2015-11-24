#include <cassert>

#include "git2cpp/diff.h"
#include "git2cpp/tree.h"
#include "git2cpp/repo.h"

namespace git
{
    Diff diff(Repository const & repo, Tree & a, Tree & b, git_diff_options const & opts)
    {
        git_diff * diff;
        auto op_res = git_diff_tree_to_tree(&diff, repo.ptr(), a.ptr(), b.ptr(), &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    Diff diff_to_index(Repository const & repo, Tree & t, git_diff_options const & opts)
    {
        git_diff * diff;
        auto op_res = git_diff_tree_to_index(&diff, repo.ptr(), t.ptr(), nullptr, &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    Diff diff_index_to_workdir(Repository const & repo, git_diff_options const & opts)
    {
        git_diff * diff;
        auto op_res = git_diff_index_to_workdir(&diff, repo.ptr(), nullptr, &opts);
        assert(op_res == 0);
        return Diff(diff);
    }

    namespace
    {
        int apply_callback  ( git_diff_delta const * delta
                            , git_diff_hunk const * hunk
                            , git_diff_line const * line
                            , void * payload )
        {
            assert(delta);
            assert(line);
            auto cb = reinterpret_cast<Diff::print_callback_t const *>(payload);
            (*cb)(*delta, *hunk, *line);
            return 0;
        }

        git_diff_format_t convert(Diff::format f)
        {
           switch (f)
           {
           case Diff::format::name_only:     return GIT_DIFF_FORMAT_NAME_ONLY;
           case Diff::format::name_status:   return GIT_DIFF_FORMAT_NAME_STATUS;
           case Diff::format::patch:         return GIT_DIFF_FORMAT_PATCH;
           case Diff::format::patch_header:  return GIT_DIFF_FORMAT_PATCH_HEADER;
           case Diff::format::raw:           return GIT_DIFF_FORMAT_RAW;
           }
        }
    }

    Diff& Diff::merge(Diff const & other)
    {
        git_diff_merge(diff_, other.diff_);
        return *this;
    }

    size_t Diff::deltas_num() const
    {
        return git_diff_num_deltas(diff_);
    }

    void Diff::print(format f, print_callback_t print_callback) const
    {
       git_diff_print(diff_, convert(f), &apply_callback, &print_callback);
    }
}

