#include "dhs.h"

void print_results(struct _search_resp *search_results)
{
        struct _search_resp_node *node;
        int i = 0;

        if (search_results == NULL)
                return;

        printf("Results\n");
        for (node = search_results->head; node; node = node->next) {
                printf("[%d] %ls\n", i++, node->data->str);
        }
        printf("\n");
        free_search_results(search_results);
}

int main()
{
        // ジ
        load_from_file("test/Words/es100.txt");
        print_results(search(L"ol", SEARCH_INSIDE));
        free_maps();
        return 0;
}
