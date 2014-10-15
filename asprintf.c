#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static int vasprintf(char **ret, const char *fmt, va_list ap)
{
    int n;
    va_list ap2;

    va_copy(ap2, ap);

    if ((n = vsnprintf(NULL, 0, fmt, ap2)) < 0)
        goto error;

    va_end(ap2);

    if ((*ret = malloc(n + 1)) == NULL)
        goto error;

    if ((n = vsnprintf(*ret, n + 1, fmt, ap)) < 0) {
        free(*ret);
        goto error;
    }

    return n;

  error:
    *ret = NULL;
    return -1;
}

int asprintf(char **ret, const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vasprintf(ret, fmt, ap);
    va_end(ap);

    return n;
}
