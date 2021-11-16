#include "git2cpp/initializer.h"
#include "git2cpp/remote.h"
#include "git2cpp/repo.h"

#include <git2/clone.h>

#include <cstdio>

/* Define the printf format specifer to use for size_t output */
#if defined(_MSC_VER) || defined(__MINGW32__)
#	define PRIuZ "Iu"
#	define PRIxZ "Ix"
#	define PRIdZ "Id"
#else
#	define PRIuZ "zu"
#	define PRIxZ "zx"
#	define PRIdZ "zd"
#endif

namespace
{
    struct progress_data
    {
        git_indexer_progress fetch_progress;
        size_t completed_steps;
        size_t total_steps;
        const char * path;

        void print()
        {
            int network_percent = fetch_progress.total_objects > 0 ? (100 * fetch_progress.received_objects) / fetch_progress.total_objects : 0;
            int index_percent = fetch_progress.total_objects > 0 ? (100 * fetch_progress.indexed_objects) / fetch_progress.total_objects : 0;

            int checkout_percent = total_steps > 0
                                       ? (int)((100 * completed_steps) / total_steps)
                                       : 0;
            size_t kbytes = fetch_progress.received_bytes / 1024;

            if (fetch_progress.total_objects &&
                fetch_progress.received_objects == fetch_progress.total_objects)
            {
                printf("Resolving deltas %u/%u\r",
                       fetch_progress.indexed_deltas,
                       fetch_progress.total_deltas);
            }
            else
            {
                printf("net %3d%% (%4" PRIuZ " kb, %5u/%5u)  /  idx %3d%% (%5u/%5u)  /  chk %3d%% (%4" PRIuZ "/%4" PRIuZ")%s\n",
                       network_percent, kbytes,
                       fetch_progress.received_objects, fetch_progress.total_objects,
                       index_percent, fetch_progress.indexed_objects, fetch_progress.total_objects,
                       checkout_percent,
                       completed_steps, total_steps,
                       path);
            }
        }
    };

    void checkout_progress(const char * path, size_t cur, size_t tot, void * payload)
    {
        progress_data * pd = static_cast<progress_data *>(payload);
        pd->completed_steps = cur;
        pd->total_steps = tot;
        pd->path = path;
        pd->print();
    }

    struct FetchCallbacks final : git::Remote::FetchCallbacks
    {
        FetchCallbacks(progress_data & pd)
            : pd_(pd)
        {
        }

        void sideband_progress(char const * str, int len) override
        {
            printf("remote: %.*s", len, str);
            fflush(stdout);
        }

        void transfer_progress(git_indexer_progress const & progress) override
        {
            pd_.fetch_progress = progress;
            pd_.print();
        }

        git_credential * acquire_cred(const char * url, const char * username_from_url, unsigned allowed_types) override
        {
            return nullptr;
        }

    private:
        progress_data & pd_;
    };
}

int main(int argc, char ** argv)
{
    /* Validate args */
    if (argc != 3)
    {
        printf("USAGE: %s <url> <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    auto_git_initializer;

    progress_data pd = {{0}};
    FetchCallbacks fetch_callbacks(pd);
    git_checkout_options checkout_opts = {GIT_CHECKOUT_OPTIONS_VERSION, GIT_CHECKOUT_SAFE};
    /* Set up options */
    checkout_opts.progress_cb = checkout_progress;
    checkout_opts.progress_payload = &pd;
    const char * url = argv[1];
    const char * path = argv[2];
    /* Do the clone */
    try
    {
        git::Repository::clone(url, path, checkout_opts, fetch_callbacks);
    }
    catch (git::repository_clone_error err)
    {
        printf("\n");
        if (auto detailed_info = std::get_if<git::repository_clone_error::detailed_info>(&err.data))
            printf("ERROR %d: %s\n", detailed_info->klass, detailed_info->message);
        else
            printf("ERROR %d: no detailed info\n", std::get<int>(err.data));
        return EXIT_FAILURE;
    }
}
