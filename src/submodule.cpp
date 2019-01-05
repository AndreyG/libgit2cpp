#include "git2cpp/submodule.h"

#include <git2/submodule.h>

#include "git2cpp/error.h"

namespace git
{
    Submodule::Submodule(git_submodule * sm, git_repository * repo)
        : sm_(sm)
        , repo_(repo)
    {
    }

    Submodule::~Submodule()
    {
        git_submodule_free(sm_);
    }

    Submodule::status Submodule::get_status() const
    {
        unsigned int res;
        if (auto err = git_submodule_status(&res, repo_, git_submodule_name(sm_), git_submodule_ignore(sm_)))
            throw error_t("git_submodule_status failed with error code " + std::to_string(err));
        else
            return static_cast<status>(res);
    }

    namespace
    {
        unsigned int to_raw(Submodule::status s)
        {
            return static_cast<unsigned int>(s);
        }
    }

    bool contains(Submodule::status set, Submodule::status element)
    {
        return to_raw(set) & to_raw(element);
    }
}
