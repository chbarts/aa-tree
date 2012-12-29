#ifndef AA_TREE_H
#define AA_TREE_H

typedef enum trav { TRAV_IN, TRAV_POST, TRAV_PRE } trav;
typedef struct aa aa;

/* NULL on error, comp must be a comparator that returns:
 *  -1 when l < r
 *   0 when l == r
 *   1 when l > r */
aa *aa_new(int (*comp) (void *l, void *r));
/* 0 on no error, 1 on no tree, 2 on memory error, 3 on duplicate if dup not set, 4 on dup error
 * dup returns 0 on no error, 1 on error */
int aa_add(aa * tree, void *data, int (*dup) (void *orig, void *new));
/* 0 on no error, 1 on no tree, 2 on dup error
 * dup returns 0 on no error, 1 on error */
int aa_delete(aa * tree, void *data, int (*dup) (void *orig, void *data));
/* NULL on error or not found */
void *aa_find(aa * tree, void *data);
/* 0 on no error, 1 on no tree, 2 on bad trav (not one of TRAV_IN, TRAV_POST, TRAV_PRE) */
int aa_traverse(aa * tree, void *(*func) (void *data), trav t);
/* no-op on error */
void aa_free(aa * tree);

#endif                          /* AA_TREE_H */
