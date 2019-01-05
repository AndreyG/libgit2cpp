#include <git2/odb.h>
#include <git2/repository.h>

#include "git2cpp/error.h"
#include "git2cpp/odb.h"

namespace git
{
    Odb::Odb(git_repository * repo)
    {
        if (git_repository_odb(&odb_, repo))
            throw odb_open_error();
    }

    Odb::~Odb()
    {
        if (odb_)
            git_odb_free(odb_);
    }

    OdbObject Odb::read(git_oid const & oid) const
    {
        git_odb_object * obj;
        if (git_odb_read(&obj, odb_, &oid))
            throw odb_read_error(oid);
        return OdbObject(obj);
    }

    git_oid Odb::write(const void * data, size_t len, git_otype type)
    {
        git_oid res;
        if (git_odb_write(&res, odb_, data, len, type))
            throw odb_write_error();
        return res;
    }
}
