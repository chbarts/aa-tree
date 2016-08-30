#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define _GNU_SOURCE             /* Enable getopt_long(). */
#include <getopt.h>
#include <stdbool.h>
#include "ggets.h"
#include "aa-tree.h"
#include "handle_ferr.h"

static void *freedata(void *data)
{
    char *ln = (char *) data;

    free(ln);
    return NULL;
}

static void cleanup(aa * tree)
{
    aa_traverse(tree, freedata, TRAV_IN);
    aa_free(tree);
}

static void error(aa * tree)
{
    fprintf(stderr, "memory error\n");
    cleanup(tree);
    exit(EXIT_FAILURE);
}

static void version(void)
{
    puts("nuniq version 0.1.0");
    puts("Copyright 2016 Chris Barts.");
    puts("nuniq comes with NO WARRANTY to the extent permitted by law.");
    puts("You may redistribute this software under the terms");
    puts("of the GNU General Public License.");
    puts("For more information, see the file named COPYING");
    puts("in this program's distribution file.");
}

static void usage(char *pname)
{
    printf("Usage: %s [OPTION] [FILE...]\n", pname);
    puts("Print uniqified version of file(s) to standard output without sorting them.");
    puts("With no files specified, uniqify standard input.");
    puts("");
    puts("  -h, --help        Print this help");
    puts("  -v, --version     Print version information");
    puts("");
    puts("Report bugs to <chbarts@gmail.com>.");
}

static int comparator(void *left, void *right)
{
    char *cl, *cr;
    int r;

    cl = (char *) left;
    cr = (char *) right;

    r = strcmp(cl, cr);

    if (r > 0) {
        r = 1;
    } else if (r < 0) {
        r = -1;
    }

    return r;
}

volatile int print = 1;

static int duplicate(void *orig, void *new)
{
    char *ln = (char *) new;

    free(ln);
    print = 0;
    return 0;
}

static void dofile(char *fname, FILE * fin, aa * tree)
{
    char *ln;

    while (fggets(&ln, fin) == 0) {
        switch (aa_add(tree, (void *) ln, duplicate)) {
        case 0:                /* No error */
            break;
        case 2:                /* Memory error */
            free(ln);
            error(tree);
            break;
        default:               /* Can't happen */
            fprintf(stderr, "Can't happen\n");
            free(ln);
            cleanup(tree);
            exit(EXIT_FAILURE);
            break;
        }

        if (1 == print) {
            puts(ln);
        } else {
            print = 1;
        }
    }

    if (ferror(fin) != 0)
        handle_ferr(fname, "nuniq");
}

int main(int argc, char *argv[])
{
    aa *tree;
    char *pnam;
    FILE *fin;
    int i, lind, c;
    extern int optind;
    struct option longopts[] = {
        {"help", 0, 0, 0},
        {"version", 0, 0, 0},
        {0, 0, 0, 0}
    };

    if (1 == argc) {
        if ((tree = aa_new(comparator)) == NULL) {
            fprintf(stderr, "aa_new() error\n");
            exit(EXIT_FAILURE);
        }

        dofile("stdin", stdin, tree);
        cleanup(tree);
        return 0;
    }

    pnam = argv[0];

    while ((c = getopt_long(argc, argv, "hv", longopts, &lind)) != -1) {
        switch (c) {
        case 0:
            switch (lind) {
            case 0:
                usage(pnam);
                return 0;
                break;
            case 1:
                version();
                return 0;
                break;
            default:
                usage(pnam);
                exit(EXIT_FAILURE);
                break;
            }

            break;
        case 'h':
            usage(pnam);
            return 0;
            break;
        case 'v':
            version();
            return 0;
            break;
        default:
            usage(pnam);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if ((tree = aa_new(comparator)) == NULL) {
        fprintf(stderr, "aa_new() error\n");
        exit(EXIT_FAILURE);
    }

    if (optind >= argc) {
        dofile("stdin", stdin, tree);
    } else {
        for (i = optind; i < argc; i++) {
            if ((fin = fopen(argv[i], "rb")) == NULL) {
                handle_ferr(argv[i], "nuniq");
                continue;
            }

            dofile(argv[i], fin, tree);

            if (fclose(fin) == EOF) {
                handle_ferr(argv[i], "nuniq");
            }
        }
    }

    cleanup(tree);
    return 0;
}
