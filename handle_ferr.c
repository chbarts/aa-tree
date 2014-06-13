#include <stdio.h>
#include <linux/limits.h>

void handle_ferr(char *fname, char *pname)
{
#define MSG "%s: error on %s"
    char buf[2 * PATH_MAX + sizeof(MSG) + 1];

    snprintf(buf, sizeof(buf), MSG, pname, fname);
    perror(buf);
}
