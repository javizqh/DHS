#include "dhs.h"
#include "fuzzy_search.h"
#include "standard_c_search.h"
#include <time.h>

#define SECTONANO   1000000000
#define MILISTONANO 1000000

void print_results(struct _search_resp *search_results)
{
        struct _search_resp_node *node;
        int i = 0;

        if (search_results == NULL)
                return;

        printf("Results\n");
        for (node = search_results->head; node; node = node->next) {
                printf("[%d] ", i++);
                for (int j = 0; j < node->data->len; j++) {
                        if (j == node->match_pos[0])
                                printf("\033[01;31m");
                        else if (j == node->match_pos[1] + 1)
                                printf("\033[00m");

                        printf("%lc", node->data->str[j]);
                }
                printf("\033[00m\n");
        }
        printf("\n");
        free_search_results(search_results);
}

// Returns the number of nanoseconds passed
long get_nanoseconds(struct timespec start, struct timespec end)
{
        return (end.tv_sec - start.tv_sec) * SECTONANO + (end.tv_nsec - start.tv_nsec);
}

int main()
{
        struct timespec begin, end;
        struct _search_resp *search_results;

        // ジ
        load_from_file("test/Words/all/all100.txt");

        clock_gettime(CLOCK_MONOTONIC, &begin);
        search_results = search(L"low", SEARCH_INSIDE);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("Latency: %ld ns\n", get_nanoseconds(begin, end));
        print_results(search_results);

        clock_gettime(CLOCK_MONOTONIC, &begin);
        search_results = search(L"a", SEARCH_INSIDE);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("Latency: %ld ns\n", get_nanoseconds(begin, end));
        print_results(search_results);
        free_maps();

        load_from_file_fz("test/Words/all/all100.txt");

        clock_gettime(CLOCK_MONOTONIC, &begin);
        search_results = search_fz(L"low", SEARCH_INSIDE);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("Latency: %ld ns\n", get_nanoseconds(begin, end));
        print_results(search_results);

        clock_gettime(CLOCK_MONOTONIC, &begin);
        search_results = search_fz(L"a", SEARCH_INSIDE);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("Latency: %ld ns\n", get_nanoseconds(begin, end));
        print_results(search_results);
        free_words_fz();

        load_from_file_stdc("test/Words/all/all100.txt");

        clock_gettime(CLOCK_MONOTONIC, &begin);
        search_results = search_stdc(L"low", SEARCH_INSIDE);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("Latency: %ld ns\n", get_nanoseconds(begin, end));
        print_results(search_results);

        clock_gettime(CLOCK_MONOTONIC, &begin);
        search_results = search_stdc(L"a", SEARCH_INSIDE);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("Latency: %ld ns\n", get_nanoseconds(begin, end));
        print_results(search_results);
        free_words_stdc();
        return 0;
}
