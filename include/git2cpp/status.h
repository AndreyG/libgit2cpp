#pragma once

extern "C"
{
#include <git2/strarray.h>
#include <git2/diff.h>
#include <git2/status.h>
}

struct git_status_list;

namespace git
{
    struct Status
    {
        Status(git_repository * repo, git_status_options const & opts);
        ~Status();

        size_t entrycount() const;
        git_status_entry const * operator[] (size_t i) const;

        Status              (Status const &) = delete;
        Status& operator =  (Status const &) = delete;

        Status(Status &&);

    private:
        git_status_list * status_;
    };
}

