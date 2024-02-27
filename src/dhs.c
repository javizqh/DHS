#include "dhs.h"

#define MAX_CHAR_HASH_SIZE 256
#define MAX_WORDS_IN_CHAR_HASH 1000

struct _word_data {
        int type;
        int len;
        wchar_t str[MAX_CHAR_HASH_SIZE];        // TODO: only temporary
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
        int pos_map[MAX_CHAR_HASH_SIZE];
        struct _inside_hash *inside_hash[MAX_CHAR_HASH_SIZE];
};

struct _char_hash {
        int n_values;
        int has_single_value;   // For example if 'k' hash has the word "k"
        struct _inside_hash *inside_hash[MAX_CHAR_HASH_SIZE];
        struct _word_data *data[MAX_WORDS_IN_CHAR_HASH];        // TODO: temporary it should be dynamic
};

struct _len_hash {
        int index;
        struct _len_hash *next;
        struct _char_hash *char_hash[MAX_CHAR_HASH_SIZE];
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

void free_inside_has(struct _inside_hash *to_free)
{
        for (size_t i = 0; i < MAX_CHAR_HASH_SIZE; i++) {
                if (to_free->inside_hash[i] != NULL)
                        free_inside_has(to_free->inside_hash[i]);
        }
        free(to_free);
        return;
}

void free_char_hash(struct _char_hash *to_free)
{
        for (size_t i = 0; i < MAX_CHAR_HASH_SIZE; i++) {
                if (to_free->inside_hash[i] != NULL)
                        free_inside_has(to_free->inside_hash[i]);
        }
        free(to_free);
        return;
}

void free_len_hash(struct _len_hash *to_free)
{
        for (size_t i = 0; i < MAX_CHAR_HASH_SIZE; i++) {
                if (to_free->char_hash[i] != NULL)
                        free_char_hash(to_free->char_hash[i]);
        }
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

        // TODO: temporary
        for (size_t i = 0; i < MAX_CHAR_HASH_SIZE; i++) {
                node->pos_map[i] = -1;
        }

        return node;
}

int add_inside_hash(struct _char_hash *char_hash, int index)
{
        struct _inside_hash *node = new_inside_hash(0);

        char_hash->inside_hash[index] = node;

        return 0;
}

int add_inside_hash_r(struct _inside_hash *inside_hash, int index)
{
        struct _inside_hash *node = new_inside_hash(inside_hash->depth);

        inside_hash->inside_hash[index] = node;

        return 0;
}

int update_inside_pos_map(struct _inside_hash *inside_hash, int index)
{
        for (int i = index + 1; i < MAX_CHAR_HASH_SIZE; i++) {
                if (inside_hash->pos_map[i] >= 0)
                        inside_hash->pos_map[i]++;
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

        return node;
}

int add_char_hash(struct _len_hash *len_hash, int index)
{
        struct _char_hash *node = new_char_hash();

        len_hash->char_hash[index] = node;

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

        for (i = 0; i < char_hash->n_values - 1; i++) {
                printf("%d\n\n", char_hash->n_values);
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

        for (int j = char_hash->n_values; j > i; j--) {
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
        node->next = NULL;

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
                add_len_hash(keys[0]);
        curr_len_hash = hashmap.head;

        for (int i = 0; i < len; i++) {
                if (len > 1 && len - i == 1)
                        break;
                if (hashmap.len == i) {
                        add_len_hash(keys[0]);
                        curr_len_hash = curr_len_hash->next;
                }

                curr_char_hash = curr_len_hash->char_hash[keys[i]];

                if (curr_char_hash == NULL) {
                        add_char_hash(curr_len_hash, keys[i]);
                        curr_char_hash = curr_len_hash->char_hash[keys[i]];
                }

                if (len - i == 1) {
                        curr_char_hash->has_single_value = 1;
                        break;
                }

                inserted_index = 0;

                if (curr_char_hash->n_values++ == 0) {
                        curr_char_hash->data[0] = word_data;
                } else {
                        inserted_index = insert_word(curr_char_hash, word_data);
                }

                for (int j = i + 1; j < len; j++) {
                        if (j > i + 1) {
                                if (curr_inside_hash->inside_hash[keys[j]] == NULL)
                                        add_inside_hash_r(curr_inside_hash, keys[j]);

                                curr_inside_hash = curr_inside_hash->inside_hash[keys[j]];
                        } else {
                                curr_inside_hash = curr_char_hash->inside_hash[keys[j]];
                                if (curr_inside_hash == NULL) {
                                        add_inside_hash(curr_char_hash, keys[j]);
                                        curr_inside_hash = curr_char_hash->inside_hash[keys[j]];
                                }
                        }
                        if (curr_inside_hash->pos_map[keys[j]] == -1
                            || curr_inside_hash->pos_map[keys[j]] > inserted_index) {
                                curr_inside_hash->pos_map[keys[j]] = inserted_index;
                        }
                        update_inside_pos_map(curr_inside_hash, keys[j]);
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
        free_word_map();
        free_hashmap();
}

void pr()
{
        struct _word_data *node;

        for (node = word_map.head; node; node = node->next) {
                printf("%ls\n", node->str);
        }
}
