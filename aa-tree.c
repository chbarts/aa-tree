#include <stdlib.h>
#include <setjmp.h>
#include "aa-tree.h"

typedef int (*ftype) (void *, void *);

static jmp_buf env;

struct node {
    int level;
    void *data;
    struct node *left;
    struct node *right;
};

struct aa {
    struct node *root;
    struct node *cursor;
    ftype comp;
};

aa *aa_new(ftype comp)
{
    aa *tree;

    if (comp == NULL) {
        return NULL;
    }

    if ((tree = malloc(sizeof(aa))) == NULL) {
        return NULL;
    }

    tree->comp = comp;
    tree->root = tree->cursor = NULL;

    return tree;
}

static struct node *skew(struct node *t)
{
    struct node *l;

    if (t == NULL)
        return NULL;

    if (t->left == NULL)
        return t;

    if (t->left->level == t->level) {
        l = t->left;
        t->left = l->right;
        l->right = t;
        return l;
    } else {
        return t;
    }
}

static struct node *split(struct node *t)
{
    struct node *r;

    if (t == NULL)
        return NULL;

    if ((t->right == NULL) || (t->right->right == NULL))
        return t;

    if (t->level == t->right->right->level) {
        r = t->right;
        t->right = r->left;
        r->left = t;
        r->level++;
        return r;
    } else {
        return t;
    }
}

static struct node *insert(struct node *t, void *data, ftype dup,
                           ftype comp)
{
    if (t == NULL) {
        if ((t = malloc(sizeof(struct node))) == NULL) {
            longjmp(env, 2);
        }

        t->data = data;
        t->level = 1;
        t->left = NULL;
        t->right = NULL;
        return t;
    } else if (comp(data, t->data) == -1) {
        t->left = insert(t->left, data, dup, comp);
    } else if (comp(data, t->data) == 1) {
        t->right = insert(t->right, data, dup, comp);
    } else if (dup == NULL) {
        longjmp(env, 3);
    } else {
        if (dup(t->data, data) == 0) {
            return t;
        } else {
            longjmp(env, 4);
        }
    }

    t = skew(t);
    t = split(t);
    return t;
}

int aa_add(aa * tree, void *data, int (*dup) (void *orig, void *new))
{
    int rval;

    if (tree == NULL)
        return 1;

    if ((rval = setjmp(env)) == 0) {
        tree->root = insert(tree->root, data, dup, tree->comp);
    } else {
        return rval;
    }

    return 0;
}
