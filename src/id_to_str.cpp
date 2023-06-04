#include "git2cpp/id_to_str.h"

#include <git2/oid.h>

namespace git
{
    std::string id_to_str(git_oid const & oid)
    {
        return id_to_str(oid, GIT_OID_SHA1_HEXSIZE);
    }

    std::string id_to_str(git_oid const & oid, size_t digits_num)
    {
        char buf[GIT_OID_SHA1_HEXSIZE + 1];
        git_oid_tostr(buf, sizeof(buf), &oid);
        return std::string(buf, buf + digits_num);
    }

    git_oid str_to_id(const char * str)
    {
        git_oid res;
        git_oid_fromstr(&res, str);
        return res;
    }
}
