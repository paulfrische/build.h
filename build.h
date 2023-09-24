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

//////////
// UTIL //
//////////

static inline char* bh_join_strings(const char** strs, size_t count)
{
    size_t size = count;
    for (size_t i = 0; i < count; i++) {
        size += strlen(strs[i]);
    }

    char* final = (char*)BH_MALLOC(size);
    final[0] = 0;
    strcat(final, strs[0]);
    for (size_t i = 1; i < count; i++) {
        strcat(final, " ");
        strcat(final, strs[i]);
    }

    return final;
}

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

///////////
// PATHS //
///////////

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
    BHPath p = (BHPath)BH_MALLOC(strlen(path) + 1);
    strcpy(p, path);
    arr->paths = (BHPath*)BH_REALLOC(arr->paths, (arr->len + 1) * sizeof(BHPath));
    arr->paths[arr->len] = p;
    arr->len++;
}

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
            free(entry);
        } else if (dr->d_type == DT_DIR && dr->d_name[0] != '.') {
            char* entry = (char*)BH_MALLOC(strlen(path) + strlen(dr->d_name) + 2);
            entry[0] = 0;
            strcat(entry, path);
            strcat(entry, "/");
            strcat(entry, dr->d_name);
            bh_add_path(dirs, entry);
            free(entry);
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

typedef bool (*filter_function)(const char*);

static inline BHPathArray bh_filter_paths(BHPathArray* paths, filter_function f)
{
    BHPathArray filtered = bh_make_path_arr();
    for (size_t i = 0; i < paths->len; i++) {
        if (f(paths->paths[i])) {
            bh_add_path(&filtered, paths->paths[i]);
        }
    }

    return filtered;
}

///////////
// FLAGS //
///////////

typedef char* BHFlag;

typedef struct {
    BHFlag* flags;
    size_t len;
} BHFlagArray;

static inline BHFlagArray bh_make_flag_arr()
{
    BHFlagArray a = {
        .flags = NULL,
        .len = 0,
    };
    return a;
}

static inline void bh_free_flag_arr(BHFlagArray* arr)
{
    for (size_t i = 0; i < arr->len; i++) {
        BH_FREE(arr->flags[i]);
    }
    BH_FREE(arr->flags);
}

static inline void bh_add_flag(BHFlagArray* arr, BHFlag flag)
{
    BHFlag f = (BHFlag)BH_MALLOC(strlen(flag) + 1);
    strcpy(f, flag);
    arr->flags = (BHFlag*)BH_REALLOC(arr->flags, (arr->len + 1) * sizeof(BHFlag));
    arr->flags[arr->len] = f;
    arr->len++;
}

///////////////
// COMPILING //
///////////////

typedef struct {
    char* out;
    char* cc;
    BHPathArray src;
    BHPathArray include;
    BHFlagArray cflags;
    BHFlagArray ldflags;
    BHFlagArray defines;
} target_t;

#endif // !BUILD_H_
