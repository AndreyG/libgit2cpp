#include "git2cpp/odb_object.h"

extern "C"
{
#include <git2/odb.h>
}

namespace git
{
    OdbObject::~OdbObject()
    {
        if (obj_)
            git_odb_object_free(obj_);
    }

    git_otype OdbObject::type() const
    {
        return git_odb_object_type(obj_);
    }

    unsigned char const * OdbObject::data() const
    {
        return reinterpret_cast<unsigned char const *>(git_odb_object_data(obj_));
    }

    size_t OdbObject::size() const
    {
        return git_odb_object_size(obj_);
    }
}

