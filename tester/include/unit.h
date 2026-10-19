#ifndef UNIT_H
#define UNIT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STRINGS AND GARBAGE COLLECTOR
///////////////////////////////////////

typedef struct String
{
	char *ptr;
	size_t length;
	struct String *prev;
	struct String *next;
} String_t;

typedef struct StringGC
{
	String_t *ptr;
} StringGC_t;

static StringGC_t __gc = {.ptr = NULL};

static inline __attribute__((always_inline)) void __allocate(String_t *new)
{
	if (__gc.ptr)
	{
		__gc.ptr->next = new;
		new->prev = __gc.ptr;
		__gc.ptr = new;
	}
	else
	{
		__gc.ptr = new;
	}
}

static inline __attribute__((always_inline)) void __deallocate(String_t *old)
{
	if (old->prev)
		old->prev->next = old->next;
	if (old->next)
		old->next->prev = old->prev;
	if (__gc.ptr == old)
		__gc.ptr = old->prev;
}

static inline __attribute__((always_inline)) String_t *
str_init(const size_t size)
{
	String_t *res = malloc(sizeof(String_t));
	if (!res)
		return NULL;
	res->ptr = malloc(sizeof(char) * (size + 1));
	if (!res->ptr)
	{
		free(res);
		return NULL;
	}
	res->length = size;
	res->ptr[res->length] = 0;
	res->prev = NULL;
	res->next = NULL;
	__allocate(res);
	return res;
}

static inline __attribute__((always_inline)) String_t *
str_fromsrc(const char *src)
{
	const size_t length = strlen(src);
	String_t *res = str_init(length);
	if (!res)
		return NULL;
	memcpy(res->ptr, src, length + 1);
	return res;
}

static inline __attribute__((always_inline)) String_t *
str_mirror(const String_t *str)
{
	String_t *res = str_init(str->length);
	if (!res)
		return NULL;
	memcpy(res->ptr, str->ptr, str->length + 1);
	return res;
}

static inline __attribute__((always_inline)) void delstr(String_t *str)
{
	if (!str)
		return;
	str->length = 0;
	if (str->ptr)
		free(str->ptr);
	str->ptr = NULL;
	__deallocate(str);
	free(str);
}

static inline __attribute__((always_inline, destructor)) void __clear_gc(void)
{
	while (__gc.ptr)
		delstr(__gc.ptr);
}

// ASSERTING
///////////////////////////////////////

static inline __attribute__((always_inline)) bool
check(const char *name, bool ok, const char *expr, const char *file, int line)
{
	if (ok)
		fprintf(stderr, "\033[0;32m%s:%d: OK: %s (%s)\033[0m\n", file, line,
				name, expr);
	else
		fprintf(stderr, "\033[0;31m%s:%d: FAIL: %s (%s)\033[0m\n", file, line,
				name, expr);
	return ok;
}

#define ASSERT(name, expr) check((name), (expr), #expr, __FILE__, __LINE__)

#define ASSERT_SAME(name, f, g, ...)                                           \
	check((#f " " name), (f)(__VA_ARGS__) == (g)(__VA_ARGS__), #__VA_ARGS__,   \
		  __FILE__, __LINE__)

#endif
