#pragma once

namespace git
{
    struct Initializer
    {
        Initializer();
        ~Initializer();

        Initializer              (Initializer const &) = delete;
        Initializer& operator =  (Initializer const &) = delete;
    };
}
