#pragma once

#include "commit.h"

struct git_repository;
struct git_revwalk;

namespace git
{
   struct Repository;

   struct RevWalker
   {
      explicit RevWalker(Repository const & repo);
      ~RevWalker();

      enum class sorting : unsigned int
      {
         none        = 0,
         topological = 1 << 0,
         time        = 1 << 1,
         reverse     = 1 << 2
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
