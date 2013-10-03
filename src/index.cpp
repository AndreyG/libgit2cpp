#include <cassert>

extern "C"
{
#include <git2/repository.h>
#include <git2/index.h>
}

#include "git2cpp/index.h"
#include "git2cpp/error.h"

namespace git
{
    Index::Index(git_repository * repo)
    {
        if (git_repository_index(&index_, repo) < 0)
            throw index_open_error();
    }

    Index::~Index()
    {
        if (index_)
            git_index_free(index_);
    }

    Index::Index(Index && other)
        : index_(other.index_)
    {
        other.index_ = nullptr;
    }

    namespace
    {
        int apply_callback(const char * path, const char * matched_pathspec, void * payload)
        {
            auto cb = reinterpret_cast<Index::matched_path_callback_t *>(payload);
            return (*cb)(path, matched_pathspec);
        }
    }

    void Index::update_all(git_strarray const & pathspec, matched_path_callback_t cb)
    {
        int res = cb 
            ? git_index_update_all(index_, &pathspec, &apply_callback, &cb)
            : git_index_update_all(index_, &pathspec, nullptr, nullptr);
        assert(res == 0);
    }

    void Index::add_all(git_strarray const & pathspec, matched_path_callback_t cb, unsigned int flags)
    {
        int res = cb 
            ? git_index_add_all(index_, &pathspec, flags, &apply_callback, &cb)
            : git_index_add_all(index_, &pathspec, flags, nullptr, nullptr);
        assert(res == 0);
    }

    void Index::write() const
    {
        int res = git_index_write(index_);
        assert(res == 0);
    }
}
