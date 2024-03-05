#include <stdlib.h>
#include "words.h"

void free_search_results(struct _search_resp *search_results)
{
        struct _search_resp_node *node;
        struct _search_resp_node *free_node;

        for (node = search_results->head; node;) {
                free_node = node;
                node = node->next;
                free(free_node);
        }

        free(search_results);
        return;
}
