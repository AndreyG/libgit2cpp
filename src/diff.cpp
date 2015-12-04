#include "git2cpp/diff.h"

#include <cassert>

namespace git
{
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

    Diff::~Diff() { git_diff_free(diff_); }

    void Diff::find_similar(git_diff_find_options & findopts)
    {
        git_diff_find_similar(diff_, &findopts);
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

