#include "git2cpp/blob.h"

extern "C"
{
#include <git2/blob.h>
}

namespace git
{
    Blob::Blob(git_blob * blob)
       : blob_(blob)
    {}

    Blob::Blob(git_oid const & oid, git_repository *repo)
    {
        git_blob_lookup(&blob_, repo, &oid);
    }

    Blob::Blob(Blob && other)
       : blob_(other.blob_)
    {
       other.blob_ = nullptr;
    }

    Blob::~Blob()
    {
        git_blob_free(blob_);
    }

    std::size_t Blob::size() const
    {
        return git_blob_rawsize(blob_);
    }

    const void * Blob::content() const
    {
        return git_blob_rawcontent(blob_);
    }
}
