#include <stdio.h>
#if defined(__MINGW32__) || defined(__MINGW64__)
#include <limits.h>
#else
#include <linux/limits.h>
#endif

void handle_ferr(char *fname, char *pname)
{
#define MSG "%s: error on %s"
    char buf[2 * PATH_MAX + sizeof(MSG) + 1];

    snprintf(buf, sizeof(buf), MSG, pname, fname);
    perror(buf);
}
