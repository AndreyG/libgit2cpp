#include <cassert>

#include <git2/index.h>
#include <git2/repository.h>

#include "git2cpp/error.h"
#include "git2cpp/index.h"

namespace git
{
    Index::Index(git_repository * repo)
    {
        if (git_repository_index(&index_, repo) < 0)
            throw index_open_error();
    }

    Index::Index(const char * dir)
    {
        if (git_index_open(&index_, dir))
            throw index_open_error();
        git_index_read(index_, true);
    }

    Index::~Index()
    {
        git_index_free(index_);
    }

    Index::Index(Index && other) noexcept
        : index_(other.index_)
    {
        other.index_ = nullptr;
    }

    Index & Index::operator=(Index && other) noexcept
    {
        std::swap(index_, other.index_);
        return *this;
    }

    size_t Index::entrycount() const
    {
        return git_index_entrycount(index_);
    }

    git_index_entry const * Index::operator[](size_t i) const
    {
        return git_index_get_byindex(index_, i);
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

    void Index::clear()
    {
        int res = git_index_clear(index_);
        assert(res == 0);
    }

    void Index::add_path(const char * path)
    {
        int res = git_index_add_bypath(index_, path);
        assert(res == 0);
    }

    void Index::add_path(std::string const & path)
    {
        add_path(path.c_str());
    }

    void Index::remove_path(const char * path)
    {
        int res = git_index_remove_bypath(index_, path);
        assert(res == 0);
    }

    void Index::remove_path(std::string const & path)
    {
        remove_path(path.c_str());
    }

    void Index::write() const
    {
        if (git_index_write(index_))
            throw index_write_error();
    }

    git_oid Index::write_tree() const
    {
        git_oid res;
        if (git_index_write_tree(&res, index_) < 0)
            throw index_write_tree_error();
        return res;
    }
}
