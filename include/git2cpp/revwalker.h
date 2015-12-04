#pragma once

#include <git2/revwalk.h>

#include "commit.h"

namespace git
{
   struct Repository;

   struct RevWalker
   {
      RevWalker(git_revwalk * walker, Repository const & repo)
         : walker_(walker)
         , repo_(repo)
      {}

      ~RevWalker();

      enum class sorting : unsigned int
      {
         none        = GIT_SORT_NONE,
         topological = GIT_SORT_TOPOLOGICAL,
         time        = GIT_SORT_TIME,
         reverse     = GIT_SORT_REVERSE
      };

      friend sorting operator ~ (sorting);
      friend sorting operator | (sorting, sorting);
      friend sorting operator & (sorting, sorting);
      friend sorting operator ^ (sorting, sorting);

      void sort(sorting);
      void simplify_first_parent();

      void push_head() const;
      void hide(git_oid const &) const;
      void push(git_oid const &) const;

      Commit  next()                 const;
      bool    next(char * id_buffer) const;

      RevWalker               (RevWalker const &) = delete;
      RevWalker& operator =   (RevWalker const &) = delete;

      RevWalker(RevWalker && other)
         : walker_(other.walker_)
         , repo_(other.repo_)
      {
         other.walker_ = nullptr;
      }

   private:
      git_revwalk * walker_;
      Repository const & repo_;
   };
}
