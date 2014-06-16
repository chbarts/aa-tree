#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define _GNU_SOURCE             /* Enable getopt_long(). */
#include <getopt.h>
#include "ggets.h"
#include "handle_ferr.h"
#include "aa-tree.h"
#include "ll3.h"

typedef struct {
   long long num;
   aa *tree;
} tstruct;

static void *print(void *data)
{
    ll *node = (ll *) data;
    char *ln;

    ln = get_data(node);
    puts(ln);
    return data;
}

static void *printn(void *data)
{
   tstruct *tstr = (tstruct *) data;
   aa_traverse(tstr->tree, print, TRAV_IN);
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

static void *printalln(void *data)
{
   tstruct *tstr = (tstruct *) data;
   aa_traverse(tstr->tree, printall, TRAV_IN);
   return data;
}

static void cleanup(aa * tree, bool numer);

static void *freedatan(void *data)
{
   tstruct *node = (tstruct *) data;
   cleanup(node->tree, false);
   free(node);
   return NULL;
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

static void cleanup(aa * tree, bool numer)
{
   if (numer) {
      aa_traverse(tree, freedatan, TRAV_IN);
   } else {
      aa_traverse(tree, freedata, TRAV_IN);
   }

    aa_free(tree);
}

static void error(aa * tree, bool numer)
{
    fprintf(stderr, "memory error\n");
    cleanup(tree, numer);
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

static int ncomparator(void *left, void *right)
{
   tstruct *lv = (tstruct *) left, *rv = (tstruct *) right;

   if (lv->num < rv->num)
      return -1;
   else if (lv->num > rv->num)
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

static int rncomparator(void *left, void *right)
{
   tstruct *lv = (tstruct *) left, *rv = (tstruct *) right;

   if (lv->num < rv->num)
      return 1;
   else if (lv->num > rv->num)
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

static int nduplicate(void *orig, void *new)
{
   ll *val;
   tstruct *node = (tstruct *) orig, *node2 = (tstruct *) new;
   aa *ttree;

   aa_to_root(node2->tree);
   val = aa_get_here(node2->tree);
   ttree = node->tree;

   switch (aa_add(node->tree, val, duplicate)) {
   case 0:
      break;
   case 2:                /* memory error */
      free(node);
      cleanup(node2->tree, false);
      free(node2);
      error(ttree, false);
      break;
   default:               /* can't happen */
      fprintf(stderr, "can't happen\n");
      free(node);
      cleanup(ttree, false);
      cleanup(node2->tree, false);
      free(node2);
      exit(EXIT_FAILURE);
      break;
   }

   aa_free(node2->tree);
   free(node2);

   return 0;
}

static void dofile(char *fname, FILE * fin, aa * tree, bool numer, bool rev)
{
    char *ln;
    ll *lln;
    tstruct *tstr = NULL;

    while (fggets(&ln, fin) == 0) {
        if ((lln = new_node(1, ln, NULL)) == NULL) {
            free(ln);
            error(tree, numer);
        }

        if (numer) {
           if ((tstr = malloc(sizeof(tstruct))) == NULL) {
              free(ln);
              free(lln);
              error(tree, numer);
           }

           tstr->num = atoll(ln);

           if ((tstr->tree = aa_new(rev ? rcomparator : comparator)) == NULL) {
              free(ln);
              free(lln);
              free(tstr);
              error(tree, numer);
           }

           switch (aa_add(tstr->tree, lln, duplicate)) {
           case 0:
              break;
           case 2:                /* memory error */
              free(ln);
              free(lln);
              free(tstr);
              error(tree, numer);
              break;
           default:               /* can't happen */
              fprintf(stderr, "can't happen\n");
              free(ln);
              free(lln);
              free(tstr);
              cleanup(tree, numer);
              exit(EXIT_FAILURE);
              break;
           }
        }

        switch (aa_add(tree, numer ? (void*) tstr : (void*) lln, numer ? nduplicate : duplicate)) {
        case 0:                /* no error */
            break;
        case 2:                /* memory error */
            free(ln);
            free(lln);
            free(tstr);
            error(tree, numer);
            break;
        default:               /* can't happen */
            fprintf(stderr, "can't happen\n");
            free(ln);
            free(lln);
            free(tstr);
            cleanup(tree, numer);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (ferror(fin) != 0)
        handle_ferr(fname, "sort");
}

static void version(void)
{
    puts("sort version 0.5.0");
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
    puts("All files sorted together into one output.");
    puts("");
    puts("  -u, --uniqify     Only print identical lines once");
    puts("  -r, --reverse     Reverse the sense of the comparison");
    puts("  -n, --numer       Sort numerically, then non-numerically");
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
    int i, lind, c;
    bool uniq = false, rev = false, numer = false;
    extern int optind;
    struct option longopts[] = {
        {"uniqify", 0, 0, 0},
        {"reverse", 0, 0, 0},
        {"numer", 0, 0, 0},
        {"help", 0, 0, 0},
        {"version", 0, 0, 0},
        {0, 0, 0, 0}
    };

    if (argc == 1) {
        if ((tree = aa_new(comparator)) == NULL) {
            fprintf(stderr, "aa_new() error\n");
            exit(EXIT_FAILURE);
        }

        dofile("stdin", stdin, tree, false, false);
        aa_traverse(tree, printall, TRAV_IN);
        cleanup(tree, numer);
        return 0;
    }

    pnam = argv[0];

    while ((c = getopt_long(argc, argv, "urnhv", longopts, &lind)) != -1) {
        switch (c) {
        case 0:
            switch (lind) {
            case 0:
                uniq = true;
                break;
            case 1:
                rev = true;
                break;
            case 2:
               numer = true;
               break;
            case 3:
                usage(pnam);
                return 0;
                break;
            case 4:
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
            rev = true;
            break;
        case 'n':
           numer = true;
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

    if (rev && numer) {
        if ((tree = aa_new(rncomparator)) == NULL) {
            fprintf(stderr, "aa_new error\n");
            exit(EXIT_FAILURE);
        }
    } else if (numer) {
        if ((tree = aa_new(ncomparator)) == NULL) {
            fprintf(stderr, "aa_new error\n");
            exit(EXIT_FAILURE);
        }
    } else if (rev) {
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
       dofile("stdin", stdin, tree, numer, rev);
    } else {
        for (i = optind; i < argc; i++) {
            if ((fin = fopen(argv[i], "rb")) == NULL) {
                handle_ferr(argv[i], "sort");
                continue;
            }

            dofile(argv[i], fin, tree, numer, rev);

            if (fclose(fin) == EOF) {
                handle_ferr(argv[i], "sort");
            }
        }
    }

    if (uniq && numer) {
        aa_traverse(tree, printn, TRAV_IN);
    } else if (uniq) {
       aa_traverse(tree, print, TRAV_IN);
    } else if (numer) {
       aa_traverse(tree, printalln, TRAV_IN);
    } else {
        aa_traverse(tree, printall, TRAV_IN);
    }

    cleanup(tree, numer);
    return 0;
}
