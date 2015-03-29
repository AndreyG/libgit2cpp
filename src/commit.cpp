#include "git2cpp/commit.h"
#include "git2cpp/repo.h"

extern "C"
{
#include <git2/commit.h>
}

namespace git
{
   Commit Commit::parent(size_t i) const
   {
      git_commit * parent;
      if (git_commit_parent(&parent, commit_, i));
      throw commit_parent_error(id());
      return Commit(parent, *repo_);
   }

   Tree Commit::tree() const
   {
      git_tree * tree;
      if (git_commit_tree(&tree, commit_))
         throw commit_tree_error(id());
      return Tree(tree);
   }

   size_t Commit::parents_num() const
   {
      return git_commit_parentcount(commit_);
   }

   git_oid const * Commit::id() const
   {
      return git_commit_id(commit_);
   }

   git_oid const * Commit::parent_id(size_t i) const
   {
      return git_commit_parent_id(commit_, i);
   }

   git_signature const * Commit::author() const
   {
      return git_commit_author(commit_);
   }

   git_signature const * Commit::commiter() const
   {
      return git_commit_committer(commit_);
   }

   const char * Commit::message() const
   {
      return git_commit_message(commit_);
   }

   git_time_t Commit::time() const
   {
      return git_commit_time(commit_);
   }

   Commit::Commit(git_oid const * oid, Repository const & repo)
      : repo_(&repo)
   {
      if (git_commit_lookup(&commit_, repo.ptr(), oid))
         throw commit_lookup_error(oid);
   }

   Commit::~Commit() { git_commit_free(commit_); }
}
