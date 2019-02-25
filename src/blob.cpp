#include "git2cpp/blob.h"

#include <git2/blob.h>

namespace git
{
    Blob::Blob(git_blob * blob)
        : blob_(blob)
    {
    }

    std::size_t Blob::size() const
    {
        return static_cast<std::size_t>(git_blob_rawsize(ptr()));
    }

    const void * Blob::content() const
    {
        return git_blob_rawcontent(ptr());
    }

    void Blob::Destroy::operator()(git_blob * blob) const
    {
        git_blob_free(blob);
    }
}
