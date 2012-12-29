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

static int min(int a, int b)
{
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

static struct node *decrease_level(struct node *t)
{
    int should_be;

    should_be = min(t->left->level, t->right->level) + 1;

    if (should_be < t->level) {
        t->level = should_be;
        if (should_be < t->right->level) {
            t->right->level = should_be;
        }
    }

    return t;
}

static void *predecessor(struct node *t)
{
    t = t->left;

    while (t->right != NULL) {
        t = t->right;
    }

    return t->data;
}

static void *successor(struct node *t)
{
    t = t->right;

    while (t->left != NULL) {
        t = t->left;
    }

    return t->data;
}

static struct node *delete(struct node *t, void *data, ftype dup,
                           ftype comp, void (*del) (void *))
{
    void *l;

    if (t == NULL) {
        return NULL;
    } else if (comp(data, t->data) == -1) {
        t->left = delete(t->left, data, dup, comp, del);
    } else if (comp(data, t->data) == 1) {
        t->right = delete(t->right, data, dup, comp, del);
    } else if (dup != NULL) {
        if (dup(t->data, data) == 0) {
            return t;
        } else {
            longjmp(env, 4);
        }

    } else {
        if ((t->left == NULL) && (t->right == NULL)) {
            if (del)
                del(t->data);

            free(t);
            return NULL;
        } else if (t->left == NULL) {
            l = successor(t);
            t->right = delete(t->right, l, dup, comp, del);
            if (del)
                del(t->data);

            t->data = l;
        } else {
            l = predecessor(t);
            t->left = delete(t->left, l, dup, comp, del);
            if (del)
                del(t->data);

            t->data = l;
        }
    }

    t = decrease_level(t);
    t = skew(t);
    t->right = skew(t->right);
    t->right->right = skew(t->right->right);
    t = split(t);
    t->right = split(t->right);
    return t;
}

int aa_delete(aa * tree, void *data, int (*dup) (void *orig, void *data),
              void (*del) (void *))
{
    int rval;

    if (tree == NULL)
        return 1;

    if ((rval = setjmp(env)) == 0) {
        tree->root = delete(tree->root, data, dup, tree->comp, del);
    } else {
        return rval;
    }

    return 0;
}
