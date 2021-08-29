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

    void Remote::fetch(FetchCallbacks & callbacks, char const * reflog_message)
    {
        git_fetch_options opts = GIT_FETCH_OPTIONS_INIT;
        opts.callbacks.payload = &callbacks;

        opts.callbacks.update_tips = [] (char const * refname, git_oid const * a, git_oid const * b, void * data)
        {
            auto callbacks = static_cast<FetchCallbacks*>(data);
            callbacks->update_tips(refname, *a, *b);
            return 0;
        };
        opts.callbacks.sideband_progress = [] (char const * str, int len, void * data)
        {
            auto callbacks = static_cast<FetchCallbacks*>(data);
            callbacks->sideband_progress(str, len);
            return 0;
        };
        opts.callbacks.transfer_progress = [] (git_indexer_progress const * stats, void * data)
        {
            auto callbacks = static_cast<FetchCallbacks*>(data);
            callbacks->transfer_progress(*stats);
            return 0;
        };
        opts.callbacks.credentials = [] (git_credential ** out, char const *url, char const * user_from_url, unsigned int allowed_types, void * data)
        {
            auto callbacks = static_cast<FetchCallbacks*>(data);
            auto cred = callbacks->acquire_cred(url, user_from_url, allowed_types);
            if (!cred)
                return -1;
            *out = cred;
            return 0;
        };
        git_remote_fetch(remote_.get(), nullptr, &opts, reflog_message);
    }

    void Remote::Destroy::operator()(git_remote * remote) const
    {
        git_remote_free(remote);
    }
}
