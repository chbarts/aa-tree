#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ggets.h"
#include "handle_ferr.h"
#include "aa-tree.h"

static void print(void *data)
{
    char *ln = (char *) data;
    printf(ln);
}

static void cleanup(aa * tree)
{
    aa_freedata(tree);
    aa_free(tree);
}

static void error(aa * tree)
{
    fprintf(stderr, "memory error\n");
    cleanup(tree);
    exit(EXIT_FAILURE);
}

static int comparator(void *left, void *right)
{
    char *l = (char *) left, *r = (char *) right;
    int n = strcmp(l, r);
    if (n < 0)
        return -1;
    else if (n > 0)
        return 1;
    else
        return 0;
}

static int duplicate(void *orig, void *new)
{
    free(new);
    return 0;
}

static void dofile(char *fname, FILE * fin, aa * tree)
{
    char *ln;

    while (fggets(&ln, fin) == 0) {
        switch (aa_add(tree, ln, duplicate)) {
        case 0:                /* no error */
            break;
        case 2:                /* memory error */
            free(ln);
            error(tree);
            break;
        default:               /* can't happen */
            fprintf(stderr, "can't happen\n");
            free(ln);
            cleanup(tree);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (ferror(fin) != 0)
        handle_ferr(fname, "aa-test1");
}

int main(int argc, char *argv[])
{
    aa *tree;
    FILE *fin;
    int i;

    if ((tree = aa_new(comparator)) == NULL) {
        fprintf(stderr, "aa_new error\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 1) {
        dofile("stdin", stdin, tree);
    } else {
        for (i = 1; i < argc; i++) {
            if ((fin = fopen(argv[i], "rb")) == NULL) {
                handle_ferr(argv[i], "aa-test1");
                continue;
            }

            dofile(argv[i], fin, tree);

            if (fclose(fin) == EOF) {
                handle_ferr(argv[i], "aa-test1");
            }
        }
    }

    aa_print(tree, stdout, print);

    cleanup(tree);
    return 0;
}
