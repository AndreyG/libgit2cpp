extern "C"
{
#include <git2/repository.h>
#include <git2/odb.h>
}

#include "git2cpp/odb.h"
#include "git2cpp/error.h"

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

    OdbObject Odb::read(git_oid const * oid) const
    {
        git_odb_object * obj;
        if (git_odb_read(&obj, odb_, oid))
            throw odb_read_error(oid);
    }
}

