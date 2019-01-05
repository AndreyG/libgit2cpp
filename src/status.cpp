#include "git2cpp/status.h"
#include "git2cpp/error.h"

namespace git
{
    size_t Status::entrycount() const
    {
        return git_status_list_entrycount(status_);
    }

    git_status_entry const & Status::operator[](size_t i) const
    {
        if (auto res = git_status_byindex(status_, i))
            return *res;
        else
            throw error_t("status entry index out of bounds: " + std::to_string(i));
    }

    namespace
    {
        git_status_show_t convert(Status::Options::Show show)
        {
            switch (show)
            {
            case Status::Options::Show::IndexOnly:
                return GIT_STATUS_SHOW_INDEX_ONLY;
            case Status::Options::Show::WorkdirOnly:
                return GIT_STATUS_SHOW_INDEX_ONLY;
            default:
                return GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
            }
        }
    }

    Status::Options::Options(Show show, Sort sort)
        : opts_(GIT_STATUS_OPTIONS_INIT)
    {
        opts_.show = convert(show);

        switch (sort)
        {
        case Sort::CaseSensitively:
            opts_.flags |= GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;
            break;
        case Sort::CaseInsensitively:
            opts_.flags |= GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
            break;
        }
    }

    void Status::Options::set_pathspec(char ** ptr, size_t size)
    {
        opts_.pathspec.strings = ptr;
        opts_.pathspec.count = size;
    }

    Status::Options & Status::Options::include_untracked()
    {
        opts_.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;
        return *this;
    }

    Status::Options & Status::Options::exclude_untracked()
    {
        opts_.flags &= ~GIT_STATUS_OPT_INCLUDE_UNTRACKED;
        return *this;
    }

    Status::Options & Status::Options::recurse_untracked_dirs()
    {
        opts_.flags |= GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
        return *this;
    }

    Status::Options & Status::Options::exclude_submodules()
    {
        opts_.flags |= GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
        return *this;
    }

    Status::Options & Status::Options::include_ignored()
    {
        opts_.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;
        return *this;
    }

    Status::Options & Status::Options::renames_head_to_index()
    {
        opts_.flags |= GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;
        return *this;
    }

    Status::Status(git_repository * repo, Options const & opts)
    {
        if (git_status_list_new(&status_, repo, opts.raw()))
            throw get_status_error();
    }

    Status::Status(Status && other) noexcept
        : status_(other.status_)
    {
        other.status_ = nullptr;
    }

    Status & Status::operator=(Status && other) noexcept
    {
        std::swap(status_, other.status_);
        return *this;
    }

    Status::~Status()
    {
        git_status_list_free(status_);
    }
}
