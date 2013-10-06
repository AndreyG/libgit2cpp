#pragma once

#include <cstddef>

extern "C"
{
#include <git2/types.h>
}

namespace git
{
    struct OdbObject
    {
        explicit OdbObject(git_odb_object * obj)
            : obj_(obj)
        {}

        ~OdbObject();

        git_otype               type() const;
        unsigned char const *   data() const;
        size_t                  size() const;

        OdbObject               (OdbObject const &) = delete;
        OdbObject& operator =   (OdbObject const &) = delete;

        OdbObject(OdbObject && other);

    private:
        git_odb_object * obj_;
    };
}

