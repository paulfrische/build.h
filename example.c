#include "build.h"
#include <stdio.h>

bool ends_with_c(const char* p) { return !strcmp(&p[strlen(p) - 2], ".c"); }

int main(void)
{
    BHPathArray files = bh_make_path_arr();
    BHFlagArray flags = bh_make_flag_arr();

    bh_add_flag(&flags, "-Wall");
    bh_add_flag(&flags, "-Wextra");

    bh_read_dir_recursive("/home/paul/d/programming/c/c-codebase", &files);
    BHPathArray filtered = bh_filter_paths(&files, ends_with_c);

    for (size_t i = 0; i < filtered.len; i++) {
        puts(filtered.paths[i]);
    }

    char* joint_paths = bh_join_strings(filtered.paths, filtered.len);
    puts(joint_paths);

    char* joint_flags = bh_join_strings(flags.flags, flags.len);
    puts(joint_flags);

    free(joint_paths);
    free(joint_flags);
    bh_free_path_arr(&files);
    bh_free_path_arr(&filtered);
    bh_free_flag_arr(&flags);

    return EXIT_SUCCESS;
}
