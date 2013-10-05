extern "C"
{
#include <git2/signature.h>
}

#include "git2cpp/signature.h"
#include "git2cpp/error.h"

namespace git
{
    Signature::Signature(git_repository * repo)
    {
        if (git_signature_default(&sig_, repo) < 0)
            throw signature_create_error();
    }

    Signature::~Signature()
    {
        git_signature_free(sig_);
    }
}

