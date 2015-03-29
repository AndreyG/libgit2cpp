#include <git2.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "git2cpp/repo.h"
#include "git2cpp/initializer.h"

enum print_options {
	SKIP = 1,
	VERBOSE = 2,
	UPDATE = 4,
};

void init_array(git_strarray *array, int argc, char **argv)
{
	unsigned int i;

	array->count = argc;
	array->strings = reinterpret_cast<char **>(malloc(sizeof(char*) * array->count));
	assert(array->strings!=NULL);

	for(i=0; i<array->count; i++) {
		array->strings[i]=argv[i];
	}

	return;
}

int print_matched_cb(   const char *path, const char * /*matched_pathspec*/,
                        git::Repository const & repo, print_options options )
{
	git_status_t status = repo.file_status(path);

	int ret;
	if (status & GIT_STATUS_WT_MODIFIED ||
	         status & GIT_STATUS_WT_NEW) {
		printf("add '%s'\n", path);
		ret = 0;
	} else {
		ret = 1;
	}

	if (options & SKIP) {
		ret = 1;
	}

	return ret;
}

void print_usage(void)
{
	fprintf(stderr, "usage: add [options] [--] file-spec [file-spec] [...]\n\n");
	fprintf(stderr, "\t-n, --dry-run    dry run\n");
	fprintf(stderr, "\t-v, --verbose    be verbose\n");
	fprintf(stderr, "\t-u, --update     update tracked files\n");
}

int main (int argc, char** argv)
{
    using namespace std::placeholders;

	git_strarray array = {0};

	int i = 1, options = 0;
	for (; i < argc; ++i) {
		if (argv[i][0] != '-') {
			break;
		}
		else if(!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v")) {
			options |= VERBOSE;
		}
		else if(!strcmp(argv[i], "--dry-run") || !strcmp(argv[i], "-n")) {
			options |= SKIP;
		}
		else if(!strcmp(argv[i], "--update") || !strcmp(argv[i], "-u")) {
			options |= UPDATE;
		}
		else if(!strcmp(argv[i], "-h")) {
			print_usage();
			break;
		}
		else if(!strcmp(argv[i], "--")) {
			i++;
			break;
		}
		else {
			fprintf(stderr, "Unsupported option %s.\n", argv[i]);
			print_usage();
			return 1;
		}
	}

	if (argc<=i) {
		print_usage();
		return 1;
	}

	init_array(&array, argc-i, argv+i);

    git::Initializer threads_initializer;

	git::Repository repo(".");

    git::Index::matched_path_callback_t cb;
	if (options&VERBOSE || options&SKIP) {
        cb = std::bind(&print_matched_cb, _1, _2, std::cref(repo), static_cast<print_options>(options)); 
	}

	git::Index index = repo.index();
	if (options&UPDATE) {
		index.update_all(array, cb);
	} else {
		index.add_all(array, cb);
	}

	index.write();

	return 0;
}
