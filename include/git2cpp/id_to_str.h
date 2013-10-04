#pragma once

#include <string>

struct git_oid;

namespace git
{
    std::string id_to_str(git_oid const * oid);

    std::string id_to_str(git_oid const * oid, size_t digits_num);
}
