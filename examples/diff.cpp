#include <stdio.h>
#include <git2.h>

#include <cstdlib>
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
	{GIT_DELTA_UNMODIFIED,		"UNMODIFIED"},
	{GIT_DELTA_ADDED,		"ADDED"},
	{GIT_DELTA_DELETED,		"DELETED"},
	{GIT_DELTA_MODIFIED,		"MODIFIED"},
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

const char *colors[] = {
   "\033[m", /* reset */
   "\033[1m", /* bold */
   "\033[31m", /* red */
   "\033[32m", /* green */
   "\033[36m" /* cyan */
};

// int diff_output(const git_diff_delta*, const git_diff_hunk*, const git_diff_line*, void*);

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

Diff calc_diff(Repository & repo, Tree & t1, Tree & t2, const bool cached, git_diff_options const & opts)
{
   if (t1 && t2)
      return git::diff_tree_to_tree(repo, t1, t2, opts);
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
	const char*		dir = ".";
	bool			cached_f = false;
	vector<const char *>	treestr_list;
	git_diff_options	opts = GIT_DIFF_OPTIONS_INIT;
	git_diff_find_options	findopts = GIT_DIFF_FIND_OPTIONS_INIT;
	
	// opts.flags = GIT_DIFF_NORMAL;
	// findopts.flags = GIT_DIFF_FIND_BY_CONFIG;
		
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
			findopts.flags |= GIT_DIFF_FIND_IGNORE_WHITESPACE;
		else if (!strcmp(a, "--find-renames"))
			findopts.flags |= GIT_DIFF_FIND_RENAMES;
		else if (!strcmp(a, "--find-copies"))
			findopts.flags |= GIT_DIFF_FIND_COPIES;
		else if (!strcmp(a, "--help") || !strcmp(a, "-h"))
			usage(NULL, NULL);
		else if (!check_str_param(a, "--git-dir=", &dir))
			usage("Unknown option", a);
	}

	if (treestr_list.empty() && !cached_f)
		usage(NULL, NULL);
	
	const bool		simil_f = (GIT_DIFF_FIND_RENAMES | GIT_DIFF_FIND_COPIES) & findopts.flags;
	const Diff::format	fmt = simil_f ? Diff::format::name_only : Diff::format::patch;
	
	git::Initializer threads_initializer;
	
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
		
		auto	func = [](git_diff_delta const &delta, const git_diff_hunk *hunk, git_diff_line const &ln)
		{
			const string	status_s = s_DiffStatusMap.count(delta.status) ? s_DiffStatusMap.at(delta.status) : "<unknown>";
				
			cerr << " " << status_s << " " << delta.old_file.path;
			
			switch (delta.status)
			{
				case GIT_DELTA_RENAMED:
				case GIT_DELTA_COPIED:
					cerr << " to " << delta.new_file.path << " simil = " << delta.similarity;
					break;
					
				case GIT_DELTA_MODIFIED:
					if (hunk && hunk->header_len)
						cerr << " lines " << string(hunk->header, hunk->header + (hunk->header_len - 1));
					else	cerr << " null hunk";
					break;
					
				case GIT_DELTA_ADDED:
				case GIT_DELTA_DELETED:
				default:
				
					break;
			}
			
			cerr << endl;
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

