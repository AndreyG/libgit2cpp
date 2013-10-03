#include "git2cpp/threads_initializer.h"

extern "C"
{
#include <git2/threads.h>
}

namespace git
{
    ThreadsInitializer::ThreadsInitializer()
    {
        git_threads_init();
    }

    ThreadsInitializer::~ThreadsInitializer()
    {
        git_threads_shutdown();
    }
}

