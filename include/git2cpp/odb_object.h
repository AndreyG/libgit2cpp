#pragma once

#include <cstddef>

struct git_odb_object;

namespace git
{
    struct OdbObject
    {
        OdbObject(git_odb_object * obj);
        ~OdbObject();

        size_t size() const;

        OdbObject               (OdbObject const &) = delete;
        OdbObject& operator =   (OdbObject const &) = delete;

        OdbObject(OdbObject && other);

    private:
        git_odb_object * obj_;
    };
}

