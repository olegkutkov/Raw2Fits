/*
 *
 */

typedef struct node {
    char *object;
    struct node *next;
} list_node_t;

typedef void (*iterate_cb)(char*, void*);

list_node_t *add_object_to_list(list_node_t *list, char *object);
void free_list(list_node_t *list);
void iterate_list_cb(list_node_t *list, iterate_cb cb, void* arg);

