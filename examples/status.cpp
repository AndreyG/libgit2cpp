/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#include <string.h>

#include <iostream>

#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

#ifdef USE_BOOST
	#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

	template<typename T>
	using Optional = boost::optional<T>;

	using StringView = boost::string_view;
#else
	#include <string_view>
	#include <optional>

	template<typename T>
	using Optional = std::optional<T>;

	using StringView = std::string_view;
#endif

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

bool starts_with(StringView str, StringView prefix)
{
    return str.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), str.begin());
}

void show_branch(Repository const & repo, int format)
{
    Optional<StringView> branch;
    try
    {
        branch = repo.head().name();
        static const StringView refs_heads = "refs/heads/";
        if (starts_with(*branch, refs_heads))
            branch->remove_prefix(refs_heads.length());
    }
    catch (non_existing_branch_error    const &) {}
    catch (missing_head_error           const &) {}
    catch (std::exception const & e)
    {
        throw e;
    }

    if (format == FORMAT_LONG)
        std::cout << "# On branch " << branch.value_or("Not currently on any branch.");
    else
        std::cout<< "## ", branch.value_or("HEAD (no branch)");
    std::cout << "\n";
}

void print_long(Status const & status)
{
	size_t maxi = status.entrycount();
	int rm_in_workdir = 0;
	const char *old_path, *new_path;

	/* print index changes */

    bool changes_in_index = false;

	for (size_t i = 0; i < maxi; ++i) {
        auto const & s = status[i];

        if (s.status == GIT_STATUS_CURRENT)
			continue;

        if (s.status & GIT_STATUS_WT_DELETED)
			rm_in_workdir = 1;

		const char *istatus = nullptr;

        if (s.status & GIT_STATUS_INDEX_NEW)
			istatus = "new file: ";
        if (s.status & GIT_STATUS_INDEX_MODIFIED)
			istatus = "modified: ";
        if (s.status & GIT_STATUS_INDEX_DELETED)
			istatus = "deleted:  ";
        if (s.status & GIT_STATUS_INDEX_RENAMED)
			istatus = "renamed:  ";
        if (s.status & GIT_STATUS_INDEX_TYPECHANGE)
			istatus = "typechange:";

		if (!istatus)
			continue;

        if (!changes_in_index) {
            std::cout   << "# Changes to be committed:\n"
                        << "#   (use \"git reset HEAD <file>...\" to unstage)\n"
                        << "#\n";
            changes_in_index = true;
        }

        std::cout << "#\t" << istatus << "  ";

        old_path = s.head_to_index->old_file.path;
        new_path = s.head_to_index->new_file.path;

		if (old_path && new_path && strcmp(old_path, new_path))
			std::cout << old_path << " -> " << new_path;
		else if (old_path)
			std::cout << old_path;
		else
			std::cout << new_path;

		std::cout << "\n";
	}

    if (changes_in_index)
        std::cout << "#\n";

    bool changed_in_workdir = false;

	/* print workdir changes to tracked files */

	for (size_t i = 0; i < maxi; ++i) {
        auto const & s = status[i];

        if (s.status == GIT_STATUS_CURRENT || !s.index_to_workdir)
			continue;

		const char *wstatus = nullptr;

        if (s.status & GIT_STATUS_WT_MODIFIED)
			wstatus = "modified: ";
        if (s.status & GIT_STATUS_WT_DELETED)
			wstatus = "deleted:  ";
        if (s.status & GIT_STATUS_WT_RENAMED)
			wstatus = "renamed:  ";
        if (s.status & GIT_STATUS_WT_TYPECHANGE)
			wstatus = "typechange:";

        if (!wstatus)
            continue;

        if (!changed_in_workdir) {
            std::cout   << "# Changes not staged for commit:\n"
                        << "#   (use \"git add" << (rm_in_workdir ? "/rm" : "") << "<file>...\" to update what will be committed)\n"
                        << "#   (use \"git checkout -- <file>...\" to discard changes in working directory)\n"
                        << "#\n";
            changed_in_workdir = true;
        }

        std::cout << "#\t" << wstatus << "  ";

        old_path = s.index_to_workdir->old_file.path;
        new_path = s.index_to_workdir->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path))
            std::cout << old_path << " -> " << new_path;
        else if (old_path)
            std::cout << old_path;
        else
            std::cout << new_path;
        std::cout << "\n";
    }

    if (changed_in_workdir)
        std::cout << "#\n";

    /* print untracked files */
    bool were_untracked_files = false;
	for (size_t i = 0; i < maxi; ++i) 
    {
        auto const & s = status[i];

        if (s.status == GIT_STATUS_WT_NEW)
        {
            if (!were_untracked_files)
            {
                std::cout   << "# Untracked files:\n"
                            << "#   (use \"git add <file>...\" to include in what will be committed)\n"
                            << "#\n";
                were_untracked_files = true;
            }

            std::cout << "#\t" << s.index_to_workdir->old_file.path << "\n";
        }
    }

    /* print ignored files */
    bool were_ignored_files = false;

    for (size_t i = 0; i < maxi; ++i) {
        auto const & s = status[i];

        if (s.status == GIT_STATUS_IGNORED) 
        {
            if (!were_ignored_files)
            {
                std::cout   << "# Ignored files:\n"
                            << "#   (use \"git add -f <file>...\" to include in what will be committed)\n"
                            << "#\n";
                were_ignored_files = true;
            }

            std::cout << "#\t" << s.index_to_workdir->old_file.path << "\n";
        }
    }

    if (!changes_in_index && changed_in_workdir)
        std::cout << "no changes added to commit (use \"git add\" and/or \"git commit -a\")\n";
}

void print_short(Repository const & repo, Status const & status)
{
    size_t maxi = status.entrycount();

    for (size_t i = 0; i < maxi; ++i) 
    {
        auto const & s = status[i];

        if (s.status == GIT_STATUS_CURRENT)
            continue;

        const char *a = nullptr;
        const char *b = nullptr;
        const char *c = nullptr;
        char istatus = ' ';
        char wstatus = ' ';
        const char *extra = "";

        if (s.status & GIT_STATUS_INDEX_NEW)
			istatus = 'A';
        if (s.status & GIT_STATUS_INDEX_MODIFIED)
			istatus = 'M';
        if (s.status & GIT_STATUS_INDEX_DELETED)
			istatus = 'D';
        if (s.status & GIT_STATUS_INDEX_RENAMED)
			istatus = 'R';
        if (s.status & GIT_STATUS_INDEX_TYPECHANGE)
			istatus = 'T';

        if (s.status & GIT_STATUS_WT_NEW) {
			if (istatus == ' ')
				istatus = '?';
			wstatus = '?';
		}
        if (s.status & GIT_STATUS_WT_MODIFIED)
			wstatus = 'M';
        if (s.status & GIT_STATUS_WT_DELETED)
			wstatus = 'D';
        if (s.status & GIT_STATUS_WT_RENAMED)
			wstatus = 'R';
        if (s.status & GIT_STATUS_WT_TYPECHANGE)
			wstatus = 'T';

        if (s.status & GIT_STATUS_IGNORED) {
			istatus = '!';
			wstatus = '!';
		}

		if (istatus == '?' && wstatus == '?')
			continue;

        if (s.index_to_workdir && s.index_to_workdir->new_file.mode == GIT_FILEMODE_COMMIT)
        {
            auto sm = repo.submodule_lookup(s.index_to_workdir->new_file.path);
            auto smstatus = sm.get_status();

            typedef Submodule::status status;

            if (contains(smstatus, status::wd_modified))
                extra = " (new commits)";
            else if (contains(smstatus, status::wd_index_modified))
                extra = " (modified content)";
            else if (contains(smstatus, status::wd_wd_modified))
                extra = " (modified content)";
            else if (contains(smstatus, status::wd_untracked))
                extra = " (untracked content)";
        }

        if (s.head_to_index) {
            a = s.head_to_index->old_file.path;
            b = s.head_to_index->new_file.path;
		}
        if (s.index_to_workdir) {
			if (!a)
                a = s.index_to_workdir->old_file.path;
			if (!b)
                b = s.index_to_workdir->old_file.path;
            c = s.index_to_workdir->new_file.path;
		}

        std::cout << istatus << wstatus << ' ' << a;
        if (istatus == 'R')
            std::cout << ' ' << b;
        if (wstatus == 'R')
            std::cout << ' ' << c;
        std::cout << extra << "\n";
    }

    for (size_t i = 0; i < maxi; ++i)
    {
        auto const & s = status[i];

        if (s.status == GIT_STATUS_WT_NEW)
            std::cout << "?? " << s.index_to_workdir->old_file.path << "\n";
    }
}

int main(int argc, char *argv[])
{
	auto_git_initializer;
	
	int npaths = 0, format = FORMAT_DEFAULT, zterm = 0, showbranch = 0;
    Status::Options opt;
	const char *repodir = ".";

    const size_t MAX_PATHSPEC = 8;
    char * pathspec[MAX_PATHSPEC];

    opt.include_untracked().renames_head_to_index();

	for (int i = 1; i < argc; ++i) {
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
            opt.include_ignored();
		else if (!strcmp(argv[i], "-uno") ||
				 !strcmp(argv[i], "--untracked-files=no"))
            opt.exclude_untracked();
		else if (!strcmp(argv[i], "-unormal") ||
				 !strcmp(argv[i], "--untracked-files=normal"))
            opt.include_untracked();
		else if (!strcmp(argv[i], "-uall") ||
				 !strcmp(argv[i], "--untracked-files=all"))
            opt.include_untracked().recurse_untracked_dirs();
		else if (!strcmp(argv[i], "--ignore-submodules=all"))
            opt.exclude_submodules();
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
    if (npaths > 0)
        opt.set_pathspec(pathspec, npaths);

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

