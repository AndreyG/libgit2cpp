#pragma once

#include <functional>

#include <git2/diff.h>

namespace git
{
    struct Diff
    {
        size_t deltas_num() const;

        void find_similar(git_diff_find_options &);

        Diff& merge(Diff const & other);

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

