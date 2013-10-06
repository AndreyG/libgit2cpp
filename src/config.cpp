#include "git2cpp/config.h"
#include "git2cpp/error.h"

extern "C"
{
#include <git2/config.h>
}

namespace git
{
    Config::Config(std::string const & filename)
    {
        if (git_config_open_ondisk(&cfg_, filename.c_str()))
            throw config_open_error();
    }

    Config::~Config()
    {
        git_config_free(cfg_);
    }

    std::string Config::operator [] (const char * key) const
    {
        const char * res;
        if (git_config_get_string(&res, cfg_, key))
            throw no_such_key_error(key);
        return res;
    }

    int Config::get_int(const char * key) const
    {
        int res;
        if (git_config_get_int32(&res, cfg_, key))
            throw no_such_key_error(key);
        return res;
    }
}
