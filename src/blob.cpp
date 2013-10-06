#include "git2cpp/blob.h"

extern "C"
{
#include <git2/blob.h>
}

namespace git
{
    Blob::Blob(const git_oid *oid, git_repository *repo)
    {
        git_blob_lookup(&blob_, repo, oid);
    }

    Blob::~Blob()
    {
        git_blob_free(blob_);
    }

    git_off_t Blob::size() const
    {
        return git_blob_rawsize(blob_);
    }

    const void * Blob::content() const
    {
        return git_blob_rawcontent(blob_);
    }
}
