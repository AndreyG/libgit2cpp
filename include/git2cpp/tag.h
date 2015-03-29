#pragma once

#include "object.h"

struct git_tag;

namespace git
{
   struct Repository;

   struct Tag
   {
      Tag(git_oid const & oid, Repository const &);
      ~Tag();

      Tag& operator = (Tag const &) = delete;
      Tag             (Tag const &) = delete;

      Tag(Tag &&);

      Object          target()        const;
      git_otype       target_type()   const;
      const char *    name()          const;
      const char *    message()       const;

   private:
      git_tag * tag_;
      Repository const & repo_;
   };
}
