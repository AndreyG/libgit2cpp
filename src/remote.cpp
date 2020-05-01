#include "git2cpp/remote.h"

#include <git2/remote.h>

namespace git
{
    const char * Remote::url() const
    {
        return git_remote_url(remote_.get());
    }

    const char * Remote::pushurl() const
    {
        return git_remote_pushurl(remote_.get());
    }

    void Remote::Destroy::operator()(git_remote * remote) const
    {
        git_remote_free(remote);
    }
}
