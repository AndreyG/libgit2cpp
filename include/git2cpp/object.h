#pragma once

#include "commit.h"
#include "blob.h"

struct git_object;
struct git_oid;

namespace git
{
   struct Repository;

   struct Object
   {
      Object() {}

      Object(git_object * obj, Repository const &);
      ~Object();

      explicit operator bool() const { return obj_ != nullptr; }

      git_otype       type()  const;
      git_oid const & id()    const;

      git_blob    const * as_blob()   const;
      git_commit  const * as_commit() const;
      git_tree    const * as_tree()   const;
      git_tag     const * as_tag()    const;

      Commit  to_commit()  /*&&*/;
      Tree    to_tree()    /*&&*/;
      Blob    to_blob()    /*&&*/;

      Object              (Object const &) = delete;
      Object& operator =  (Object const &) = delete;

      Object(Object && other);

   private:
      git_object * obj_          = nullptr;
      Repository const * repo_   = nullptr;
   };
}

