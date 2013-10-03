#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <boost/optional.hpp>

#include "git2cpp/repo.h"
#include "git2cpp/pathspec.h"
#include "git2cpp/threads_initializer.h"

static void usage(const char *message, const char *arg)
{
	if (message && arg)
		fprintf(stderr, "%s: %s\n", message, arg);
	else if (message)
		fprintf(stderr, "%s\n", message);
	fprintf(stderr, "usage: log [<options>]\n");
	exit(1);
}

struct log_state 
{
	std::string repodir = ".";
	boost::optional<git::Repository> repo;
    std::shared_ptr<git::RevWalker>  walker;
    int hide = 0;
    int sorting = GIT_SORT_TIME;
};

static void set_sorting(struct log_state *s, unsigned int sort_mode)
{
	if (!s->repo) {
        s->repo = boost::in_place(s->repodir);
	}

	if (!s->walker)
		s->walker = s->repo->rev_walker();

	if (sort_mode == GIT_SORT_REVERSE)
		s->sorting = s->sorting ^ GIT_SORT_REVERSE;
	else
		s->sorting = sort_mode | (s->sorting & GIT_SORT_REVERSE);

	s->walker->sort(s->sorting);
}

static void push_rev(struct log_state *s, git_object *obj, int hide)
{
	hide = s->hide ^ hide;

	if (!s->walker) {
		s->walker = s->repo->rev_walker();
		s->walker->sort(s->sorting);
	}

	if (!obj)
		s->walker->push_head();
	else if (hide)
		s->walker->hide(git_object_id(obj));
	else
		s->walker->push(git_object_id(obj));

	git_object_free(obj);
}

static int add_revision(struct log_state *s, const char *revstr)
{
	int hide = 0;

	if (!s->repo) {
        s->repo = boost::in_place(s->repodir);
	}

	if (!revstr) {
		push_rev(s, NULL, hide);
		return 0;
	}

	git_revspec revs;
	if (*revstr == '^') {
		revs.flags = GIT_REVPARSE_SINGLE;
		hide = !hide;

		if (s->repo->revparse_single(revs.from, revstr + 1) < 0)
			return -1;
	} else if (s->repo->revparse(revs, revstr) < 0)
		return -1;

	if ((revs.flags & GIT_REVPARSE_SINGLE) != 0)
		push_rev(s, revs.from, hide);
	else {
		push_rev(s, revs.to, hide);

		if ((revs.flags & GIT_REVPARSE_MERGE_BASE) != 0) {
			git_oid base;
            s->repo->merge_base(base, git_object_id(revs.from), git_object_id(revs.to));
			s->repo->object_lookup(revs.to, base, GIT_OBJ_COMMIT);

			push_rev(s, revs.to, hide);
		}

		push_rev(s, revs.from, !hide);
	}

	return 0;
}

static void print_time(const git_time *intime, const char *prefix)
{
	char sign, out[32];
	struct tm intm;
	int offset, hours, minutes;
	time_t t;

	offset = intime->offset;
	if (offset < 0) {
		sign = '-';
		offset = -offset;
	} else {
		sign = '+';
	}

	hours   = offset / 60;
	minutes = offset % 60;

	t = (time_t)intime->time + (intime->offset * 60);

	gmtime_r(&t, &intm);
	strftime(out, sizeof(out), "%a %b %e %T %Y", &intm);

	printf("%s%s %c%02d%02d\n", prefix, out, sign, hours, minutes);
}

static void print_commit(git::Commit const & commit)
{
	char buf[GIT_OID_HEXSZ + 1];
	int i, count;
	const git_signature *sig;
	const char *scan, *eol;

	git_oid_tostr(buf, sizeof(buf), commit.id());
	printf("commit %s\n", buf);

	if ((count = (int)commit.parents_num()) > 1) {
		printf("Merge:");
		for (i = 0; i < count; ++i) {
			git_oid_tostr(buf, 8, commit.parent_id(i));
			printf(" %s", buf);
		}
		printf("\n");
	}

	if ((sig = commit.author()) != NULL) {
		printf("Author: %s <%s>\n", sig->name, sig->email);
		print_time(&sig->when, "Date:   ");
	}
	printf("\n");

	for (scan = commit.message(); scan && *scan; ) {
		for (eol = scan; *eol && *eol != '\n'; ++eol) /* find eol */;

		printf("    %.*s\n", (int)(eol - scan), scan);
		scan = *eol ? eol + 1 : NULL;
	}
	printf("\n");
}

static int print_diff(
	const git_diff_delta *delta,
	const git_diff_range *range,
	char usage,
	const char *line,
	size_t line_len,
	void *data)
{
	(void)delta; (void)range; (void)usage; (void)line_len; (void)data;
	fputs(line, stdout);
	return 0;
}

static int match_int(int *value, const char *arg, int allow_negative)
{
	char *found;
	*value = (int)strtol(arg, &found, 10);
	return (found && *found == '\0' && (allow_negative || *value >= 0));
}

static int match_int_arg(
	int *value, const char *arg, const char *pfx, int allow_negative)
{
	size_t pfxlen = strlen(pfx);
	if (strncmp(arg, pfx, pfxlen) != 0)
		return 0;
	if (!match_int(value, arg + pfxlen, allow_negative))
		usage("Invalid value after argument", arg);
	return 1;
}

static int match_with_parent(git::Commit const & commit, int i, git_diff_options const & opts)
{
    auto parent = commit.parent(i);
    auto diff = git::diff(commit.owner(), parent.tree(), commit.tree(), opts);

    return diff.deltas_num() > 0;
}

struct log_options {
	int show_diff;
	int skip, limit;
	int min_parents, max_parents;
	git_time_t before;
	git_time_t after;
	char *author;
	char *committer;
};

int parse_options   ( int argc, char *argv[]
                    , log_state & s
                    , log_options & opt
                    , int count
                    )
{
    int i = 1;
	for (; i < argc; ++i) {
		char* a = argv[i];

		if (a[0] != '-') {
			if (!add_revision(&s, a))
				++count;
			else /* try failed revision parse as filename */
				break;
		} else if (!strcmp(a, "--")) {
			++i;
			break;
		}
		else if (!strcmp(a, "--date-order"))
			set_sorting(&s, GIT_SORT_TIME);
		else if (!strcmp(a, "--topo-order"))
			set_sorting(&s, GIT_SORT_TOPOLOGICAL);
		else if (!strcmp(a, "--reverse"))
			set_sorting(&s, GIT_SORT_REVERSE);
		else if (!strncmp(a, "--git-dir=", strlen("--git-dir=")))
			s.repodir = a + strlen("--git-dir=");
		else if (match_int_arg(&opt.skip, a, "--skip=", 0))
			/* found valid --skip */;
		else if (match_int_arg(&opt.limit, a, "--max-count=", 0))
			/* found valid --max-count */;
		else if (a[1] >= '0' && a[1] <= '9') {
			if (!match_int(&opt.limit, a + 1, 0))
				usage("Invalid limit on number of commits", a);
		} else if (!strcmp(a, "-n")) {
			if (i + 1 == argc || !match_int(&opt.limit, argv[i + 1], 0))
				usage("Argument -n not followed by valid count", argv[i + 1]);
			else
				++i;
		}
		else if (!strcmp(a, "--merges"))
			opt.min_parents = 2;
		else if (!strcmp(a, "--no-merges"))
			opt.max_parents = 1;
		else if (!strcmp(a, "--no-min-parents"))
			opt.min_parents = 0;
		else if (!strcmp(a, "--no-max-parents"))
			opt.max_parents = -1;
		else if (match_int_arg(&opt.max_parents, a, "--max-parents=", 1))
			/* found valid --max-parents */;
		else if (match_int_arg(&opt.min_parents, a, "--min-parents=", 0))
			/* found valid --min_parents */;
		else if (!strcmp(a, "-p") || !strcmp(a, "-u") || !strcmp(a, "--patch"))
			opt.show_diff = 1;
		else
			usage("Unsupported argument", a);
	}

    return i;
}

int main(int argc, char *argv[])
{
	log_options opt;
	memset(&opt, 0, sizeof(opt));
	opt.max_parents = -1;
 	opt.limit = -1;
	
    log_state s;

    int count = 0;
    int parsed_options_num = parse_options(argc, argv, s, opt, count);

    git::ThreadsInitializer threads_initializer;

	if (!count)
		add_revision(&s, NULL);

	git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;
	diffopts.pathspec.strings = &argv[parsed_options_num];
	diffopts.pathspec.count   = argc - parsed_options_num;
    git::Pathspec ps(diffopts.pathspec);

	count = 0;
    int printed = 0;

	while (auto oid = s.walker->next())
    {
		git::Commit commit = s.repo->commit_lookup(*oid);

		int parents = commit.parents_num();
		if (parents < opt.min_parents)
			continue;
		if (opt.max_parents > 0 && parents > opt.max_parents)
			continue;

		if (diffopts.pathspec.count > 0) 
        {
			int unmatched = parents;

			if (parents == 0) 
            {
				if (commit.tree().pathspec_match(GIT_PATHSPEC_NO_MATCH_ERROR, ps) != 0)
					unmatched = 1;
			} 
            else if (parents == 1) 
            {
				unmatched = match_with_parent(commit, 0, diffopts) ? 0 : 1;
			} 
            else 
            {
				for (int i = 0; i < parents; ++i) 
                {
					if (match_with_parent(commit, i, diffopts))
						unmatched--;
				}
			}

			if (unmatched > 0)
				continue;
		}

		if (count++ < opt.skip)
			continue;
		if (opt.limit != -1 && printed++ >= opt.limit) {
			break;
		}

		print_commit(commit);

		if (opt.show_diff) 
        {
			if (parents > 1)
				continue;
            git::Tree b = commit.tree();
            git::Tree a;
			if (parents == 1) 
            {
                a = commit.parent(0).tree();
			}

			git::diff(commit.owner(), a, b, diffopts).print_patch(print_diff);
		}
	}

	return 0;
}
