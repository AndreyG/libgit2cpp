#pragma once

namespace git
{
    struct ThreadsInitializer
    {
        ThreadsInitializer();
        ~ThreadsInitializer();

        ThreadsInitializer              (ThreadsInitializer const &) = delete;
        ThreadsInitializer& operator =  (ThreadsInitializer const &) = delete;
    };
}
