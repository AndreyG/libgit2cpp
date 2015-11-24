/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

extern "C"
{
#include <git2/submodule.h>
}

#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

using namespace git;

enum {
	FORMAT_DEFAULT   = 0,
	FORMAT_LONG      = 1,
	FORMAT_SHORT     = 2,
	FORMAT_PORCELAIN = 3,
};

/*
 * This example demonstrates the use of the libgit2 status APIs,
 * particularly the `git_status_list` object, to roughly simulate the
 * output of running `git status`.  It serves as a simple example of
 * using those APIs to get basic status information.
 *
 * This does not have:
 * - Robust error handling
 * - Colorized or paginated output formatting
 *
 * This does have:
 * - Examples of translating command line arguments to the status
 *   options settings to mimic `git status` results.
 * - A sample status formatter that matches the default "long" format
 *   from `git status`
 * - A sample status formatter that matches the "short" format
 */

void show_branch(Repository const & repo, int format)
{
	const char * branch = nullptr;
    try
	{
        Reference head = repo.head();
		branch = head.name();
		if (!strncmp(branch, "refs/heads/", strlen("refs/heads/")))
			branch += strlen("refs/heads/");
    }
    catch (non_existing_branch_error    const &) {}
    catch (missing_head_error           const &) {}
    catch (std::exception const & e)
    {
        throw e;
    }

	if (format == FORMAT_LONG)
		printf("# On branch %s\n",
			branch ? branch : "Not currently on any branch.");
	else
		printf("## %s\n", branch ? branch : "HEAD (no branch)");
}

void print_long(Status const & status)
{
	size_t maxi = status.entrycount();
	int header = 0, changes_in_index = 0;
	int changed_in_workdir = 0, rm_in_workdir = 0;
	const char *old_path, *new_path;

	/* print index changes */

	for (size_t i = 0; i < maxi; ++i) {
		auto s = status[i];

		if (s->status == GIT_STATUS_CURRENT)
			continue;

		if (s->status & GIT_STATUS_WT_DELETED)
			rm_in_workdir = 1;

		const char *istatus = nullptr;

		if (s->status & GIT_STATUS_INDEX_NEW)
			istatus = "new file: ";
		if (s->status & GIT_STATUS_INDEX_MODIFIED)
			istatus = "modified: ";
		if (s->status & GIT_STATUS_INDEX_DELETED)
			istatus = "deleted:  ";
		if (s->status & GIT_STATUS_INDEX_RENAMED)
			istatus = "renamed:  ";
		if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
			istatus = "typechange:";

		if (istatus == NULL)
			continue;

		if (!header) {
			printf("# Changes to be committed:\n");
			printf("#   (use \"git reset HEAD <file>...\" to unstage)\n");
			printf("#\n");
			header = 1;
		}

		old_path = s->head_to_index->old_file.path;
		new_path = s->head_to_index->new_file.path;

		if (old_path && new_path && strcmp(old_path, new_path))
			printf("#\t%s  %s -> %s\n", istatus, old_path, new_path);
		else
			printf("#\t%s  %s\n", istatus, old_path ? old_path : new_path);
	}

	if (header) {
		changes_in_index = 1;
		printf("#\n");
	}
	header = 0;

	/* print workdir changes to tracked files */

	for (size_t i = 0; i < maxi; ++i) {
		auto s = status[i];

		if (s->status == GIT_STATUS_CURRENT || !s->index_to_workdir)
			continue;

		const char *wstatus = nullptr;

		if (s->status & GIT_STATUS_WT_MODIFIED)
			wstatus = "modified: ";
		if (s->status & GIT_STATUS_WT_DELETED)
			wstatus = "deleted:  ";
		if (s->status & GIT_STATUS_WT_RENAMED)
			wstatus = "renamed:  ";
		if (s->status & GIT_STATUS_WT_TYPECHANGE)
			wstatus = "typechange:";

		if (wstatus == NULL)
			continue;

		if (!header) {
			printf("# Changes not staged for commit:\n");
			printf("#   (use \"git add%s <file>...\" to update what will be committed)\n", rm_in_workdir ? "/rm" : "");
			printf("#   (use \"git checkout -- <file>...\" to discard changes in working directory)\n");
			printf("#\n");
			header = 1;
		}

		old_path = s->index_to_workdir->old_file.path;
		new_path = s->index_to_workdir->new_file.path;

		if (old_path && new_path && strcmp(old_path, new_path))
			printf("#\t%s  %s -> %s\n", wstatus, old_path, new_path);
		else
			printf("#\t%s  %s\n", wstatus, old_path ? old_path : new_path);
	}

	if (header) {
		changed_in_workdir = 1;
		printf("#\n");
	}
	header = 0;

	/* print untracked files */

	header = 0;

	for (size_t i = 0; i < maxi; ++i) {
		auto s = status[i];

		if (s->status == GIT_STATUS_WT_NEW) {

			if (!header) {
				printf("# Untracked files:\n");
				printf("#   (use \"git add <file>...\" to include in what will be committed)\n");
				printf("#\n");
				header = 1;
			}

			printf("#\t%s\n", s->index_to_workdir->old_file.path);
		}
	}

	header = 0;

	/* print ignored files */

	for (size_t i = 0; i < maxi; ++i) {
		auto s = status[i];

		if (s->status == GIT_STATUS_IGNORED) {

			if (!header) {
				printf("# Ignored files:\n");
				printf("#   (use \"git add -f <file>...\" to include in what will be committed)\n");
				printf("#\n");
				header = 1;
			}

			printf("#\t%s\n", s->index_to_workdir->old_file.path);
		}
	}

	if (!changes_in_index && changed_in_workdir)
		printf("no changes added to commit (use \"git add\" and/or \"git commit -a\")\n");
}

void print_short(Repository const & repo, Status const & status)
{
	size_t maxi = status.entrycount();
	char istatus, wstatus;
	const char *extra, *a, *b, *c;

	for (size_t i = 0; i < maxi; ++i) 
    {
		auto s = status[i];

		if (s->status == GIT_STATUS_CURRENT)
			continue;

		a = b = c = NULL;
		istatus = wstatus = ' ';
		extra = "";

		if (s->status & GIT_STATUS_INDEX_NEW)
			istatus = 'A';
		if (s->status & GIT_STATUS_INDEX_MODIFIED)
			istatus = 'M';
		if (s->status & GIT_STATUS_INDEX_DELETED)
			istatus = 'D';
		if (s->status & GIT_STATUS_INDEX_RENAMED)
			istatus = 'R';
		if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
			istatus = 'T';

		if (s->status & GIT_STATUS_WT_NEW) {
			if (istatus == ' ')
				istatus = '?';
			wstatus = '?';
		}
		if (s->status & GIT_STATUS_WT_MODIFIED)
			wstatus = 'M';
		if (s->status & GIT_STATUS_WT_DELETED)
			wstatus = 'D';
		if (s->status & GIT_STATUS_WT_RENAMED)
			wstatus = 'R';
		if (s->status & GIT_STATUS_WT_TYPECHANGE)
			wstatus = 'T';

		if (s->status & GIT_STATUS_IGNORED) {
			istatus = '!';
			wstatus = '!';
		}

		if (istatus == '?' && wstatus == '?')
			continue;

		if (s->index_to_workdir &&
			s->index_to_workdir->new_file.mode == GIT_FILEMODE_COMMIT)
		{
			git_submodule *sm = NULL;
			unsigned int smstatus = 0;

			if (!repo.submodule_lookup(sm, s->index_to_workdir->new_file.path) &&
				!git_submodule_status(&smstatus, repo.ptr(), git_submodule_name(sm), git_submodule_ignore(sm)))
			{
				if (smstatus & GIT_SUBMODULE_STATUS_WD_MODIFIED)
					extra = " (new commits)";
				else if (smstatus & GIT_SUBMODULE_STATUS_WD_INDEX_MODIFIED)
					extra = " (modified content)";
				else if (smstatus & GIT_SUBMODULE_STATUS_WD_WD_MODIFIED)
					extra = " (modified content)";
				else if (smstatus & GIT_SUBMODULE_STATUS_WD_UNTRACKED)
					extra = " (untracked content)";
			}
		}

		if (s->head_to_index) {
			a = s->head_to_index->old_file.path;
			b = s->head_to_index->new_file.path;
		}
		if (s->index_to_workdir) {
			if (!a)
				a = s->index_to_workdir->old_file.path;
			if (!b)
				b = s->index_to_workdir->old_file.path;
			c = s->index_to_workdir->new_file.path;
		}

		if (istatus == 'R') {
			if (wstatus == 'R')
				printf("%c%c %s %s %s%s\n", istatus, wstatus, a, b, c, extra);
			else
				printf("%c%c %s %s%s\n", istatus, wstatus, a, b, extra);
		} else {
			if (wstatus == 'R')
				printf("%c%c %s %s%s\n", istatus, wstatus, a, c, extra);
			else
				printf("%c%c %s%s\n", istatus, wstatus, a, extra);
		}
	}

	for (size_t i = 0; i < maxi; ++i) {
		auto s = status[i];

		if (s->status == GIT_STATUS_WT_NEW)
			printf("?? %s\n", s->index_to_workdir->old_file.path);
	}
}

int main(int argc, char *argv[])
{
	git::Initializer threads_initializer;
	
	int i, npaths = 0, format = FORMAT_DEFAULT, zterm = 0, showbranch = 0;
	git_status_options opt = GIT_STATUS_OPTIONS_INIT;
	const char *repodir = ".";

    const size_t MAX_PATHSPEC = 8;
    char * pathspec[MAX_PATHSPEC];

	opt.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	opt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
		GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			if (npaths < MAX_PATHSPEC)
            {
				pathspec[npaths++] = argv[i];
            }
			else
            {
				std::cerr << "Example only supports a limited pathspec\n";
                return 1;
            }
		}
		else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--short"))
			format = FORMAT_SHORT;
		else if (!strcmp(argv[i], "--long"))
			format = FORMAT_LONG;
		else if (!strcmp(argv[i], "--porcelain"))
			format = FORMAT_PORCELAIN;
		else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--branch"))
			showbranch = 1;
		else if (!strcmp(argv[i], "-z")) {
			zterm = 1;
			if (format == FORMAT_DEFAULT)
				format = FORMAT_PORCELAIN;
		}
		else if (!strcmp(argv[i], "--ignored"))
			opt.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;
		else if (!strcmp(argv[i], "-uno") ||
				 !strcmp(argv[i], "--untracked-files=no"))
			opt.flags &= ~GIT_STATUS_OPT_INCLUDE_UNTRACKED;
		else if (!strcmp(argv[i], "-unormal") ||
				 !strcmp(argv[i], "--untracked-files=normal"))
			opt.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;
		else if (!strcmp(argv[i], "-uall") ||
				 !strcmp(argv[i], "--untracked-files=all"))
			opt.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED |
				GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
		else if (!strcmp(argv[i], "--ignore-submodules=all"))
			opt.flags |= GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
		else if (!strncmp(argv[i], "--git-dir=", strlen("--git-dir=")))
			repodir = argv[i] + strlen("--git-dir=");
		else
        {
            std::cerr << "Unsupported option '" << argv[i] << "'\n";
            return 1;
        }
	}

	if (format == FORMAT_DEFAULT)
		format = FORMAT_LONG;
	if (format == FORMAT_LONG)
		showbranch = 1;
	if (npaths > 0) {
		opt.pathspec.strings = pathspec;
		opt.pathspec.count   = npaths;
	}

	Repository repo(repodir);

	if (repo.is_bare())
    {
	    std::cerr << "Cannot report status on bare repository\n";
        return 1;
    }

	/*
	 * Run status on the repository
	 *
	 * Because we want to simluate a full "git status" run and want to
	 * support some command line options, we use `git_status_foreach_ext()`
	 * instead of just the plain status call.  This allows (a) iterating
	 * over the index and then the workdir and (b) extra flags that control
	 * which files are included.  If you just want simple status (e.g. to
	 * enumerate files that are modified) then you probably don't need the
	 * extended API.
	 */
	Status status = repo.status(opt);

	if (showbranch)
		show_branch(repo, format);

	if (format == FORMAT_LONG)
		print_long(status);
	else
		print_short(repo, status);

	return 0;
}

