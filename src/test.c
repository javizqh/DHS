#define _GNU_SOURCE
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include "dhs.h"
#include "fuzzy_search.h"
#include "standard_c_search.h"
#include "test.h"
#include <time.h>

#define CSV_PERMISSIONS 0777

#define N_TESTS 100
#define TEST_REPETITIONS 1000

#define SECTONANO   1000000000
#define MILISTONANO 1000000

#define BUFFER_SIZE 1024
#define MAX_FILENAME 1024

struct info {
        engines search_engine;
        int search_type;
        int matches;
        long latency;
        struct info *next;
};

struct test_info {
        wchar_t *str;
        int search_mode;
};

const struct test_info tests[N_TESTS] =
    { {L"balust", SEARCH_EXACT}, {L"quepace", SEARCH_INSIDE}, {L"zab", SEARCH_INSIDE}, {L"phloeum",
                                                                                        SEARCH_EXACT},
{L"ah", SEARCH_INSIDE}, {L"hs", SEARCH_INSIDE}, {L"dilla", SEARCH_INSIDE}, {L"ka",
                                                                            SEARCH_EXACT},
{L"nate", SEARCH_INSIDE}, {L"centenaria", SEARCH_INSIDE}, {L"yus", SEARCH_INSIDE}, {L"rec",
                                                                                    SEARCH_INSIDE},
{L"def", SEARCH_EXACT}, {L"ling", SEARCH_INSIDE}, {L"un", SEARCH_INSIDE}, {L"bal",
                                                                           SEARCH_INSIDE},
{L"ehemip", SEARCH_INSIDE}, {L"fue", SEARCH_EXACT}, {L"ugnab", SEARCH_INSIDE}, {L"eng",
                                                                                SEARCH_INSIDE},
{L"mágico", SEARCH_INSIDE}, {L"ela", SEARCH_EXACT}, {L"falsarr", SEARCH_EXACT}, {L"carne",
                                                                                  SEARCH_INSIDE},
{L"petis", SEARCH_INSIDE}, {L"otate", SEARCH_INSIDE}, {L"ignívomo", SEARCH_EXACT}, {L"miento",
                                                                                     SEARCH_INSIDE},
{L"áfi", SEARCH_INSIDE}, {L"ca", SEARCH_INSIDE}, {L"inhi", SEARCH_EXACT}, {L"lada",
                                                                            SEARCH_INSIDE},
{L"petaquera", SEARCH_INSIDE}, {L"uflac", SEARCH_EXACT}, {L"nsit", SEARCH_INSIDE}, {L"spacl",
                                                                                    SEARCH_EXACT},
{L"ucha", SEARCH_INSIDE}, {L"esti", SEARCH_EXACT}, {L"neuquino", SEARCH_INSIDE}, {L"rate",
                                                                                  SEARCH_INSIDE},
{L"立", SEARCH_EXACT}, {L"模擬", SEARCH_INSIDE}, {L"諾", SEARCH_INSIDE}, {L"不動",
                                                                              SEARCH_INSIDE},
{L"先者", SEARCH_EXACT}, {L"白", SEARCH_EXACT}, {L"集合", SEARCH_INSIDE},
{L"高式", SEARCH_INSIDE}, {L"検査", SEARCH_EXACT}, {L"族", SEARCH_INSIDE}, {L"顔曇",
                                                                                 SEARCH_INSIDE},
{L"踞", SEARCH_INSIDE}, {L"カルノ", SEARCH_EXACT}, {L"人事", SEARCH_INSIDE}, {L"曙",
                                                                                    SEARCH_INSIDE},
{L"小惑帯", SEARCH_EXACT}, {L"遣会", SEARCH_INSIDE}, {L"涙", SEARCH_INSIDE},
{L"倶紋々", SEARCH_EXACT}, {L"薔", SEARCH_INSIDE}, {L"ege", SEARCH_EXACT}, {L"or",
                                                                                SEARCH_EXACT},
{L"ntacruc", SEARCH_INSIDE}, {L"ton", SEARCH_EXACT}, {L"尖端恐怖症", SEARCH_EXACT},
{L"喧々", SEARCH_INSIDE}, {L"peres", SEARCH_INSIDE}, {L"pilu", SEARCH_EXACT}, {L"自業",
                                                                                 SEARCH_INSIDE},
{L"rom", SEARCH_EXACT}, {L"noncomplacency", SEARCH_INSIDE}, {L"furl", SEARCH_INSIDE}, {L"bt",
                                                                                       SEARCH_INSIDE},
{L"載", SEARCH_INSIDE}, {L"一", SEARCH_EXACT}, {L"gro", SEARCH_INSIDE}, {L"光頭",
                                                                           SEARCH_INSIDE},
{L"美も寸", SEARCH_EXACT}, {L"nwee", SEARCH_EXACT}, {L"ón", SEARCH_INSIDE}, {L"a",
                                                                                 SEARCH_INSIDE},
{L"b", SEARCH_EXACT}, {L"l", SEARCH_INSIDE}, {L"l", SEARCH_EXACT}, {L"l", SEARCH_INSIDE},
{L"ñ", SEARCH_INSIDE}, {L"hua", SEARCH_INSIDE}, {L"awo", SEARCH_EXACT}, {L"lpdk",
                                                                          SEARCH_INSIDE},
{L"ñpa", SEARCH_INSIDE}, {L"eure", SEARCH_EXACT}, {L"sdjioa", SEARCH_INSIDE}, {L"ji",
                                                                                SEARCH_INSIDE},
{L"aiw", SEARCH_INSIDE}, {L"wiue", SEARCH_INSIDE}, {L"udio", SEARCH_INSIDE}, {L"dji",
                                                                              SEARCH_EXACT},
{L"lpq", SEARCH_INSIDE}, {L"qwi", SEARCH_INSIDE}, {L"ola", SEARCH_INSIDE}
};

const char *test_files[4] =
    { "test/Words/all/all%d.txt", "test/Words/es/es%d.txt", "test/Words/en/en%d.txt",
        "test/Words/kanji/kanji%d.txt"
};

long int get_nanoseconds(struct timespec start, struct timespec end);
long int get_seconds(struct timespec start, struct timespec end);
int write_legend(int csv_fd);
int write_to_csv(int csv_fd, engines search_engine, int search_type, int matches, long latency);
struct info *new_info(engines search_engine, int search_type);
struct info *add_info(struct info *last_info, engines search_engine, int search_type);

struct info *new_info(engines search_engine, int search_type)
{
        struct info *create_info;

        create_info = malloc(sizeof(struct info));
        if (create_info == NULL) {
                err(EXIT_FAILURE, "Failed to allocate info");
        }
        memset(create_info, 0, sizeof(struct info));

        create_info->search_engine = search_engine;
        create_info->search_type = search_type;
        create_info->matches = 0;
        create_info->latency = 0;
        create_info->next = NULL;

        return create_info;
}

struct info *add_info(struct info *last_info, engines search_engine, int search_type)
{
        struct info *new = new_info(search_engine, search_type);

        if (last_info)
                last_info->next = new;
        return new;
}

// Returns the number of nanoseconds passed
long int get_nanoseconds(struct timespec start, struct timespec end)
{
        return (end.tv_sec - start.tv_sec) * SECTONANO + (end.tv_nsec - start.tv_nsec);
}

// Returns the number of seconds passed
long int get_seconds(struct timespec start, struct timespec end)
{
        return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / SECTONANO;
}

int write_legend(int csv_fd)
{
        char *buf = malloc(BUFFER_SIZE);
        int msg_len = 0;

        if (buf == NULL) {
                err(EXIT_FAILURE, "Couldn't allocate memory");
        }
        memset(buf, 0, BUFFER_SIZE);

        sprintf(buf, "SEARCH_ENGINE,SEARCH_MODE,N_MATCHES,SEARCH_TIME\n");
        msg_len = strlen(buf);
        write(csv_fd, buf, msg_len);
        free(buf);
        return 1;
}

int write_to_csv(int csv_fd, engines search_engine, int search_type, int matches, long latency)
{
        char *buf = malloc(BUFFER_SIZE);
        int msg_len = 0;

        if (buf == NULL) {
                err(EXIT_FAILURE, "Couldn't allocate memory");
        }
        memset(buf, 0, BUFFER_SIZE);

        sprintf(buf, "%d,%d,%d,%ld\n", search_engine, search_type, matches, latency);
        msg_len = strlen(buf);
        write(csv_fd, buf, msg_len);
        free(buf);
        return 1;
}

typedef struct _search_resp *(*_search_func) (const wchar_t *str, int mode);
typedef int (*_load_func)(const char *filename);
typedef void (*_free_func)(void);

int test(engines search_engine, int size_sample)
{
        int csv_fd;
        char *base_file;
        char *load_file;
        struct timespec begin, end;
        struct info *data = NULL;
        struct info *last_data = data;
        struct _search_resp *search_results;

        _search_func search_func = NULL;
        _load_func load_func = NULL;
        _free_func free_func = NULL;

        base_file = malloc(MAX_FILENAME);
        if (base_file == NULL)
                err(EXIT_FAILURE, "Malloc failed");
        memset(base_file, 0, MAX_FILENAME);

        load_file = malloc(MAX_FILENAME);
        if (load_file == NULL)
                err(EXIT_FAILURE, "Malloc failed");
        memset(load_file, 0, MAX_FILENAME);

        sprintf(base_file, "data/%d/", size_sample);

        switch (search_engine) {
        case DDHS:
                strcat(base_file, "ddhs.csv");
                search_func = search;
                load_func = load_from_file;
                free_func = free_maps;
                break;
        case FUZZY:
                strcat(base_file, "fuzzy.csv");
                search_func = search_fz;
                load_func = load_from_file_fz;
                free_func = free_words_fz;
                break;
        case STDC:
                strcat(base_file, "stdc.csv");
                search_func = search_stdc;
                load_func = load_from_file_stdc;
                free_func = free_words_stdc;
                break;
        }

        csv_fd = open(base_file, O_CREAT | O_RDWR | O_TRUNC, CSV_PERMISSIONS);

        if (csv_fd < 0) {
                err(EXIT_FAILURE, "Couldn't open the necessary files");
        }

        write_legend(csv_fd);

        for (size_t k = 0; k < 4; k++) {
                sprintf(load_file, test_files[k], size_sample);
                load_func(load_file);

                for (int i = 0; i < N_TESTS; i++) {
                        for (int j = 0; j < TEST_REPETITIONS; j++) {
                                clock_gettime(CLOCK_MONOTONIC, &begin);
                                search_results = search_func(tests[i].str, tests[i].search_mode);
                                clock_gettime(CLOCK_MONOTONIC, &end);
                                // if (search_results && search_results->len == 0)
                                //         continue;

                                last_data = add_info(last_data, search_engine, SEARCH_INSIDE);
                                if (search_results)
                                        last_data->matches = search_results->len;
                                last_data->latency = get_nanoseconds(begin, end);
                                if (data == NULL)
                                        data = last_data;
                        }
                }

                free_func();
        }

        for (struct info * curr = data; curr; curr = curr->next) {
                write_to_csv(csv_fd, curr->search_engine, curr->search_type, curr->matches,
                             curr->latency);
        }

        close(csv_fd);

        return 0;
}
