extern "C"
{
#include <git2/revwalk.h>
}

#include "git2cpp/error.h"
#include "git2cpp/revwalker.h"
#include "git2cpp/repo.h"

namespace git
{
    RevWalker::RevWalker(Repository const & repo)
       : repo_(repo)
    {
        if (git_revwalk_new(&walker_, repo.ptr()))
            throw revwalk_new_error();
    }

    RevWalker::~RevWalker()
    {
        git_revwalk_free(walker_);
    }

    namespace
    {
       unsigned int raw(RevWalker::sorting s)
       {
          return static_cast<unsigned int>(s);
       }
    }

    RevWalker::sorting operator ~ (RevWalker::sorting s)
    {
       return RevWalker::sorting(~raw(s));
    }

    RevWalker::sorting operator | (RevWalker::sorting a, RevWalker::sorting b)
    {
       return RevWalker::sorting(raw(a) | raw(b));
    }

    RevWalker::sorting operator ^ (RevWalker::sorting a, RevWalker::sorting b)
    {
       return RevWalker::sorting(raw(a) ^ raw(b));
    }

    RevWalker::sorting operator & (RevWalker::sorting a, RevWalker::sorting b)
    {
       return RevWalker::sorting(raw(a) & raw(b));
    }

    void RevWalker::sort(sorting s)
    {
        git_revwalk_sorting(walker_, raw(s));
    }

    void RevWalker::simplify_first_parent()
    {
       git_revwalk_simplify_first_parent(walker_);
    }

    void RevWalker::push_head() const
    {
        if (git_revwalk_push_head(walker_))
            throw invalid_head_error();
    }

    void RevWalker::hide(git_oid const & obj) const
    {
        if (git_revwalk_hide(walker_, &obj))
            throw non_commit_object_error(obj);
    }

    void RevWalker::push(git_oid const & obj) const
    {
        if (git_revwalk_push(walker_, &obj))
            throw non_commit_object_error(obj);
    }

    Commit RevWalker::next() const
    {
        git_oid oid;
        if (git_revwalk_next(&oid, walker_) == 0)
            return repo_.commit_lookup(oid);
        else
            return Commit();
    }

    bool RevWalker::next(char * id_buffer) const
    {
        git_oid oid;
        bool valid = (git_revwalk_next(&oid, walker_) == 0);
        if (valid)
            git_oid_fmt(id_buffer, &oid);
        return valid;
    }
}

