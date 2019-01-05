#include "git2cpp/initializer.h"

#include <git2/global.h>

namespace git
{
    Initializer::Initializer()
    {
        git_libgit2_init();
    }

    Initializer::~Initializer()
    {
        git_libgit2_shutdown();
    }
}
