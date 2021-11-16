#pragma once

#include <memory>

struct git_remote;
struct git_oid;
struct git_indexer_progress;
struct git_credential;

namespace git
{
    struct Remote
    {
        const char * url()      const;
        const char * pushurl()  const;

        struct FetchCallbacks
        {
        protected:
            ~FetchCallbacks() = default;

        public:
            virtual void update_tips(char const * refname, git_oid const & a, git_oid const & b) {}
            virtual void sideband_progress(char const * str, int len) {}
            virtual void transfer_progress(git_indexer_progress const &) {}

            virtual git_credential* acquire_cred(const char * url, const char * username_from_url, unsigned int allowed_types) = 0;
        };

        void fetch(FetchCallbacks &, char const * reflog_message = nullptr);

    private:
        friend struct Repository;

        struct Destroy { void operator() (git_remote*) const; };

        explicit Remote(git_remote * remote)
            : remote_(remote)
        {}

        std::unique_ptr<git_remote, Destroy> remote_;
    };
}
