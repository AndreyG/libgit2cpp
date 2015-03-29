#include <stdio.h>
#include <git2.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"
#include "git2cpp/diff.h"

using namespace git;

Tree resolve_to_tree(Repository const & repo, const char *identifier)
{
   Object obj = revparse_single(repo, identifier);

   switch (obj.type())
   {
   case GIT_OBJ_TREE:
      return obj.to_tree();
      break;
   case GIT_OBJ_COMMIT:
      return obj.to_commit().tree();
      break;
   }

   std::ostringstream ss;
   ss
         << "object corresponding to identifier "
         << identifier
         << " is neither commit or tree";

   throw std::logic_error(ss.str());
}

const char *colors[] = {
   "\033[m", /* reset */
   "\033[1m", /* bold */
   "\033[31m", /* red */
   "\033[32m", /* green */
   "\033[36m" /* cyan */
};

int diff_output(const git_diff_delta*, const git_diff_hunk*, const git_diff_line*, void*);

static int check_str_param(const char *arg, const char *pattern, const char **val)
{
   size_t len = strlen(pattern);
   if (strncmp(arg, pattern, len))
      return 0;
   *val = (const char *)(arg + len);
   return 1;
}

static void usage(const char *message, const char *arg)
{
   if (message && arg)
      fprintf(stderr, "%s: %s\n", message, arg);
   else if (message)
      fprintf(stderr, "%s\n", message);
   fprintf(stderr, "usage: diff [<tree-oid> [<tree-oid>]]\n");
   exit(1);
}

enum {
   FORMAT_PATCH = 0,
   FORMAT_COMPACT = 1,
   FORMAT_RAW = 2
};

/* <sha1> <sha2> */
/* <sha1> --cached */
/* <sha1> */
/* --cached */
/* nothing */

Diff calc_diff(Repository & repo, Tree & t1, Tree & t2, bool cached, git_diff_options const & opts)
{
   if (t1 && t2)
      return git::diff(repo, t1, t2, opts);
   else if (t1 && cached)
      return diff_to_index(repo, t1, opts);
   else if (t1)
      return std::move(diff_to_index(repo, t1, opts).merge(diff_index_to_workdir(repo, opts)));
   else if (cached)
   {
      auto tree = resolve_to_tree(repo, "HEAD");
      return diff_to_index(repo, tree, opts);
   }
   else
      return diff_index_to_workdir(repo, opts);
}

int main(int argc, char *argv[])
{
}

