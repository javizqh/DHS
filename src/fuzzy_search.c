#include "fuzzy_search.h"

struct _word_map {
        int len;
        struct _word_data *head;
        struct _word_data *tail;
};

int are_maps_init = 0;
struct _word_map word_map;

void init_word_map()
{
        word_map.len = 0;
        word_map.head = NULL;
        word_map.tail = NULL;
}

void free_words_fz()
{
        struct _word_data *node;
        struct _word_data *free_node;

        for (node = word_map.head; node;) {
                free_node = node;
                node = node->next;
                free(free_node->str);
                free(free_node);
        }

        word_map.len = 0;
        word_map.head = NULL;
        word_map.tail = NULL;
}

struct _word_data *new_word_data(const wchar_t *str)
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

int add_word_data(struct _word_data *node)
{
        if (word_map.head == NULL) {
                word_map.head = node;
                word_map.tail = node;
        } else {
                word_map.tail->next = node;
                word_map.tail = node;
        }

        return word_map.len++;
}


int add_word(const wchar_t *str)
{
        struct _word_data *word_data;
        struct _search_resp *is_inside;

        word_data = new_word_data(str);

        is_inside = search(str, SEARCH_EXACT);
        if (is_inside != NULL) {
                free_search_results(is_inside);
                free(word_data->str);
                free(word_data);
                return 1;
        }

        add_word_data(word_data);
        return 0;
};


int load_from_file_fz(const char *filename)
{
        FILE *fp;
        size_t len = 0;
        char *buff = NULL;
        ssize_t readed = 0;

        fp = fopen(filename, "r");
        if (fp == NULL)
                err(EXIT_FAILURE, "open failed");

        if (!are_maps_init) {
                init_word_map();
        }

        while ((readed = getline(&buff, &len, fp)) != -1) {
                if (readed == 0)
                        continue;
                wchar_t ws[readed];

                buff[readed - 1] = '\0';
                mbstowcs(ws, buff, readed);
                add_word(ws);
        }

        free(buff);
        fclose(fp);
        return 0;
}