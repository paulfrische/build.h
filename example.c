#include "build.h"
#include <stdio.h>

int main(void)
{
    BHPathArray files = bh_make_path_arr();

    bh_read_dir_recursive("/home/paul/notes", &files);

    for (size_t i = 0; i < files.len; i++) {
        puts(files.paths[i]);
    }

    bh_free_path_arr(&files);

    return EXIT_SUCCESS;
}
