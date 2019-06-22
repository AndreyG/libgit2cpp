#include <cassert>

#include <git2/index.h>
#include <git2/repository.h>

#include "git2cpp/error.h"
#include "git2cpp/index.h"

namespace git
{
    Index::Index(git_repository * repo)
    {
        git_index * index;
        if (git_repository_index(&index, repo) < 0)
            throw index_open_error();
        index_.reset(index);
    }

    Index::Index(const char * dir)
    {
        git_index * index;
        if (git_index_open(&index, dir))
            throw index_open_error();
        index_.reset(index);
        git_index_read(index, true);
    }

    void Index::Destroy::operator()(git_index* index) const
    {
        git_index_free(index);
    }

    size_t Index::entrycount() const
    {
        return git_index_entrycount(index_.get());
    }

    git_index_entry const * Index::operator[](size_t i) const
    {
        return git_index_get_byindex(index_.get(), i);
    }

    git_index_entry const* Index::get_by_path(const char* path, int stage) const
    {
        return git_index_get_bypath(index_.get(), path, stage);
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
                      ? git_index_update_all(index_.get(), &pathspec, &apply_callback, &cb)
                      : git_index_update_all(index_.get(), &pathspec, nullptr, nullptr);
        assert(res == 0);
    }

    void Index::add_all(git_strarray const & pathspec, matched_path_callback_t cb, unsigned int flags)
    {
        int res = cb
                      ? git_index_add_all(index_.get(), &pathspec, flags, &apply_callback, &cb)
                      : git_index_add_all(index_.get(), &pathspec, flags, nullptr, nullptr);
        assert(res == 0);
    }

    void Index::clear()
    {
        int res = git_index_clear(index_.get());
        assert(res == 0);
    }

    void Index::add_path(const char * path)
    {
        int res = git_index_add_bypath(index_.get(), path);
        assert(res == 0);
    }

    void Index::add_path(std::string const & path)
    {
        add_path(path.c_str());
    }

    void Index::remove_path(const char * path)
    {
        int res = git_index_remove_bypath(index_.get(), path);
        assert(res == 0);
    }

    void Index::remove_path(std::string const & path)
    {
        remove_path(path.c_str());
    }

    void Index::write() const
    {
        if (git_index_write(index_.get()))
            throw index_write_error();
    }

    git_oid Index::write_tree() const
    {
        git_oid res;
        if (git_index_write_tree(&res, index_.get()) < 0)
            throw index_write_tree_error();
        return res;
    }
}
