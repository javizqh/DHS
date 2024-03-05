#include <stdio.h>
#include "standard_c_search.h"

struct _word_map_stdc {
        int len;
        struct _word_data *head;
        struct _word_data *tail;
};

int are_maps_init_stdc = 0;
int max_len_stdc = 0;
struct _word_map_stdc word_map_stdc;

struct _search_resp_node *new_search_resp_node_stdc(struct _word_data *data, int start, int end)
{
        struct _search_resp_node *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(node, 0, sizeof(*node));

        node->match_pos[0] = start;
        node->match_pos[1] = end;
        node->next = NULL;
        node->data = data;

        return node;
}

int add_search_resp_node_stdc(struct _search_resp *list, struct _word_data *data, int start,
                              int end)
{
        struct _search_resp_node *node = new_search_resp_node_stdc(data, start, end);

        if (list->head == NULL) {
                list->head = node;
                list->tail = node;
        } else {
                list->tail->next = node;
                list->tail = node;
        }

        return list->len++;
}

struct _search_resp *search_len_hash_stdc(const wchar_t *str, int len, int exact_search, int begin)
{
        struct _search_resp *result;
        wchar_t *find;
        int index = 0;

        result = malloc(sizeof(*result));
        if (result == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(result, 0, sizeof(*result));

        result->head = NULL;
        result->tail = NULL;
        result->len = 0;

        for (struct _word_data * node = word_map_stdc.head; node; node = node->next) {
                find = wcsstr(node->str, str);
                if (find) {
                        index = find - node->str;
                        if ((exact_search && node->len != len) || (begin && index > 0))
                                continue;
                        add_search_resp_node_stdc(result, node, index, index + len - 1);
                }
        }

        if (result->len > 0)
                return result;

        free(result);
        return NULL;
}

struct _search_resp *search_stdc(const wchar_t *str, int mode)
{
        int len = wcslen(str);

        if (max_len_stdc < len - 1)
                return NULL;

        if (len == 1)
                mode = SEARCH_BEGGINING;

        switch (mode) {
        case SEARCH_EXACT:
                return search_len_hash_stdc(str, len, 1, 0);
        case SEARCH_INSIDE:
                return search_len_hash_stdc(str, len, 0, 0);
        case SEARCH_BEGGINING:
                return search_len_hash_stdc(str, len, 0, 1);
        }

        return NULL;
}

void init_word_map_stdc()
{
        word_map_stdc.len = 0;
        word_map_stdc.head = NULL;
        word_map_stdc.tail = NULL;
}

void free_words_stdc()
{
        struct _word_data *node;
        struct _word_data *free_node;

        for (node = word_map_stdc.head; node;) {
                free_node = node;
                node = node->next;
                free(free_node->str);
                free(free_node);
        }

        word_map_stdc.len = 0;
        word_map_stdc.head = NULL;
        word_map_stdc.tail = NULL;
}

struct _word_data *new_word_data_stdc(const wchar_t *str)
{
        struct _word_data *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(node, 0, sizeof(*node));

        node->type = 1;
        node->len = wcslen(str);
        node->str = malloc((node->len + 1) * (sizeof(*node->str)));
        wcscpy(node->str, str);
        node->next = NULL;

        return node;
}

int add_word_data_stdc(struct _word_data *node)
{
        if (word_map_stdc.head == NULL) {
                word_map_stdc.head = node;
                word_map_stdc.tail = node;
        } else {
                word_map_stdc.tail->next = node;
                word_map_stdc.tail = node;
        }

        return word_map_stdc.len++;
}

int add_word_stdc(const wchar_t *str)
{
        struct _word_data *word_data;
        struct _search_resp *is_inside;

        word_data = new_word_data_stdc(str);

        is_inside = search_stdc(str, SEARCH_EXACT);
        if (is_inside != NULL) {
                free_search_results(is_inside);
                free(word_data->str);
                free(word_data);
                return 1;
        }

        add_word_data_stdc(word_data);

        if (word_data->len > max_len_stdc)
                max_len_stdc = word_data->len;
        return 0;
};

int load_from_file_stdc(const char *filename)
{
        FILE *fp;
        size_t len = 0;
        char *buff = NULL;
        ssize_t readed = 0;

        fp = fopen(filename, "r");
        if (fp == NULL)
                err(EXIT_FAILURE, "open failed");

        if (!are_maps_init_stdc) {
                init_word_map_stdc();
        }

        while ((readed = getline(&buff, &len, fp)) != -1) {
                if (readed == 0)
                        continue;
                wchar_t ws[readed];

                buff[readed - 1] = '\0';
                mbstowcs(ws, buff, readed);
                add_word_stdc(ws);
        }

        free(buff);
        fclose(fp);
        return 0;
}
