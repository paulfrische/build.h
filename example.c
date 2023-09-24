#include "build.h"
#include <stdio.h>

int main(void)
{
    string_list_t a = bh_string_list_init();
    string_list_t b = bh_string_list_init();
    bh_read_dir("/home/paul/notes", &a, &b);

    printf("%i\n", a.size);
    printf("%i\n", b.size);

    puts(bh_string_from_list(&a));
    puts(bh_string_from_list(&b));

    bh_string_list_free(&a);
    bh_string_list_free(&b);

    puts("the end");
    return EXIT_SUCCESS;
}
