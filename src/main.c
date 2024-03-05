#include "test.h"
#include <time.h>

// void print_results(struct _search_resp *search_results)
// {
//         struct _search_resp_node *node;
//         int i = 0;

//         if (search_results == NULL)
//                 return;

//         printf("Results\n");
//         for (node = search_results->head; node; node = node->next) {
//                 printf("[%d] ", i++);
//                 for (int j = 0; j < node->data->len; j++) {
//                         if (j == node->match_pos[0])
//                                 printf("\033[01;31m");
//                         else if (j == node->match_pos[1] + 1)
//                                 printf("\033[00m");

//                         printf("%lc", node->data->str[j]);
//                 }
//                 printf("\033[00m\n");
//         }
//         printf("\n");
//         free_search_results(search_results);
// }

int main()
{
        int data_sizes[4] = { 10, 100, 1000, 10000 };

        for (size_t i = 0; i < 4; i++) {
                test(DDHS, data_sizes[i]);
                test(FUZZY, data_sizes[i]);
                test(STDC, data_sizes[i]);
        }

        return 0;
}
