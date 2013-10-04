#pragma once

#include "odb_object.h"

struct git_odb;

namespace git
{
    struct Odb
    {
        explicit Odb(git_repository * repo);
        ~Odb();

        OdbObject read(git_oid const * oid) const;

        Odb             (Odb const &) = delete;
        Odb& operator = (Odb const &) = delete;

        Odb(Odb && other);

    private:
        git_odb * odb_;
    };
}

