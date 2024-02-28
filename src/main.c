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
        init_maps();
        add_word(L"library");
        add_word(L"libras");
        add_word(L"lobster");
        add_word(L"lobster");
        add_word(L"ジalsoa");
        print_results(search(L"y", SEARCH_INSIDE));
        free_maps();
        return 0;
}
