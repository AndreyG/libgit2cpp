#include "git2cpp/odb_object.h"

#include <git2/odb.h>

namespace git
{
    void OdbObject::Destroy::operator()(git_odb_object* obj) const
    {
        git_odb_object_free(obj);
    }

    git_otype OdbObject::type() const
    {
        return git_odb_object_type(obj_.get());
    }

    unsigned char const * OdbObject::data() const
    {
        return reinterpret_cast<unsigned char const *>(git_odb_object_data(obj_.get()));
    }

    size_t OdbObject::size() const
    {
        return git_odb_object_size(obj_.get());
    }

}
