#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

void init_maps();
void free_maps();
int add_word(const wchar_t *str);
void pr();
