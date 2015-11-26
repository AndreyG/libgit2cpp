#pragma once

#include <string>

extern "C"
{
#include <git2/oid.h>
}

namespace git
{
    std::string id_to_str(git_oid const & id, const size_t digits_num = GIT_OID_HEXSZ);

    git_oid str_to_id(const char * str);
    git_oid str_to_id(const std::string &s);
}
