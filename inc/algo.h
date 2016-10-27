#ifndef __ALGO_H__
#define __ALGO_H__

#define define_list_insert_head(t)         \
void t##_insert_head(t *node, t **list) {  \
    if (list) {                            \
        if (*list) {                       \
            (*list)->prev = node;          \
        }                                  \
        node->next = (*list);              \
        (*list) = node;                    \
    }                                      \
}

#define define_list_erase_head(t)   \
void t##_erase_head(t **list) {     \
    if (list && *list) {            \
        *list = (*list)->next;      \
        if (*list) {                \
            (*list)->prev = NULL;   \
        }                           \
    }                               \
}                                   

#define define_list_erase_node(t)       \
void t##_erase_node(t *node) {          \
    if (node->prev) {                   \
        node->prev->next = node->next;  \
    }                                   \
    if (node->next) {                   \
        node->next->prev = node->prev;  \
    }                                   \
}                                       \

#define define_list_operations(t) \
define_list_insert_head(t)        \
define_list_erase_head(t)         \
define_list_erase_node(t)

#define define_splay_tree(t)    \
    \
typedef struct t##_node t##_node;   \
    \
struct t##_node{    \
    t##_node *l, *r, *p;    \
    t v;    \
};  \
    \
block_allocator * t##_alloc = NULL;  \
    \
t##_node *make_##t##_node(t##_node *l, t##_node *r, t##_node *p, t v) { \
    t##_node *res = (t##_node *) allocate_block( t##_alloc); \
    res->l = l; \
    res->r = r; \
    res->p = p; \
    res->v = v; \
    return res; \
}   \
    \
void t##_right_rotate(t##_node *P) {    \
    t##_node *T = P->l; \
    t##_node *B = T->r; \
    t##_node *D = P->p; \
    if (D) {    \
        if(D->r == P) { \
            D->r = T;   \
        } else {    \
            D->l = T;   \
        }   \
    }   \
    if (B) {    \
        B->p=P; \
    }   \
    T->p=D; \
    T->r=P; \
        \
    P->p=T; \
    P->l=B; \
}   \
    \
void t##_left_rotate(t##_node *P) { \
    t##_node *T = P->r; \
    t##_node *B = T->l; \
    t##_node *D = P->p; \
    if (D) {    \
        if (D->r == P) {    \
            D->r = T;   \
        } else {    \
            D->l = T;   \
        }   \
    }   \
    if (B) {    \
        B->p = P;   \
    }   \
    T->p = D;   \
    T->l = P;   \
        \
    P->p = T;   \
    P->r = B;   \
}   \
    \
void t##_splay(t##_node **root, t##_node *T) {   \
    for ( ;; ) {    \
        t##_node *p = T->p; \
        if (!p) {   \
            break;  \
        }   \
        t##_node *pp = p->p;    \
        if (!pp) { /*Zig*/    \
            if (p->l == T) {    \
                t##_right_rotate(p);    \
            } else {    \
                t##_left_rotate(p); \
            }   \
            break;  \
        }   \
        if (pp->l == p) {   \
            if (p->l == T) { /*ZigZig*/   \
                t##_right_rotate(pp);   \
                t##_right_rotate(p);    \
            } else { /*ZigZag*/   \
                t##_left_rotate(p); \
                t##_right_rotate(pp);   \
            }   \
        } else {    \
            if (p->l == T) { /*ZigZag*/   \
                t##_right_rotate(p);    \
                t##_left_rotate(pp);    \
            } else { /*ZigZig*/   \
                t##_left_rotate(pp);    \
                t##_left_rotate(p); \
            }   \
        }   \
    }   \
    (*root) = T;   \
}   \
    \
void t##_insert(t##_node **root, t v) { \
    if (!*root) {   \
        (*root) = make_##t##_node(NULL, NULL, NULL, v); \
        return; \
    }   \
    t##_node *P = (*root);  \
    for ( ;; ) {    \
        if (P->v == v) { /*not multiset*/    \
            break;  \
        }   \
        if (v < (P->v)) {   \
            if(P->l) {  \
                P = P->l;   \
            } else {    \
                P->l = make_##t##_node(NULL, NULL, P, v);   \
                P = P->l;   \
                break;  \
            }   \
        } else {    \
            if (P->r) { \
                P = P->r;   \
            } else {    \
                P->r = make_##t##_node(NULL, NULL, P, v);   \
                P = P->r;   \
                break;  \
            }   \
        }   \
    }   \
    t##_splay(root, P);   \
}   \
    \
t##_node* t##_find(t##_node **root, t v) {  \
    if (!(*root)) { \
        return NULL;    \
    }   \
    t##_node *P = *root;    \
    for (; P && P->v != v; ) {  \
        if (v < P->v) { \
            if (P->l) { \
                P = P->l;   \
            } else {    \
                break;  \
            }   \
        } else {    \
            if (P->r) { \
                P = P->r;   \
            } else {    \
                break;  \
            }   \
        }   \
    }   \
    t##_splay(root, P);   \
    if (P && P->v == v) {   \
        return P;   \
    }   \
    return NULL;    \
}   \
    \
bool t##_erase(t##_node **root, t v) {  \
    t##_node *N = t##_find(root, v);    \
    if (!N) {   \
        return false;   \
    }   \
    t##_splay(root, N); /*check once more*/ \
    t##_node *P = N->l; \
    if (!P) {   \
        (*root) = N->r; \
        (*root)->p = NULL;  \
        free_block( t##_alloc, (ptr) N); \
        return true;    \
    }   \
    for (; P->r; P = P->r); \
    if (N->r) { \
        P->r = N->r;    \
        N->r->p = P;    \
    }   \
    (*root) = N->l; \
    (*root)->p = NULL;  \
    free_block( t##_alloc, (ptr) N); \
    return true;    \
}

#define splay_node(t)           t##_node
#define splay_node_allocator(t) t##_alloc
#define splay_insert(t)         t##_insert
#define splay_erase(t)          t##_erase
#define splay_find(t)           t##_find

#endif /* __ALGO_H__ */