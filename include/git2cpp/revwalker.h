#pragma once

#include <boost/optional.hpp>

namespace git
{
    struct RevWalker
    {
        explicit RevWalker(git_repository * repo);
        ~RevWalker();

        void sort(int sorting);

        void push_head() const;
        void hide(git_oid const * obj) const;
        void push(git_oid const * obj) const;

        boost::optional<git_oid> next() const;

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
