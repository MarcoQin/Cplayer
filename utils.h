#ifndef _utils_h_
#define _utils_h_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *parse_p(char *s);
int index_of(char[], char[]);
char *extract_file_name(char *path);
int extract_song_id(const char *name);
#endif
