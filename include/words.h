
#ifndef WORDS
#define WORDS

enum search_flags {
        SEARCH_EXACT = 0,
        SEARCH_INSIDE,
        SEARCH_BEGGINING
};

struct _word_data {
        int type;
        int len;
        wchar_t *str;
        struct _word_data *next;
};

struct _search_resp_node {
        int match_pos[2];
        struct _word_data *data;
        struct _search_resp_node *next;
};

struct _search_resp {
        int len;
        struct _search_resp_node *head;
        struct _search_resp_node *tail;
};

void free_search_results(struct _search_resp *search_results);

#endif                          // WORDS
