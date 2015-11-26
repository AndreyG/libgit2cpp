#include <stdio.h>
#include <git2.h>
#include <string.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <sstream>

#include "git2cpp/initializer.h"
#include "git2cpp/id_to_str.h"
#include "git2cpp/repo.h"
#include "git2cpp/diff.h"

using namespace std;
using namespace git;

static const
unordered_map<int, string>	s_DiffStatusMap
{
	{GIT_DELTA_UNMODIFIED,	"UNMODIFIED"},
	{GIT_DELTA_ADDED,		"ADDED"},
	{GIT_DELTA_DELETED,		"DELETED"},
	{GIT_DELTA_MODIFIED,	"MODIFIED"},
	{GIT_DELTA_RENAMED,		"RENAMED"},
	{GIT_DELTA_COPIED,		"COPIED"},
	{GIT_DELTA_IGNORED,		"IGNORED"},
};

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
   default:
      break;
   }

   ostringstream ss;
   ss
         << "object corresponding to identifier "
         << identifier
         << " is neither commit or tree";

   throw std::logic_error(ss.str());
}

// (currently unused)
const char *colors[]
{
   "\033[m", /* reset */
   "\033[1m", /* bold */
   "\033[31m", /* red */
   "\033[32m", /* green */
   "\033[36m" /* cyan */
};

static int check_str_param(const char *arg, const char *pattern, const char **val)
{
   const size_t len = strlen(pattern);
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
   fprintf(stderr, "usage: diff [<options>] [<tree-oid> [<tree-oid>]]\n");
   exit(1);
}

/* <sha1> <sha2> */
/* <sha1> --cached */
/* <sha1> */
/* --cached */
/* nothing */

Diff calc_diff(Repository & repo, Tree & t1, Tree & t2, const bool cached, Diff::option const & opts)
{
   if (t1 && t2)
      return Diff(repo, t1, t2, opts);
   else if (t1 && cached)
      return Diff::diff_to_index(repo, t1, opts);
   else if (t1)
      return std::move(Diff::diff_to_index(repo, t1, opts).merge(Diff::diff_index_to_workdir(repo, opts)));
   else if (cached)
   {
      auto tree = resolve_to_tree(repo, "HEAD");
      return Diff::diff_to_index(repo, tree, opts);
   }
   else
      return Diff::diff_index_to_workdir(repo, opts);
}

int main(int argc, char *argv[])
{
	const char*		        dir = ".";
	bool			        cached_f, show_diff, show_files;
	vector<const char *>    treestr_list;
    Diff::option	        opts = Diff::option::normal;
	Diff::find_option       findopts = Diff::find_option::none;
	
	cached_f = show_diff = show_files = false;
    
	for (int i = 1; i < argc; ++i)
	{
		const char *a = argv[i];

		if (a[0] != '-') {
			if (treestr_list.size() >= 2)
				usage("No more than two trees should be provided", NULL);
			else	treestr_list.push_back(a);
		}
		else if (!strcmp(a, "--cached"))
			cached_f = true;
		else if (!strcmp(a, "--ignore-all-space"))
			findopts = findopts | Diff::find_option::ignore_whitespace;
		else if (!strcmp(a, "--find-renames"))
			findopts = findopts | Diff::find_option::renames;
		else if (!strcmp(a, "--find-copies"))
			findopts = findopts | Diff::find_option::copies;
		else if (!strcmp(a, "-p") || !strcmp(a, "-u") || !strcmp(a, "--patch"))
            show_diff = true;
        else if (!strcmp(a, "--name-only") || !strcmp(a, "--name-status"))
            show_files = true;
        else if (!strcmp(a, "--help") || !strcmp(a, "-h"))
			usage(NULL, NULL);
		else if (!check_str_param(a, "--git-dir=", &dir))
			usage("Unknown option", a);
	}

	if (treestr_list.empty() && !cached_f)
		usage(NULL, NULL);
	
	git::Initializer threads_initializer;
	
	const bool		simil_f = (~(Diff::find_option::renames | Diff::find_option::copies) & findopts) != findopts;
	const Diff::format	fmt = (show_files || simil_f) ? Diff::format::name_only : Diff::format::patch;
	
	try
	{
		Repository repo(dir);
		
		Tree	tree1 = (treestr_list.size() >= 1) ? resolve_to_tree(repo, treestr_list[0]) : Tree();
		Tree	tree2 = (treestr_list.size() == 2) ? resolve_to_tree(repo, treestr_list[1]) : Tree();
		
		Diff	df = calc_diff(repo, tree1, tree2, cached_f, opts);
		
		if (simil_f)	df.find_similar(findopts);
		
		const size_t	n_diffs = df.deltas_num();
		
		cerr << n_diffs << " delta(s)" << endl;
		if (!n_diffs)	return 0;
		
		auto	func = [=](git_diff_delta const &delta, const git_diff_hunk *hunk, git_diff_line const &line)
		{
			// (hunk may be null)
			const string	status_s = s_DiffStatusMap.count(delta.status) ? s_DiffStatusMap.at(delta.status) : "<unknown>";
				
			if (show_files)
            {
                cerr << " " << status_s << " " << delta.old_file.path;
			
                switch (delta.status)
                {
                    case GIT_DELTA_RENAMED:
                    case GIT_DELTA_COPIED:
                        cerr << " to " << delta.new_file.path << " simil = " << delta.similarity;
                        break;
                        
                    case GIT_DELTA_MODIFIED:
                    case GIT_DELTA_ADDED:
                    case GIT_DELTA_DELETED:
                    default:
                        
                        break;
                }
                
                cerr << endl;
            }
            
            if ((delta.status == GIT_DELTA_MODIFIED) && show_diff)
            {
                assert(line.content);       // not null-terminated
                const size_t    len = line.content_len;
                
                string  s(line.content, len);
                
                cerr << s;
            }
		};
			
		df.print(fmt, func);
	}
	catch (std::exception const &e)
	{
		cerr << e.what() << endl;
		if (auto err = giterr_last())
		{
			if (err->message)
				cerr << "libgit2 last error: " << err->message << endl;
		}

		return 1;
	}
	
	return 0;
}

