#include <stdio.h>
#include "avl.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

int _compare(void *compare_arg, void *a, void *b);
int _free(void *key);
int _printer(char *buff, void *key);

int main(int argc, char **argv)
{
    int i, max_nodes;
    avl_tree *tree;
    avl_node *node;

    max_nodes = 25;

    if (argc == 2) {
        max_nodes = atoi(argv[1]);
        if (max_nodes == 0)
            max_nodes = 10;
    }

    printf("avl test... max_nodes = %d...\n", max_nodes);

    tree = avl_tree_new(_compare, NULL);

    printf("Filling tree...\n");
    for (i = 0; i < max_nodes; i++) {
        avl_insert(tree, (void *)rand());
    }
    
    printf("Traversing tree...\n");
    node = avl_get_first(tree);
    while (node) {
        i = (int)node->key;

        printf("...%5d\n", i);

        node = avl_get_next(node);
    }

    printf("Trying to go backwards...\n");
    node = tree->root->right;
    while (node) {
        i = (int)node->key;
        printf("...%5d\n", i);
        node = avl_get_prev(node);
    }

    printf("Printing tree...\n");
    avl_print_tree(tree, _printer);

    avl_tree_free(tree, _free);
    
    return 0;
}

int _compare(void *compare_arg, void *a, void *b)
{
    int i, j;

    i = (int)a;
    j = (int)b;

    if (i > j)
        return 1;
    if (j > i)
        return -1;
    return 0;
}

int _free(void *key) 
{
    return 1;
}

int _printer(char *buff, void *key)
{
    return snprintf(buff, 25, "%d", (int)key);
}












