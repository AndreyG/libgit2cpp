#include "git2cpp/config.h"
#include "git2cpp/error.h"

#include <git2/config.h>

namespace git
{
    Config::Config(std::string const & filename)
    {
        git_config * cfg;
        if (git_config_open_ondisk(&cfg, filename.c_str()))
            throw config_open_error();
        cfg_.reset(cfg);
    }

    void Config::Destroy::operator()(git_config* cfg) const
    {
        git_config_free(cfg);
    }

    std::string Config::operator[](const char * key) const
    {
        const char * res;
        if (git_config_get_string(&res, cfg_.get(), key))
            throw no_such_key_error(key);
        return res;
    }

    int Config::get_int(const char * key) const
    {
        int res;
        if (git_config_get_int32(&res, cfg_.get(), key))
            throw no_such_key_error(key);
        return res;
    }
}
