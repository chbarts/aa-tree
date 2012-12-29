#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

static int duplicate(void *orig, void *new)
{
    int n;
    ll *node = (ll *) orig, *node2 = (ll *) new;
    char *ln;

    n = get_tag(node);
    set_tag(node, n++);
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
        if ((lln = new_node(0, ln, NULL)) == NULL) {
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

int main(int argc, char *argv[])
{
    aa *tree;
    FILE *fin;
    int i, u = 0;

    if ((tree = aa_new(comparator)) == NULL) {
        fprintf(stderr, "aa_new error\n");
        exit(EXIT_FAILURE);
    }

    if ((argc > 1) && !strcmp(argv[1], "-u")) {
        u++;
        argc--;
        argv++;
    }

    if (argc == 1) {
        dofile("stdin", stdin, tree);
    } else {
        for (i = 1; i < argc; i++) {
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
