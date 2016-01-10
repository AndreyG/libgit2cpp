#pragma once

#include <functional>

#include <git2/diff.h>

#include "tagged_mask.h"
#include "buffer.h"

namespace git
{
    namespace diff
    {
        namespace stats {
        namespace format
        {
            typedef tagged_mask_t<struct format_tag> type;

            extern const type none;
            extern const type full;
            extern const type _short;
            extern const type number;
            extern const type include_summary;
        }}
    }

    struct Diff
    {
        struct Stats
        {
            ~Stats();

            Stats               (Stats const &) = delete;
            Stats& operator =   (Stats const &) = delete;

            Stats(Stats &&);

            Buffer to_buf(diff::stats::format::type, size_t width) const;

        private:
            friend struct Diff;

            explicit Stats(git_diff_stats * stats)
                : stats_(stats)
            {}

        private:
            git_diff_stats * stats_;
        };

        size_t deltas_num() const;

        void find_similar(git_diff_find_options &);

        Stats stats() const;

        enum class format
        {
            patch, patch_header, raw, name_only, name_status
        };

        typedef
            std::function<void (git_diff_delta const &, git_diff_hunk const &, git_diff_line const &)>
            print_callback_t;

        void print(format, print_callback_t print_callback) const;

        explicit Diff(git_diff * diff)
            : diff_(diff)
        {}

        ~Diff();

        Diff              (Diff const &) = delete;
        Diff& operator =  (Diff const &) = delete;

        Diff(Diff && other)
            : diff_(other.diff_)
        {
            other.diff_ = nullptr;
        }

    private:
        git_diff * diff_;
    };
}

