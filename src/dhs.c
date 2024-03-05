#include "dhs.h"

struct _word_map {
        int len;
        struct _word_data *head;
        struct _word_data *tail;
};

struct _key_node {
        int value;
        wchar_t chr;
        struct _key_node *next;
};

struct _key_map {
        int len;
        struct _key_node *head;
        struct _key_node *tail;
};

struct _inside_hash {
        int depth;
        int max_inside_hash;
        int pos_map[2];
        struct _inside_hash **inside_hash;
};

struct _char_hash {
        int n_values;
        int max_inside_hash;
        struct _word_data *single_char; // For example if 'k' hash has the word "k"
        struct _inside_hash **inside_hash;
        struct _word_data **data;
};

struct _len_hash {
        int index;
        int len;
        struct _len_hash *next;
        struct _char_hash **char_hash;
};

struct _hashmap {
        int len;
        struct _len_hash *head;
        struct _len_hash *tail;
};

int are_maps_init = 0;
struct _key_map key_map;
struct _word_map word_map;
struct _hashmap hashmap;

void init_key_map()
{
        setlocale(LC_ALL, "");
        key_map.len = 0;
        key_map.head = NULL;
        key_map.tail = NULL;
}

void free_key_map()
{
        struct _key_node *node;
        struct _key_node *free_node;

        for (node = key_map.head; node;) {
                free_node = node;
                node = node->next;
                free(free_node);
        }

        key_map.len = 0;
        key_map.head = NULL;
        key_map.tail = NULL;
}

struct _key_node *new_key_node(int value, wchar_t chr)
{
        struct _key_node *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(node, 0, sizeof(*node));

        node->value = value;
        node->chr = chr;
        node->next = NULL;

        return node;
}

int add_key_node(wchar_t chr)
{
        struct _key_node *node = new_key_node(key_map.len++, chr);

        if (key_map.head == NULL) {
                key_map.head = node;
                key_map.tail = node;
        } else {
                key_map.tail->next = node;
                key_map.tail = node;
        }

        return key_map.len - 1;
}

int get_key_value(wchar_t chr)
{
        struct _key_node *node;

        for (node = key_map.head; node; node = node->next) {
                if (chr == node->chr)
                        return node->value;
        }

        return add_key_node(chr);
}

struct _search_resp *sum_search_resp(struct _search_resp *x, struct _search_resp *y)
{
        if (y == NULL)
                return x;
        if (y->len == 0) {
                free(y);
                return x;
        }

        if (x == NULL) {
                x = malloc(sizeof(*x));
                if (x == NULL)
                        err(EXIT_FAILURE, "malloc failed");
                memset(x, 0, sizeof(*x));
        }

        if (x->len == 0) {
                x->head = y->head;
                x->tail = y->tail;
        } else {
                x->tail->next = y->head;
                x->tail = y->tail;
        }

        x->len += y->len;
        free(y);
        return x;
}

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

struct _search_resp_node *new_search_resp_node(struct _word_data *data, int start, int end)
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

int add_search_resp_node(struct _search_resp *list, struct _word_data *data, int start, int end)
{
        struct _search_resp_node *node = new_search_resp_node(data, start, end);

        if (list->head == NULL) {
                list->head = node;
                list->tail = node;
        } else {
                list->tail->next = node;
                list->tail = node;
        }

        return list->len++;
}

struct _search_resp *search_len_hash(int *keys, int len, struct _len_hash *len_hash,
                                     int exact_search)
{
        struct _char_hash *curr_char_hash = NULL;
        struct _inside_hash *inside_hash = NULL;
        struct _search_resp *result;

        result = malloc(sizeof(*result));
        if (result == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(result, 0, sizeof(*result));

        result->head = NULL;
        result->tail = NULL;
        result->len = 0;

        if (len_hash->len > keys[0])
                curr_char_hash = len_hash->char_hash[keys[0]];

        if (curr_char_hash == NULL)
                goto out_free_result;

        if (len_hash->index == 0 && len == 1) {
                if (curr_char_hash->single_char != NULL) {
                        add_search_resp_node(result, curr_char_hash->single_char, 0, 0);
                }
                for (int i = 0; i < curr_char_hash->n_values; i++) {
                        add_search_resp_node(result, curr_char_hash->data[i], 0, 0);
                }
                return result;
        }

        if (keys[1] > curr_char_hash->max_inside_hash)
                goto out_free_result;
        else
                inside_hash = curr_char_hash->inside_hash[keys[1]];

        if (inside_hash == NULL)
                goto out_free_result;

        if (inside_hash->depth == len - 1) {
                for (int k = inside_hash->pos_map[0]; k <= inside_hash->pos_map[1]; k++) {
                        if (exact_search) {
                                if (curr_char_hash->data[k]->len == len) {
                                        add_search_resp_node(result, curr_char_hash->data[k], 0,
                                                             len - 1);
                                        return result;
                                }
                        } else {
                                add_search_resp_node(result, curr_char_hash->data[k],
                                                     len_hash->index,
                                                     len_hash->index + inside_hash->depth);
                        }
                }
        }

        for (int j = 2; j < len; j++) {
                if (keys[j] > inside_hash->max_inside_hash)
                        break;
                else
                        inside_hash = inside_hash->inside_hash[keys[j]];

                if (inside_hash == NULL)
                        break;

                if (inside_hash->depth == len - 1) {
                        for (int k = inside_hash->pos_map[0]; k <= inside_hash->pos_map[1]; k++) {
                                if (exact_search) {
                                        if (curr_char_hash->data[k]->len == len) {
                                                add_search_resp_node(result,
                                                                     curr_char_hash->data[k], 0,
                                                                     len - 1);
                                                return result;
                                        }
                                } else {
                                        add_search_resp_node(result, curr_char_hash->data[k],
                                                             len_hash->index,
                                                             len_hash->index + inside_hash->depth);
                                }
                        }
                }
        }

        if (result->len > 0)
                return result;

 out_free_result:
        free(result);
        return NULL;
}

struct _search_resp *search(const wchar_t *str, int mode)
{
        int keys[wcslen(str)];
        int len = 0;
        struct _len_hash *curr_len_hash = NULL;
        struct _search_resp *result;

        for (wchar_t *ptr = str; *ptr != '\0'; ptr++) {
                keys[len++] = get_key_value(*ptr);
        }

        if (hashmap.len < len - 1)
                return NULL;

        if (len == 1)
                mode = SEARCH_BEGGINING;

        switch (mode) {
        case SEARCH_EXACT:
                return search_len_hash(keys, len, hashmap.head, 1);
        case SEARCH_INSIDE:
                curr_len_hash = hashmap.head;
                result = search_len_hash(keys, len, curr_len_hash, 0);

                for (int i = 1; i < hashmap.len - len + 2; i++) {
                        curr_len_hash = curr_len_hash->next;
                        result =
                            sum_search_resp(result, search_len_hash(keys, len, curr_len_hash, 0));
                }
                return result;
        case SEARCH_BEGGINING:
                return search_len_hash(keys, len, hashmap.head, 0);
        }

        return NULL;
}

void init_word_map()
{
        word_map.len = 0;
        word_map.head = NULL;
        word_map.tail = NULL;
}

void free_word_map()
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

void init_hashmap()
{
        hashmap.len = 0;
        hashmap.head = NULL;
        hashmap.tail = NULL;
}

void free_inside_hash(struct _inside_hash *to_free)
{
        if (to_free->max_inside_hash > -1) {
                for (int i = 0; i < to_free->max_inside_hash + 1; i++) {
                        if (to_free->inside_hash[i] != NULL)
                                free_inside_hash(to_free->inside_hash[i]);
                }
        }
        free(to_free->inside_hash);
        free(to_free);
        return;
}

void free_char_hash(struct _char_hash *to_free)
{
        for (int i = 0; i < to_free->max_inside_hash + 1; i++) {
                if (to_free->inside_hash[i] != NULL)
                        free_inside_hash(to_free->inside_hash[i]);
        }
        free(to_free->inside_hash);
        free(to_free->data);
        free(to_free);
        return;
}

void free_len_hash(struct _len_hash *to_free)
{
        for (int i = 0; i < to_free->len; i++) {
                if (to_free->char_hash[i] != NULL)
                        free_char_hash(to_free->char_hash[i]);
        }
        free(to_free->char_hash);
        free(to_free);
        return;
}

void free_hashmap()
{
        struct _len_hash *node;
        struct _len_hash *free_node;

        for (node = hashmap.head; node;) {
                free_node = node;
                node = node->next;
                free_len_hash(free_node);
        }

        hashmap.len = 0;
        hashmap.head = NULL;
        hashmap.tail = NULL;
}

struct _inside_hash *new_inside_hash(int depth)
{
        struct _inside_hash *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(node, 0, sizeof(*node));

        node->depth = depth + 1;
        node->max_inside_hash = -1;
        node->pos_map[0] = -1;
        node->pos_map[1] = -1;
        node->inside_hash = NULL;

        return node;
}

int add_inside_hash(struct _char_hash *char_hash, int ind)
{
        struct _inside_hash *node = new_inside_hash(0);

        // Copy the old len_hash into the new one with one more space to malloc
        struct _inside_hash **old_ins_hashes = char_hash->inside_hash;

        if (ind > char_hash->max_inside_hash) {
                char_hash->inside_hash = calloc(ind + 1, sizeof(struct _inside_hash *));
                memcpy(char_hash->inside_hash, old_ins_hashes,
                       sizeof(*old_ins_hashes) * (char_hash->max_inside_hash + 1));
                free(old_ins_hashes);
                char_hash->max_inside_hash = ind;
        }

        char_hash->inside_hash[ind] = node;

        return 0;
}

int add_inside_hash_r(struct _inside_hash *inside_hash, int ind)
{
        struct _inside_hash *node = new_inside_hash(inside_hash->depth);

        // Copy the old len_hash into the new one with one more space to malloc
        struct _inside_hash **old_ins_hashes = inside_hash->inside_hash;

        if (ind > inside_hash->max_inside_hash) {
                inside_hash->inside_hash = calloc(ind + 1, sizeof(struct _inside_hash *));
                memcpy(inside_hash->inside_hash, old_ins_hashes,
                       sizeof(*old_ins_hashes) * (inside_hash->max_inside_hash + 1));
                free(old_ins_hashes);
                inside_hash->max_inside_hash = ind;
        }

        inside_hash->inside_hash[ind] = node;

        return 0;
}

int update_inside_pos_map(struct _inside_hash *inside_hash, int pos)
{
        if (inside_hash->pos_map[0] > -1) {
                inside_hash->pos_map[0]++;
                inside_hash->pos_map[1]++;
                for (int i = 0; i < inside_hash->max_inside_hash + 1; i++) {
                        if (inside_hash->inside_hash[i] != NULL)
                                update_inside_pos_map(inside_hash->inside_hash[i], pos);
                }

        }
        return 0;
}

struct _char_hash *new_char_hash()
{
        struct _char_hash *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(node, 0, sizeof(*node));

        node->n_values = 0;
        node->max_inside_hash = -1;
        node->single_char = NULL;
        node->data = NULL;

        return node;
}

int add_char_hash(struct _len_hash *len_hash, int ind)
{
        struct _char_hash *node = new_char_hash();

        // Copy the old len_hash into the new one with one more space to malloc
        struct _char_hash **old_char_hashes = len_hash->char_hash;

        if (ind + 1 > len_hash->len) {
                len_hash->char_hash = calloc(ind + 1, sizeof(struct _char_hash *));
                memcpy(len_hash->char_hash, old_char_hashes,
                       sizeof(*old_char_hashes) * len_hash->len);
                free(old_char_hashes);
                len_hash->len = ind + 1;
        }

        len_hash->char_hash[ind] = node;

        return 0;
}

int insert_word(struct _char_hash *char_hash, struct _word_data *word, int start_index)
{
        int i = 0;
        int len = 0;
        int in_len = 0;
        int min_len = 0;
        int in_keys[word->len - start_index];

        for (wchar_t *ptr = word->str; *ptr != '\0'; ptr++) {
                if (in_len++ >= start_index)
                        in_keys[in_len - start_index - 1] = get_key_value(*ptr);
        }

        // Copy the old len_hash into the new one with one more space to malloc
        struct _word_data **old_data = char_hash->data;

        char_hash->n_values++;

        char_hash->data = calloc(char_hash->n_values, sizeof(struct _word_data *));
        memcpy(char_hash->data, old_data, sizeof(struct _word_data *) * (char_hash->n_values - 1));
        free(old_data);

        if (char_hash->n_values == 1) {
                char_hash->data[0] = word;
                return 0;
        }

        for (i = 0; i < char_hash->n_values - 1; i++) {
                int keys[char_hash->data[i]->len];

                len = 0;
                min_len = 0;

                for (wchar_t *ptr = char_hash->data[i]->str; *ptr != '\0'; ptr++) {
                        if (len++ >= start_index)
                                keys[len - start_index - 1] = get_key_value(*ptr);
                }

                min_len = (in_len > len) ? len : in_len;

                for (int j = 1; j < min_len - start_index; j++) {
                        if (keys[j] > in_keys[j])
                                goto out_for_loop;
                        else if (keys[j] < in_keys[j])
                                goto out_inside_loop;
                }
                if (in_len < len)
                        goto out_for_loop;

 out_inside_loop:
        }

 out_for_loop:

        for (int j = char_hash->n_values - 1; j > i; j--) {
                char_hash->data[j] = char_hash->data[j - 1];
        }

        char_hash->data[i] = word;

        return i;
}

struct _len_hash *new_len_hash(int index)
{
        struct _len_hash *node;

        node = malloc(sizeof(*node));
        if (node == NULL)
                err(EXIT_FAILURE, "malloc failed");
        memset(node, 0, sizeof(*node));

        node->index = index;
        node->len = 0;
        node->next = NULL;
        node->char_hash = NULL;

        return node;
}

int add_len_hash(int index)
{
        struct _len_hash *node = new_len_hash(index);

        if (hashmap.head == NULL) {
                hashmap.head = node;
                hashmap.tail = node;
        } else {
                hashmap.tail->next = node;
                hashmap.tail = node;
        }

        return hashmap.len++;
}

int add_word(const wchar_t *str)
{
        int keys[wcslen(str)];
        int len = 0;
        int inserted_index = 0;
        struct _len_hash *curr_len_hash = NULL;
        struct _char_hash *curr_char_hash = NULL;
        struct _inside_hash *curr_inside_hash = NULL, *prev_inside_hash = NULL;
        struct _word_data *word_data;
        struct _search_resp *is_inside;

        word_data = new_word_data(str);

        for (wchar_t *ptr = str; *ptr != '\0'; ptr++) {
                keys[len++] = get_key_value(*ptr);
        }

        if (hashmap.len == 0)
                add_len_hash(hashmap.len);
        curr_len_hash = hashmap.head;

        is_inside = search(str, SEARCH_EXACT);
        if (is_inside != NULL) {
                free_search_results(is_inside);
                free(word_data->str);
                free(word_data);
                return 1;
        }

        add_word_data(word_data);

        for (int i = 0; i < len; i++) {
                if (len > 1 && len - i == 1)
                        break;
                if (hashmap.len == i) {
                        add_len_hash(hashmap.len);
                        curr_len_hash = curr_len_hash->next;
                } else if (i != 0) {
                        curr_len_hash = curr_len_hash->next;
                }

                curr_char_hash = NULL;

                if (curr_len_hash->len > keys[i]) {
                        curr_char_hash = curr_len_hash->char_hash[keys[i]];
                }

                if (curr_char_hash == NULL) {
                        add_char_hash(curr_len_hash, keys[i]);
                        curr_char_hash = curr_len_hash->char_hash[keys[i]];
                }

                if (i == 0 && len == 1) {
                        curr_char_hash->single_char = word_data;
                        break;
                }

                inserted_index = insert_word(curr_char_hash, word_data, i);

                for (int j = i + 1; j < len; j++) {
                        if (j > i + 1) {
                                prev_inside_hash = curr_inside_hash;

                                if (keys[j] > curr_inside_hash->max_inside_hash
                                    || curr_inside_hash->max_inside_hash < 0) {
                                        add_inside_hash_r(curr_inside_hash, keys[j]);
                                        curr_inside_hash = curr_inside_hash->inside_hash[keys[j]];
                                } else {
                                        if (curr_inside_hash->inside_hash[keys[j]] == NULL)
                                                add_inside_hash_r(curr_inside_hash, keys[j]);

                                        curr_inside_hash = curr_inside_hash->inside_hash[keys[j]];
                                }

                                if (curr_inside_hash->pos_map[0] < 0) {
                                        curr_inside_hash->pos_map[0] = inserted_index;
                                        curr_inside_hash->pos_map[1] = inserted_index;
                                } else {
                                        curr_inside_hash->pos_map[1]++;
                                }

                                for (int k = keys[j] + 1;
                                     k < prev_inside_hash->max_inside_hash + 1; k++) {
                                        if (prev_inside_hash->inside_hash[k] != NULL)
                                                update_inside_pos_map
                                                    (prev_inside_hash->inside_hash[k],
                                                     inserted_index);
                                }
                                if (j + 1 == len) {
                                        for (int k = 0; k < curr_inside_hash->max_inside_hash + 1;
                                             k++) {
                                                if (curr_inside_hash->inside_hash[k] != NULL)
                                                        update_inside_pos_map
                                                            (curr_inside_hash->inside_hash[k],
                                                             inserted_index);
                                        }
                                }
                        } else {
                                if (keys[j] > curr_char_hash->max_inside_hash
                                    || curr_char_hash->max_inside_hash < 0) {
                                        curr_inside_hash = NULL;
                                } else {
                                        curr_inside_hash = curr_char_hash->inside_hash[keys[j]];
                                }
                                if (curr_inside_hash == NULL) {
                                        add_inside_hash(curr_char_hash, keys[j]);
                                        curr_inside_hash = curr_char_hash->inside_hash[keys[j]];
                                }

                                if (curr_inside_hash->pos_map[0] < 0) {
                                        curr_inside_hash->pos_map[0] = inserted_index;
                                        curr_inside_hash->pos_map[1] = inserted_index;
                                } else {
                                        curr_inside_hash->pos_map[1]++;
                                }

                                for (int k = keys[j] + 1; k < curr_char_hash->max_inside_hash + 1;
                                     k++) {
                                        if (curr_char_hash->inside_hash[k] != NULL)
                                                update_inside_pos_map(curr_char_hash->inside_hash
                                                                      [k], inserted_index);
                                }
                                if (j + 1 == len) {
                                        for (int k = 0;
                                             k < curr_inside_hash->max_inside_hash + 1; k++) {
                                                if (curr_inside_hash->inside_hash[k] != NULL)
                                                        update_inside_pos_map
                                                            (curr_inside_hash->inside_hash[k],
                                                             inserted_index);
                                        }
                                }
                        }
                }
        }
        return 0;
};

void init_maps()
{
        init_key_map();
        init_word_map();
        init_hashmap();
}

void free_maps()
{
        free_key_map();
        free_hashmap();
        free_word_map();
}

int load_from_file(const char *filename)
{
        FILE *fp;
        size_t len = 0;
        char *buff = NULL;
        ssize_t readed = 0;

        fp = fopen(filename, "r");
        if (fp == NULL)
                err(EXIT_FAILURE, "open failed");

        if (!are_maps_init) {
                init_maps();
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
