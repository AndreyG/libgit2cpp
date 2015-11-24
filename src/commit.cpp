#include "git2cpp/commit.h"
#include "git2cpp/repo.h"
#include "git2cpp/error.h"

extern "C"
{
#include <git2/commit.h>
#include <git2/merge.h>
}

namespace git
{
   Commit Commit::parent(size_t i) const
   {
      git_commit * parent;
      if (git_commit_parent(&parent, commit_, i))
         throw commit_parent_error(id());
      return Commit(parent, *repo_);
   }

   Tree Commit::tree() const
   {
      git_tree * tree;
      if (git_commit_tree(&tree, commit_))
         throw commit_tree_error(id());
      return Tree(tree, *repo_);
   }

   size_t Commit::parents_num() const
   {
      return git_commit_parentcount(commit_);
   }

   git_oid const & Commit::id() const
   {
      return *git_commit_id(commit_);
   }

   git_oid const & Commit::parent_id(size_t i) const
   {
      return *git_commit_parent_id(commit_, i);
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

   const char * Commit::summary() const
   {
      return git_commit_summary(commit_);
   }

   git_time_t Commit::time() const
   {
      return git_commit_time(commit_);
   }

   git_oid Commit::merge_base(size_t p1, size_t p2) const
   {
      git_oid res;
      if (git_merge_base(&res, repo_->ptr(), &parent_id(p1), &parent_id(p2)))
         throw merge_base_error(parent_id(p1), parent_id(p2));
      return res;
   }

   Commit::Commit(git_oid const & oid, Repository const & repo)
      : repo_(&repo)
   {
      if (git_commit_lookup(&commit_, repo.ptr(), &oid))
         throw commit_lookup_error(oid);
   }

   Commit::~Commit() { git_commit_free(commit_); }
}
