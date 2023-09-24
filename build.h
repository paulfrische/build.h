/*
Copyright © 2023 Paul Frische
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions: The above copyright
notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/

#ifndef BUILD_H_
#define BUILD_H_

#define BH_ASSERT assert
#define BH_MALLOC malloc
#define BH_REALLOC realloc
#define BH_FREE free

#define BH_LOG_LEVEL BH_INFO

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    BH_INFO,
    BH_WARN,
    BH_ERROR,

    BH_LOG_LEVEL_MAX,
} BHLogLevel;

const char* bh_level_name[] = { "INFO", "WARN", "ERROR" };

#define BH_LOG(level, ...)                                                                                             \
    if (level <= BH_LOG_LEVEL) {                                                                                       \
        printf("%s", bh_level_name[level]);                                                                            \
        printf(__VA_ARGS__);                                                                                           \
        printf("\n");                                                                                                  \
    }

typedef char* BHPath;

typedef struct {
    BHPath* paths;
    size_t len;
} BHPathArray;

static inline BHPathArray bh_make_path_arr()
{
    BHPathArray a = {
        .paths = NULL,
        .len = 0,
    };
    return a;
}

static inline void bh_free_path_arr(BHPathArray* arr)
{
    for (size_t i = 0; i < arr->len; i++) {
        BH_FREE(arr->paths[i]);
    }
    BH_FREE(arr->paths);
}

static inline void bh_add_path(BHPathArray* arr, BHPath path)
{
    arr->paths = (BHPath*)BH_REALLOC(arr->paths, (arr->len + 1) * sizeof(BHPath));
    arr->paths[arr->len] = path;
    arr->len++;
}

typedef struct {
    char* out;
    char* cc;
    BHPathArray src;
    BHPathArray include;
    /* string_list_t cflags; */
    /* string_list_t ldflags; */
    /* string_list_t defines; */
} target_t;

static inline bool bh_read_dir(const char* path, BHPathArray* files, BHPathArray* dirs)
{
    DIR* dir = opendir(path);
    if (dir == NULL) {
        char log[1000];
        perror(log);
        puts(log);
        BH_LOG(BH_ERROR, "could not open dir %s", path);
        return false;
    }

    struct dirent* dr = readdir(dir);
    if (dr == NULL) {
        char log[1000];
        perror(log);
        puts(log);
        BH_LOG(BH_ERROR, "could not read dir %s", path);
    }
    while (dr != NULL) {
        if (dr->d_type == DT_REG) {
            char* entry = (char*)BH_MALLOC(strlen(path) + strlen(dr->d_name) + 2);
            entry[0] = 0;
            strcat(entry, path);
            strcat(entry, "/");
            strcat(entry, dr->d_name);
            bh_add_path(files, entry);
        } else if (dr->d_type == DT_DIR && dr->d_name[0] != '.') {
            char* entry = (char*)BH_MALLOC(strlen(path) + strlen(dr->d_name) + 2);
            entry[0] = 0;
            strcat(entry, path);
            strcat(entry, "/");
            strcat(entry, dr->d_name);
            bh_add_path(dirs, entry);
        }

        dr = readdir(dir);
    }
    closedir(dir);

    return true;
}

static inline bool bh_read_dir_recursive(const char* path, BHPathArray* files)
{
    BHPathArray dirs = bh_make_path_arr();
    if (!bh_read_dir(path, files, &dirs)) {
        return false;
    }
    for (size_t i = 0; i < dirs.len; i++) {
        bh_read_dir_recursive(dirs.paths[i], files);
    }
    bh_free_path_arr(&dirs);
    return true;
}

#endif // !BUILD_H_
