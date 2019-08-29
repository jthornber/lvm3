#include "builder.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//----------------------------------------------------------------

static bool _ensure(struct string_builder *sb, size_t count)
{
	if ((sb->alloc_e - sb->e) < count) {
		size_t len = sb->e - sb->b;
		size_t new_len = ((sb->alloc_e - sb->b) + count) * 2;
		char *new = malloc(new_len);
		if (!new)
			return false;

		memcpy(new, sb->b, len);
		sb->e = new + len;
		sb->b = new;
		sb->alloc_e = sb->b + new_len;
	}

	return true;
}

static bool _terminate(struct string_builder *sb)
{
	if (!_ensure(sb, 1))
		return false;

	*sb->e = '\0';
	return true;
}

bool string_builder_init(struct string_builder *sb, size_t hint)
{
	sb->b = sb->e = malloc(hint);
	if (!sb->b)
		return false;

	sb->alloc_e = sb->b + hint;
	return true;
}

void string_builder_exit(struct string_builder *sb)
{
	free(sb->b);
}

char *string_builder_peek(struct string_builder *sb)
{
	if (!_terminate(sb))
		return NULL;

	return sb->b;
}

char *string_builder_get(struct string_builder *sb)
{
	char *r;

	if (!_terminate(sb))
		return NULL;

	r = sb->b;
	sb->b = NULL;
	return r;
}

bool string_builder_add_char(struct string_builder *sb, char c)
{
	if (_ensure(sb, 1)) {
		*sb->e = c;
		sb->e++;
		return true;
	}

	return false;
}

bool string_builder_add_tok(struct string_builder *sb, const char *b, const char *e)
{
	size_t len = e - b;

	if (_ensure(sb, len)) {
		memcpy(sb->e, b, e - b);
		sb->e += len;
	}

	return false;
}

bool string_builder_add_str(struct string_builder *sb, const char *str)
{
	size_t len = strlen(str);
	return string_builder_add_tok(sb, str, str + len);
}

//----------------------------------------------------------------
