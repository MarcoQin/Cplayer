#include "utils.h"

/* convert string's space to \space to fit bash command line */
char *parse_p(char *s)
{
    int i = 0;
    int j = 0;
    int total = 0;
    while(s[i] != '\0')
    {
        if(s[i] == ' ')
            total += 1;
        i++;
    }
    i = 0;
    char *r = malloc(1 + total + strlen(s));
    while(s[i] != '\0')
    {
        if(s[i] == ' ')
        {
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

