#include <cassert>

#include "git2cpp/diff.h"
#include "git2cpp/tree.h"
#include "git2cpp/repo.h"

using namespace git;

static
uint32_t raw(Diff::option o)
{
	return static_cast<uint32_t>(o);
}

static
uint32_t raw(Diff::find_option o)
{
	return static_cast<uint32_t>(o);
}

namespace git
{
	Diff::option operator ~ (Diff::option o)
	{
		return Diff::option(~raw(o));
	}

	Diff::option operator | (Diff::option a, Diff::option b)
	{
		return Diff::option(raw(a) | raw(b));
	}

	Diff::option operator & (Diff::option a, Diff::option b)
	{
		return Diff::option(raw(a) & raw(b));
	}


		Diff::find_option operator | (Diff::find_option a, Diff::find_option b)
	{
		return Diff::find_option(raw(a) | raw(b));
	}

	Diff::find_option operator & (Diff::find_option a, Diff::find_option b)
	{
		return Diff::find_option(raw(a) & raw(b));
	}

	Diff::find_option operator ~ (Diff::find_option o)
	{
		return Diff::find_option(~raw(o));
	}

} // namespace git

static
git_diff_format_t convert(Diff::format f)
{
	switch (f)
	{
		case Diff::format::name_only:     	return GIT_DIFF_FORMAT_NAME_ONLY;
		case Diff::format::name_status:   	return GIT_DIFF_FORMAT_NAME_STATUS;
		case Diff::format::patch:         	return GIT_DIFF_FORMAT_PATCH;
		case Diff::format::patch_header:  	return GIT_DIFF_FORMAT_PATCH_HEADER;
		case Diff::format::raw:           	return GIT_DIFF_FORMAT_RAW;
	}
}

// static
Diff	Diff::diff_to_index(Repository const & repo, Tree & t, Diff::option const & optmask)
{
	git_diff_options	opts = GIT_DIFF_OPTIONS_INIT;

	opts.flags = raw(optmask);
	
	git_diff * diff;
	const auto op_res = git_diff_tree_to_index(&diff, repo.ptr(), t.ptr(), nullptr, &opts);
	assert(op_res == 0);
	return Diff(diff);
}

// static
Diff	Diff::diff_index_to_workdir(Repository const & repo, Diff::option const & optmask)
{
	git_diff_options	opts = GIT_DIFF_OPTIONS_INIT;

	opts.flags = raw(optmask);
	
	git_diff * diff;
	const auto op_res = git_diff_index_to_workdir(&diff, repo.ptr(), nullptr, &opts);
	assert(op_res == 0);
	return Diff(diff);
}

static
Diff	diff_tree_to_tree(Repository const & repo, Tree & a, Tree & b, Diff::option const & optmask)
{
	git_diff_options	opts = GIT_DIFF_OPTIONS_INIT;

	opts.flags = raw(optmask);

	git_diff * diff;
	const auto	op_res = git_diff_tree_to_tree(&diff, repo.ptr(), a.ptr(), b.ptr(), &opts);
	assert(op_res == 0);
	return Diff(diff);
}

	Diff::Diff(Repository const &repo, Tree &a, Tree &b, option const &optmask)
		: Diff(diff_tree_to_tree(repo, a, b, optmask))
{
}

void	Diff::find_similar(find_option findmask)
{
	git_diff_find_options	opts = GIT_DIFF_FIND_OPTIONS_INIT;

	opts.flags = raw(findmask);

	git_diff_find_similar(diff_, &opts);
}

Diff&	Diff::merge(Diff const & other)
{
	git_diff_merge(diff_, other.diff_);
	return *this;
}

size_t	Diff::deltas_num() const
{
	return git_diff_num_deltas(diff_);
}

static
int	apply_callback(git_diff_delta const * delta, const git_diff_hunk *hunk_p, git_diff_line const * line, void * payload )
{
	assert(delta);
	assert(line);
	
	auto cb = reinterpret_cast<Diff::print_callback_t const *>(payload);
	(*cb)(*delta, hunk_p, *line);
	
	return 0;
}
        
void	Diff::print(format fmt, print_callback_t print_callback) const
{
	git_diff_print(diff_, convert(fmt), &apply_callback, &print_callback);		// not correct context?
}
