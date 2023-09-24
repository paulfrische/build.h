#include "build.h"
#include <stdio.h>

int main(void)
{
    BHPathArray a = bh_make_path_arr();
    BHPathArray b = bh_make_path_arr();
    bh_read_dir("/home/paul/notes", &a, &b);

    printf("%lu\n", a.len);
    printf("%lu\n", b.len);

    for (size_t i = 0; i < a.len; i++) {
        puts(a.paths[i]);
    }

    for (size_t i = 0; i < b.len; i++) {
        puts(b.paths[i]);
    }

    bh_free_path_arr(&a);
    bh_free_path_arr(&b);

    return EXIT_SUCCESS;
}
