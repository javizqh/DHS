#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "words.h"

int load_from_file(const char *filename);
struct _search_resp *search(const wchar_t *str, int mode);
void free_maps();
