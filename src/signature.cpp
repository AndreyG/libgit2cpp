extern "C" {
#include <git2/signature.h>
}

#include "git2cpp/error.h"
#include "git2cpp/signature.h"

namespace git
{
    Signature::Signature(git_repository * repo)
    {
        if (git_signature_default(&sig_, repo) < 0)
            throw signature_create_error();
    }

    Signature::Signature(const char * name, const char * email, git_time_t time, int offset)
    {
        git_signature_new(&sig_, name, email, time, offset);
    }

    Signature::~Signature()
    {
        git_signature_free(sig_);
    }
}
