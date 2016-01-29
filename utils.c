#include "utils.h"

/* convert string's space to \space to fit bash command line */
char *parse_p(char *s) {
    int i = 0;
    int j = 0;
    int total = 0;
    while (s[i] != '\0') {
        if (s[i] == ' ')
            total += 1;
        i++;
    }
    i = 0;
    char *r = malloc(1 + total + strlen(s));
    while (s[i] != '\0') {
        if (s[i] == ' ') {
            r[j] = '\\';
            r[++j] = s[i];
        } else {
            r[j] = s[i];
        }
        i++;
        j++;
    }
    r[++j] = '\0';
    return r;
}

int index_of(char src[], char str[]) {
    int i, j, firstOcc;
    i = 0, j = 0;

    while (src[i] != '\0') {

        while (src[i] != str[0] && src[i] != '\0')
            i++;

        if (src[i] == '\0')
            return (-1);

        firstOcc = i;

        while (src[i] == str[j] && src[i] != '\0' && str[j] != '\0') {
            i++;
            j++;
        }

        if (str[j] == '\0')
            return (firstOcc);
        if (src[i] == '\0')
            return (-1);

        i = firstOcc + 1;
        j = 0;
    }
}

char *extract_file_name(char *path) {
    int i, j, index;
    i = 0, j = 0, index = 0;
    while (path[i] != '\0') {
        if (path[i] == '/')
            index = i;
        i++;
    }
    char *r = malloc(strlen(path) - index + 1);
    index++;
    while (path[index] != '\0') {
        r[j++] = path[index++];
    }
    r[j] = '\0';
    return r;
}

int extract_song_id(char *name) {
    int i;
    i = 0;
    char id[5];
    while (name[i] != '.') {
        id[i] = name[i];
        i++;
    }
    id[i] = '\0';
    return atoi(id);
}
