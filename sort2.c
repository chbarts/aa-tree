#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define _GNU_SOURCE             /* Enable getopt_long(). */
#include <getopt.h>
#include <stdbool.h>
#include "ll3.h"
#include "ggets.h"
#include "aa-tree.h"
/* #include "asprintf.h" */
#include "handle_ferr.h"

static void *print(void *data)
{
    ll *node = (ll *) data;
    char *ln;

    ln = get_data(node);
    puts(ln);
    return data;
}

static void *lprint(void *data)
{
    ll *node = (ll *) data;
    char *ln;

    ln = get_data(node);
    puts(strchr(ln, ':') + 1);
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

static void *lprintall(void *data)
{
    ll *node = (ll *) data;
    char *ln, *lno;
    int n;

    ln = get_data(node);
    lno = strchr(ln, ':') + 1;
    n = get_tag(data);
    while (n-- > 0)
        puts(lno);

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

static void version(void)
{
    puts("sort2 version 0.1.0");
    puts("Copyright 2014 Chris Barts.");
    puts("sort comes with NO WARRANTY to the extent permitted by law.");
    puts("You may redistribute this software under the terms");
    puts("of the GNU General Public License.");
    puts("For more information, see the file named COPYING");
    puts("in this program's distribution file.");
}

static void usage(char *pname)
{
    printf("Usage: %s [OPTION] [FILE...]\n", pname);
    puts("Sort files or, if no files on the command line, sort stdin.");
    puts("Multiple sorting criteria can be combined.");
    puts("All files sorted together into one output.");
    puts("");
    puts("  -u, --uniqify     Only print identical lines once");
    puts("  -r, --reverse     Reverse the sense of the comparison");
    puts("  -n, --numer       Sort numerically, then non-numerically");
    puts("  -b, --bylen       Sort by length first, then by other critera");
    puts("  -h, --help        Print this help");
    puts("  -v, --version     Print version information");
    puts("");
    puts("Report bugs to <chbarts@gmail.com>.");
}

static int rev;

static int sncomp(char *left, char *right)
{
    long int nl, nr;
    char *clo, *cro;
    int r = 0;

    nl = strtoll(left, &clo, 10);
    nr = strtoll(right, &cro, 10);

    if (nl < nr) {
        r = -1;
    } else if (nr > nl) {
        r = 1;
    } else {
        r = strcmp(clo, cro);

        if (r < 0) {
            r = -1;
        } else if (r > 0) {
            r = 1;
        }
    }

    return (rev * r);
}

static int ncomparator(void *left, void *right)
{
    ll *l = (ll *) left, *r = (ll *) right;

    return sncomp((char *) get_data(l), (char *) get_data(r));
}

static int lncomparator(void *left, void *right)
{
    ll *l = (ll *) left, *r = (ll *) right;
    unsigned long long int ln, rn;
    char *ls, *rs, *cro, *clo;
    int res = 0;

    ls = (char *) get_data(l);
    rs = (char *) get_data(r);

    ln = strtoll(ls, &clo, 10);
    rn = strtoll(rs, &cro, 10);

    if (ln < rn) {
        res = -1;
    } else if (ln > rn) {
        res = 1;
    } else {
        return sncomp(clo + 1, cro + 1);
    }

    return (rev * res);
}

static int comparator(void *left, void *right)
{
    char *cl, *cr;
    int r;

    cl = (char *) get_data((ll *) left);
    cr = (char *) get_data((ll *) right);

    r = strcmp(cl, cr);

    if (r > 0) {
       r = 1;
    } else if (r < 0) {
       r = -1;
    }

    return (rev * r);
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

static void dofile(char *fname, FILE * fin, aa * tree, bool bylen)
{
    char *ln, *lnl;
    ll *lln;

    while (fggets(&ln, fin) == 0) {
#ifdef DEBUG
       printf("read: %s\n", ln);
#endif

        if (bylen) {
            if (asprintf
                (&lnl, "%llu:%s", (unsigned long long int) strlen(ln),
                 ln) == -1) {
                free(ln);
                error(tree);
            }

            free(ln);
            ln = lnl;
#ifdef DEBUG
            printf("%s\n", ln);
#endif
        }

        if ((lln = new_node(1, ln, NULL)) == NULL) {
            free(ln);
            error(tree);
        }

        switch (aa_add(tree, (void *) lln, duplicate)) {
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
        handle_ferr(fname, "sort2");
}

int main(int argc, char *argv[])
{
    aa *tree;
    char *pnam;
    FILE *fin;
    int i, lind, c;
    bool uniq = false, numer = false, bylen = false;
    extern int optind;
    struct option longopts[] = {
        {"uniqify", 0, 0, 0},
        {"reverse", 0, 0, 0},
        {"numer", 0, 0, 0},
        {"bylen", 0, 0, 0},
        {"help", 0, 0, 0},
        {"version", 0, 0, 0},
        {0, 0, 0, 0}
    };

    rev = 1;

#ifdef DEBUG
    printf("rev = %d\n", rev);
#endif

    if (argc == 1) {
        if ((tree = aa_new(comparator)) == NULL) {
            fprintf(stderr, "aa_new() error\n");
            exit(EXIT_FAILURE);
        }

        dofile("stdin", stdin, tree, false);
        aa_traverse(tree, printall, TRAV_IN);
        cleanup(tree);
        return 0;
    }

    pnam = argv[0];

    while ((c = getopt_long(argc, argv, "urnbhv", longopts, &lind)) != -1) {
        switch (c) {
        case 0:
            switch (lind) {
            case 0:
                uniq = true;
                break;
            case 1:
                rev = -1;
                break;
            case 2:
                numer = true;
                break;
            case 3:
                bylen = true;
                break;
            case 4:
                usage(pnam);
                return 0;
                break;
            case 5:
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
            uniq = true;
            break;
        case 'r':
            rev = -1;
            break;
        case 'n':
            numer = true;
            break;
        case 'b':
            bylen = true;
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

    if (numer && bylen) {
        if ((tree = aa_new(lncomparator)) == NULL) {
            fprintf(stderr, "aa_new error\n");
            exit(EXIT_FAILURE);
        }

    } else if (numer || bylen) {
        if ((tree = aa_new(ncomparator)) == NULL) {
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
        dofile("stdin", stdin, tree, bylen);
    } else {
        for (i = optind; i < argc; i++) {
            if ((fin = fopen(argv[i], "rb")) == NULL) {
                handle_ferr(argv[i], "sort");
                continue;
            }

            dofile(argv[i], fin, tree, bylen);

            if (fclose(fin) == EOF) {
                handle_ferr(argv[i], "sort");
            }
        }
    }

    if (bylen && uniq) {
        aa_traverse(tree, lprint, TRAV_IN);
    } else if (bylen) {
        aa_traverse(tree, lprintall, TRAV_IN);
    } else if (uniq) {
        aa_traverse(tree, print, TRAV_IN);
    } else {
        aa_traverse(tree, printall, TRAV_IN);
    }

    cleanup(tree);
    return 0;
}
