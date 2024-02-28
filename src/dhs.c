#include "dhs.h"

struct _word_data {
        int type;
        int len;
        wchar_t *str;
        struct _word_data *next;
};

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
        int has_single_value;   // For example if 'k' hash has the word "k"
        int max_inside_hash;
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
        if (inside_hash->pos_map[0] > 0) {
                inside_hash->pos_map[0]++;
                inside_hash->pos_map[1]++;
                for (int i = 0; i < inside_hash->max_inside_hash; i++) {
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
        node->has_single_value = 0;
        node->max_inside_hash = -1;
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

int insert_word(struct _char_hash *char_hash, struct _word_data *word)
{
        int i = 0;
        int len = 0;
        int in_keys[word->len];

        for (wchar_t *ptr = word->str; *ptr != '\0'; ptr++) {
                in_keys[len++] = get_key_value(*ptr);
        }

        // Copy the old len_hash into the new one with one more space to malloc
        struct _word_data **old_data = char_hash->data;

        char_hash->n_values++;

        char_hash->data = calloc(char_hash->n_values, sizeof(struct _word_data *));
        memcpy(char_hash->data, old_data, sizeof(*old_data) * (char_hash->n_values - 1));
        free(old_data);

        if (char_hash->n_values == 1) {
                char_hash->data[0] = word;
                return 0;
        }

        for (i = 0; i < char_hash->n_values - 1; i++) {
                int keys[char_hash->data[i]->len];

                len = 0;
                for (wchar_t *ptr = char_hash->data[i]->str; *ptr != '\0'; ptr++) {
                        keys[len++] = get_key_value(*ptr);
                }
                for (int j = 1; i < len; j++) {
                        if (keys[j] > in_keys[j])
                                break;
                }
        }

        for (int j = char_hash->n_values; j > i + 1; j--) {
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
        struct _inside_hash *curr_inside_hash = NULL;
        struct _word_data *word_data;

        word_data = new_word_data(str);

        for (wchar_t *ptr = str; *ptr != '\0'; ptr++) {
                keys[len++] = get_key_value(*ptr);
        }

        // TODO: check if already inside
        // if (inside) {
        //         free(word_data);
        //         return 1;
        // }

        add_word_data(word_data);

        if (hashmap.len == 0)
                add_len_hash(hashmap.len);
        curr_len_hash = hashmap.head;

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

                if (len - i == 1) {
                        curr_char_hash->has_single_value = 1;
                        break;
                }

                inserted_index = insert_word(curr_char_hash, word_data);

                for (int j = i + 1; j < len; j++) {
                        if (j > i + 1) {
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

                                // Iterate for all inside_hashes[keys[j to max_inside_hashes]]
                                for (int k = keys[j] + 1; k < curr_inside_hash->max_inside_hash;
                                     k++) {
                                        if (curr_inside_hash->inside_hash[k] != NULL)
                                                update_inside_pos_map(curr_inside_hash->
                                                                      inside_hash[k],
                                                                      inserted_index);
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

                                // Iterate for all inside_hashes[keys[j to max_inside_hashes]]
                                for (int k = keys[j] + 1; k < curr_char_hash->max_inside_hash; k++) {
                                        if (curr_char_hash->inside_hash[k] != NULL)
                                                update_inside_pos_map(curr_char_hash->
                                                                      inside_hash[k],
                                                                      inserted_index);
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

void pr()
{
        struct _word_data *node;

        for (node = word_map.head; node; node = node->next) {
                printf("%ls\n", node->str);
        }
}
