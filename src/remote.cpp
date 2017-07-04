#include "git2cpp/remote.h"

#include <git2/remote.h>

namespace git
{
    const char * Remote::url() const
    {
        return git_remote_url(remote_);
    }
}
