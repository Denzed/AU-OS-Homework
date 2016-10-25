#define define_list_insert_head(t)     \
void insert_head(t *node, t **list) {  \
    if (list) {                        \
        if (*list) {                   \
            (*list)->prev = node;      \
        }                              \
        node->next = (*list);          \
        (*list) = node;                \
    }                                  \
}

#define define_list_erase_head(t)   \
void erase_head(t **list) {         \
    if (list && *list) {            \
        *list = (*list)->next;      \
        if (*list) {                \
            (*list)->prev = NULL;   \
        }                           \
    }                               \
}                                   

#define define_list_erase_node(t)       \
void erase_node(t *node) {              \
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
