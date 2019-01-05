#include <stdio.h>
#include <string.h>

#include <git2/index.h>

#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

using namespace git;

int main(int argc, char ** argv)
{
    const char * dir = ".";

    Initializer threads_initalizer;

    if (argc > 1)
        dir = argv[1];
    if (!dir || argc > 2)
    {
        fprintf(stderr, "usage: showindex [<repo-dir>]\n");
        return 1;
    }

    size_t dirlen = strlen(dir);
    Index index = (dirlen > 5 && strcmp(dir + dirlen - 5, "index") == 0)
                      ? Index(dir)
                      : Repository(dir).index();

    size_t ecount = index.entrycount();
    if (!ecount)
        printf("Empty index\n");

    for (size_t i = 0; i < ecount; ++i)
    {
        auto e = index[i];

        char out[41];
        out[40] = '\0';
        git_oid_fmt(out, &e->id);

        printf("File Path: %s\n", e->path);
        printf("    Stage: %d\n", git_index_entry_stage(e));
        printf(" Blob SHA: %s\n", out);
        printf("File Mode: %07o\n", e->mode);
        printf("File Size: %d bytes\n", (int)e->file_size);
        printf("Dev/Inode: %d/%d\n", (int)e->dev, (int)e->ino);
        printf("  UID/GID: %d/%d\n", (int)e->uid, (int)e->gid);
        printf("    ctime: %d\n", (int)e->ctime.seconds);
        printf("    mtime: %d\n", (int)e->mtime.seconds);
        printf("\n");
    }

    return 0;
}
