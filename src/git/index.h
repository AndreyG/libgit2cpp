#pragma once

#include <functional>

struct git_index;
struct git_repository;
struct git_strarray;

namespace git
{
    struct Index
    {
        explicit Index(git_repository * repo);
        ~Index();

        typedef std::function<int (const char * path, const char * mathched_pathspec)> matched_path_callback_t;
        
        void update_all (git_strarray const & pathspec, matched_path_callback_t cb);
        void add_all    (git_strarray const & pathspec, matched_path_callback_t cb, unsigned int flags = 0);

        void write() const;

        Index               (Index const &) = delete;
        Index& operator =   (Index const &) = delete;

        Index(Index && other);

    private:
        git_index * index_;
    };
}
