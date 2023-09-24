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

typedef struct {
    char** content;
    unsigned int size;
} string_list_t;

string_list_t bh_string_list_init()
{
    string_list_t l = {
        .content = (char**)malloc(sizeof(char*)),
        .size = 0,
    };
    return l;
}

void bh_string_list_add(string_list_t* l, char* str)
{
    l->content = (char**)BH_REALLOC(l->content, l->size + sizeof(char*));
    *(l->content + l->size) = str;
    l->size++;
}

void bh_string_list_free(string_list_t* l) { BH_FREE(l->content); }

char* bh_string_from_list(string_list_t* l)
{
    unsigned int size = 1;
    for (unsigned int i = 0; i < l->size; i++) {
        puts(l->content[i]);
        size += strlen(*(l->content + i));
    }

    printf("string size: %i\n", size);
    char* s = (char*)BH_MALLOC(size);
    memset(s, 0, size);
    for (unsigned int i = 0; i < l->size; i++) {
        strcat(s, *(l->content + i));
    }
    return s;
}

typedef struct {
    char* out;
    char* cc;
    string_list_t src;
    string_list_t include;
    string_list_t cflags;
    string_list_t ldflags;
    string_list_t defines;
} target_t;

target_t bh_target_init()
{
    target_t t = {
        .out = NULL,
        .cc = "cc",
        .include = bh_string_list_init(),
        .cflags = bh_string_list_init(),
        .ldflags = bh_string_list_init(),
        .defines = bh_string_list_init(),
    };

    return t;
}

bool bh_read_dir(const char* path, string_list_t* files, string_list_t* dirs)
{
    DIR* dir = opendir(path);
    if (dir == NULL) {
        char log[1000];
        perror(&log);
        puts(log);
        BH_LOG(BH_ERROR, "could not open dir %s", path);
        return false;
    }

    struct dirent* dr = readdir(dir);
    if (dr == NULL) {
        char log[1000];
        perror(&log);
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
            bh_string_list_add(files, entry);
        } else if (dr->d_type == DT_DIR && dr->d_name[0] != '.') {
            char* entry = (char*)BH_MALLOC(strlen(path) + strlen(dr->d_name) + 2);
            entry[0] = 0;
            strcat(entry, path);
            strcat(entry, "/");
            strcat(entry, dr->d_name);
            bh_string_list_add(dirs, entry);
        }

        dr = readdir(dir);
    }

    return true;
}

#endif // !BUILD_H_
