#pragma once

#include <git2/strarray.h>
#include <git2/diff.h>
#include <git2/status.h>

namespace git
{
    struct Status
    {
        struct Options
        {
            enum class Show
            {
                IndexOnly,
                WorkdirOnly,
                IndexAndWorkdir
            };

            enum class Sort
            {
                CaseSensitively,
                CaseInsensitively,
            };

            Options(Show = Show::IndexAndWorkdir, Sort = Sort::CaseSensitively);

            Options& include_untracked();
            Options& exclude_untracked();
            Options& renames_head_to_index();
            Options& include_ignored();
            Options& recurse_untracked_dirs();
            Options& exclude_submodules();

            void set_pathspec(char ** ptr, size_t size);

            git_status_options const * raw() const { return &opts_; }

        private:
            git_status_options opts_;
        };

        Status(git_repository * repo, Options const & opts);
        ~Status();

        size_t entrycount() const;
        git_status_entry const & operator[] (size_t i) const;

        Status              (Status const &) = delete;
        Status& operator =  (Status const &) = delete;

        Status              (Status &&) noexcept;
        Status& operator =  (Status &&) noexcept;

    private:
        git_status_list * status_;
    };
}

