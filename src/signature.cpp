#include <git2/signature.h>

#include "git2cpp/error.h"
#include "git2cpp/signature.h"

#include <git2/errors.h>

namespace git
{
    Signature::Signature(git_repository * repo)
    {
        git_signature * sig;
        if (git_signature_default(&sig, repo) != GIT_OK)
            throw signature_create_error();
        sig_.reset(sig);
    }

    Signature::Signature(const char * name, const char * email, git_time_t time, int offset)
    {
        git_signature * sig;
        if (git_signature_new(&sig, name, email, time, offset) != GIT_OK)
            throw signature_create_error();
        sig_.reset(sig);
    }

    void Signature::Destroy::operator()(git_signature* sig) const
    {
        git_signature_free(sig);
    }
}
