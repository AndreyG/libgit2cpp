#pragma once

#include "commit.h"

struct git_repository;
struct git_revwalk;

namespace git
{
    struct Repository;

    struct RevWalker
    {
        explicit RevWalker(git_repository * repo);
        ~RevWalker();

        void sort(int sorting);

        void push_head() const;
        void hide(git_oid const * obj) const;
        void push(git_oid const * obj) const;

        Commit next(Repository const & repo) const;

        RevWalker               (RevWalker const &) = delete;
        RevWalker& operator =   (RevWalker const &) = delete;

        RevWalker(RevWalker && other)
            : walker_(other.walker_)
        {
            other.walker_ = nullptr;
        }

    private:
        git_revwalk * walker_;
    };
}
