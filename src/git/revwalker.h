#pragma once

namespace git
{
    struct RevWalker
    {
        explicit RevWalker(git_revwalk * walker)
            : walker_(walker)
        {}

        ~RevWalker()
        {
            git_revwalk_free(walker_);
        }

        void sort(int sorting)
        {
            git_revwalk_sorting(walker_, sorting);
        }

        void push_head() const
        {
            auto res = git_revwalk_push_head(walker_);
            assert(res == 0);
        }

        void hide(git_oid const * obj) const
        {
            auto res = git_revwalk_hide(walker_, obj);
            assert(res == 0);
        }

        void push(git_oid const * obj) const
        {
            auto res = git_revwalk_push(walker_, obj);
            assert(res == 0);
        }

        boost::optional<git_oid> next() const
        {
            git_oid oid;
            if (git_revwalk_next(&oid, walker_) == 0)
                return oid;
            else
                return boost::none;
        }

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
