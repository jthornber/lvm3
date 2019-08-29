#ifndef BASE_STRING_BUILDER_H
#define BASE_STRING_BUILDER_H

#include <stdbool.h>
#include <stdlib.h>

//----------------------------------------------------------------

struct string_builder {
	char *b, *e, *alloc_e;
};

bool string_builder_init(struct string_builder *sb, size_t hint);
void string_builder_exit(struct string_builder *sb);

char *string_builder_peek(struct string_builder *sb);

// takes ownership
char *string_builder_get(struct string_builder *sb);

bool string_builder_add_char(struct string_builder *sb, char c);
bool string_builder_add_tok(struct string_builder *sb, const char *b, const char *e);
bool string_builder_add_str(struct string_builder *sb, const char *str);

//----------------------------------------------------------------

#endif

