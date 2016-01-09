#pragma once

#include "tagged_mask.h"
#include "commit.h"

namespace git
{
    struct Repository;

    namespace revwalker {
    namespace sorting
    {
        typedef tagged_mask_t<struct tag> type;

        extern const type none;
        extern const type topological;
        extern const type time;
        extern const type reverse;
    }}

    struct RevWalker
    {
        RevWalker(git_revwalk * walker, Repository const & repo)
            : walker_(walker)
            , repo_(&repo)
        {}

        ~RevWalker();

        void sort(revwalker::sorting::type);
        void simplify_first_parent();

        void push_head() const;
        void hide(git_oid const &) const;
        void push(git_oid const &) const;

        Commit  next()                 const;
        bool    next(char * id_buffer) const;

        RevWalker               (RevWalker const &) = delete;
        RevWalker& operator =   (RevWalker const &) = delete;

        RevWalker(RevWalker && other)
            : walker_(other.walker_)
            , repo_(other.repo_)
        {
            other.walker_ = nullptr;
        }

        RevWalker& operator = (RevWalker && other)
        {
            walker_ = other.walker_;
            repo_   = other.repo_;
            other.walker_ = nullptr;
            return *this;
        }

    private:
        git_revwalk * walker_;
        Repository const * repo_;
    };
}
