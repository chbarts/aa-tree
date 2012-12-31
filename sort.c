#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define _GNU_SOURCE             /* Enable getopt_long(). */
#include <getopt.h>
#include "ggets.h"
#include "handle_ferr.h"
#include "aa-tree.h"
#include "ll3.h"

static void *print(void *data)
{
    ll *node = (ll *) data;
    char *ln;

    ln = get_data(node);
    puts(ln);
    return data;
}

static void *printall(void *data)
{
    ll *node = (ll *) data;
    char *ln;
    int n;

    n = get_tag(node);
    ln = get_data(node);
    while (n-- > 0)
        puts(ln);

    return data;
}

static void *freedata(void *data)
{
    ll *node = (ll *) data;
    char *ln;

    ln = get_data(node);
    free(ln);
    free(node);
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

static int comparator(void *left, void *right)
{
    ll *lfl = (ll *) left, *rl = (ll *) right;
    char *l = get_data(lfl), *r = get_data(rl);
    int n = strcmp(l, r);
    if (n < 0)
        return -1;
    else if (n > 0)
        return 1;
    else
        return 0;
}

static int rcomparator(void *left, void *right)
{
    ll *lfl = (ll *) left, *rl = (ll *) right;
    char *l = get_data(lfl), *r = get_data(rl);
    int n = strcmp(l, r);
    if (n < 0)
        return 1;
    else if (n > 0)
        return -1;
    else
        return 0;
}

static int duplicate(void *orig, void *new)
{
    int n;
    ll *node = (ll *) orig, *node2 = (ll *) new;
    char *ln;

    n = get_tag(node);
    set_tag(node, n + 1);
    ln = get_data(node2);
    free(ln);
    free(node2);
    return 0;
}

static void dofile(char *fname, FILE * fin, aa * tree)
{
    char *ln;
    ll *lln;

    while (fggets(&ln, fin) == 0) {
        if ((lln = new_node(1, ln, NULL)) == NULL) {
            free(ln);
            error(tree);
        }

        switch (aa_add(tree, lln, duplicate)) {
        case 0:                /* no error */
            break;
        case 2:                /* memory error */
            free(ln);
            free(lln);
            error(tree);
            break;
        default:               /* can't happen */
            fprintf(stderr, "can't happen\n");
            free(ln);
            free(lln);
            cleanup(tree);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (ferror(fin) != 0)
        handle_ferr(fname, "sort");
}

static void version(void)
{
    puts("sort version 0.3");
    puts("Copyright 2012 Chris Barts.");
    puts("sort comes with NO WARRANTY to the extent permitted by law.");
    puts("You may redistribute this software under the terms");
    puts("of the GNU General Public License.");
    puts("All other files are in the public domain.");
    puts("For more information, see the file named COPYING");
    puts("in this program's distribution file.");
}

static void usage(char *pname)
{
    printf("Usage: %s [OPTION] [FILE...]\n", pname);
    puts("Sort files or, if no files on the command line, sort stdin.");
    puts("All files sorted together into one output.");
    puts("");
    puts("  -u, --uniqify     Only print identical lines once");
    puts("  -r, --reverse     Reverse the sense of the comparison");
    puts("  -h, --help        Print this help");
    puts("  -v, --version     Print version information");
    puts("");
    puts("Report bugs to <chbarts@gmail.com>.");
}

int main(int argc, char *argv[])
{
    aa *tree;
    char *pnam;
    FILE *fin;
    int i, u = 0, r = 0, lind, c;
    extern int optind;
    struct option longopts[] = {
        {"uniqify", 0, 0, 0},
        {"reverse", 0, 0, 0},
        {"help", 0, 0, 0},
        {"version", 0, 0, 0},
        {0, 0, 0, 0}
    };

    if (argc == 1) {
        if ((tree = aa_new(comparator)) == NULL) {
            fprintf(stderr, "aa_new() error\n");
            exit(EXIT_FAILURE);
        }

        dofile("stdin", stdin, tree);
        aa_traverse(tree, printall, TRAV_IN);
        cleanup(tree);
        return 0;
    }

    pnam = argv[0];

    while ((c = getopt_long(argc, argv, "urhv", longopts, &lind)) != -1) {
        switch (c) {
        case 0:
            switch (lind) {
            case 0:
                u = 1;
                break;
            case 1:
                r = 1;
                break;
            case 2:
                usage(pnam);
                return 0;
                break;
            case 3:
                version();
                return 0;
                break;
            default:
                usage(pnam);
                exit(EXIT_FAILURE);
                break;
            }

            break;
        case 'u':
            u = 1;
            break;
        case 'r':
            r = 1;
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

    if (r == 1) {
        if ((tree = aa_new(rcomparator)) == NULL) {
            fprintf(stderr, "aa_new error\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if ((tree = aa_new(comparator)) == NULL) {
            fprintf(stderr, "aa_new error\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        dofile("stdin", stdin, tree);
    } else {
        for (i = optind; i < argc; i++) {
            if ((fin = fopen(argv[i], "rb")) == NULL) {
                handle_ferr(argv[i], "sort");
                continue;
            }

            dofile(argv[i], fin, tree);

            if (fclose(fin) == EOF) {
                handle_ferr(argv[i], "sort");
            }
        }
    }

    if (u == 1) {
        aa_traverse(tree, print, TRAV_IN);
    } else {
        aa_traverse(tree, printall, TRAV_IN);
    }

    cleanup(tree);
    return 0;
}
