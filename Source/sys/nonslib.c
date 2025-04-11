#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int stricmp(const char *d, const char *s)
{
    int c1, c2;

    for (;;)
    {   c1 = *d++;
        c2 = *s++;
        if (c1 == '\0' && c2 == '\0')
            return 0;
        if (c1 == '\0')
            return -1;
        if (c2 == '\0')
            return 1;
        c1 = toupper(c1) - toupper(c2);
        if (c1 != 0)
            return c1;
    }
}

char *strlwr(char *s)
{
    int c;
    char *p = s;

    while ((c = *p) != '\0')
        *p++ = (char)tolower(c);
    return s;
}

char *strupr(char *s)
{
    int c;
    char *p = s;

    while ((c = *p) != '\0')
        *p++ = (char)toupper(c);
    return s;
}

/* end */
