#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "words.h"

int load_from_file_stdc(const char *filename);
struct _search_resp *search_stdc(const wchar_t *str, int mode);
void free_words_stdc();
