#include "build.h"
#include <stdio.h>

bool ends_with_c(const char* p) { return !strcmp(&p[strlen(p) - 2], ".c"); }

int main(void)
{
    BHPathArray files = bh_make_path_arr();

    bh_read_dir_recursive("/home/paul/d/programming/c/c-codebase", &files);
    BHPathArray filtered = bh_filter_paths(&files, ends_with_c);

    for (size_t i = 0; i < filtered.len; i++) {
        puts(filtered.paths[i]);
    }

    char* f = bh_join_strings(filtered.paths, filtered.len);
    puts(f);

    free(f);
    bh_free_path_arr(&files);
    bh_free_path_arr(&filtered);

    return EXIT_SUCCESS;
}
