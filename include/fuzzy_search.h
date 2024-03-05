#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "words.h"

int load_from_file_fz(const char *filename);
struct _search_resp *search_fz(const wchar_t *str, int mode);
void free_words_fz();
