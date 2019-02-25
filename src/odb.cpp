#include <git2/odb.h>
#include <git2/repository.h>

#include "git2cpp/error.h"
#include "git2cpp/odb.h"

namespace git
{
    Odb::Odb(git_repository * repo)
    {
        git_odb * odb;
        if (git_repository_odb(&odb, repo))
            throw odb_open_error();
        odb_.reset(odb);
    }

    void Odb::Destroy::operator()(git_odb* odb) const
    {
        git_odb_free(odb);
    }

    OdbObject Odb::read(git_oid const & oid) const
    {
        git_odb_object * obj;
        if (git_odb_read(&obj, odb_.get(), &oid))
            throw odb_read_error(oid);
        return OdbObject(obj);
    }

    git_oid Odb::write(const void * data, size_t len, git_object_t type)
    {
        git_oid res;
        if (git_odb_write(&res, odb_.get(), data, len, type))
            throw odb_write_error();
        return res;
    }
}
