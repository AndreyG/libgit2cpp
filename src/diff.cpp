#include "git2cpp/diff.h"
#include "git2cpp/error.h"

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

    Diff::Stats Diff::stats() const
    {
        git_diff_stats * stats;
        if (git_diff_get_stats(&stats, diff_))
            throw error_t("git_diff_get_stats fail");
        else
            return Stats(stats);
    }

    Diff::Stats::~Stats()
    {
        git_diff_stats_free(stats_);
    }

    Buffer Diff::Stats::to_buf(diff::stats::format::type format, size_t width) const
    {
        git_buf buf = GIT_BUF_INIT_CONST(nullptr, 0);
        if (git_diff_stats_to_buf(&buf, stats_, git_diff_stats_format_t(format.value()), width))
            throw error_t("git_diff_stats_to_buf fail");
        else
            return Buffer(buf);
    }

    namespace diff
    {
        namespace stats {
        namespace format
        {
            const type none             (GIT_DIFF_STATS_NONE);
            const type full             (GIT_DIFF_STATS_FULL);
            const type _short           (GIT_DIFF_STATS_SHORT);
            const type number           (GIT_DIFF_STATS_NUMBER);
            const type include_summary  (GIT_DIFF_STATS_INCLUDE_SUMMARY);
        }}
    }
}

