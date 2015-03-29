#include "git2cpp/tree.h"
#include "git2cpp/error.h"

namespace git
{
    Tree::Tree(git_oid const & oid, git_repository *repo)
    {
        if (git_tree_lookup(&tree_, repo, &oid))
            throw tree_lookup_error(oid);
    }
}
