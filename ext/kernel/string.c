#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <ctype.h>

#include <php.h>
#include "php_ext.h"
#include "php_main.h"

#include <ext/standard/php_smart_string.h>
#include <ext/standard/php_string.h>
#include <ext/standard/php_rand.h>
#include <ext/standard/php_lcg.h>
#include <ext/standard/php_http.h>
#include "ext/standard/base64.h"
#include "ext/standard/md5.h"
#include "ext/standard/url.h"
#include "ext/standard/html.h"
#include "ext/date/php_date.h"

#ifdef ZEPHIR_USE_PHP_PCRE
#include <ext/pcre/php_pcre.h>
#endif

#include <ext/json/php_json.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"

#define PH_RANDOM_ALNUM 0
#define PH_RANDOM_ALPHA 1
#define PH_RANDOM_HEXDEC 2
#define PH_RANDOM_NUMERIC 3
#define PH_RANDOM_NOZERO 4

int zephir_json_encode(zval *return_value, zval *v, int opts)
{
	smart_str buf = { 0 };

	php_json_encode(&buf, v, opts);
	smart_str_0(&buf);
	ZVAL_STR(return_value, buf.s);
	//TODO: missing free of buf? check memleak

	return SUCCESS;
}

int zephir_json_decode(zval *return_value, zval *v, zend_bool assoc)
{
	zval copy;
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(v) != IS_STRING)) {
		use_copy = zend_make_printable_zval(v, &copy);
		if (use_copy) {
			v = &copy;
		}
	}
	php_json_decode(return_value, Z_STRVAL_P(v), Z_STRLEN_P(v), assoc, 512 /* JSON_PARSER_DEFAULT_DEPTH */);
	if (unlikely(use_copy)) {
		zval_dtor(&copy);
	}

	return SUCCESS;
}

/* PHP strlen wrapper */
int zephir_fast_strlen_ev(zval *str)
{
	zval copy;
	int use_copy = 0, length;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	length = Z_STRLEN_P(str);
	if (use_copy) {
		zval_dtor(str);
	}

	return length;
}

/** Wrapper for php trim() */
void zephir_fast_trim(zval *return_value, zval *str, zval *charlist, int where)
{
	zval copy;
	zend_string *ret;
	int use_copy = 0;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	//PHPAPI zend_string *php_trim(zend_string *str, char *what, size_t what_len, int mode)
	if (charlist && Z_TYPE_P(charlist) == IS_STRING) {
		ret = php_trim(Z_STR_P(str),  Z_STRVAL_P(charlist), Z_STRLEN_P(charlist), where);
	} else {
		ret = php_trim(Z_STR_P(str), NULL, 0, where);
	}

	ZVAL_STR(return_value, ret);
	if (use_copy) {
		zval_dtor(&copy);
	}
}

/**
 * Fast call to php join  function
 */
void zephir_fast_join_ex(zval *result, zend_string *glue, zval *pieces)
{
	if (Z_TYPE_P(pieces) != IS_ARRAY) {
		ZVAL_NULL(result);
		zend_error(E_WARNING, "Invalid arguments supplied for join()");
		return;
	}

	php_implode(glue, pieces, result);
}

/**
 * Fast join function
 * This function is an adaption of the php_implode function
 *
 */
void ZEPHIR_FASTCALL zephir_fast_join_str(zval *return_value, char *glue, unsigned int glue_length, zval *pieces)
{
	/* Maybe implement the fast method again? */
	zend_string *zglue = zend_string_init(glue, glue_length, 0);
	zephir_fast_join_ex(return_value, zglue, pieces);
	zend_string_release(zglue);
}

/**
 * Inmediate function resolution for strpos function
 */
void zephir_fast_strpos(zval *return_value, const zval *haystack, const zval *needle, unsigned int offset)
{
	const char *found = NULL;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING || Z_TYPE_P(needle) != IS_STRING)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Invalid arguments supplied for strpos()");
		return;
	}

	if (offset > Z_STRLEN_P(haystack)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Offset not contained in string");
		return;
	}

	if (!Z_STRLEN_P(needle)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Empty delimiter");
		return;
	}
	found = php_memnstr(Z_STRVAL_P(haystack)+offset, Z_STRVAL_P(needle), Z_STRLEN_P(needle), Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack));

	if (found) {
		ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
	} else {
		ZVAL_BOOL(return_value, 0);
	}
}

/** explode wrapper */
void zephir_fast_explode(zval *return_value, zval *delimiter, zval *str, long limit)
{
	if (unlikely(Z_TYPE_P(str) != IS_STRING || Z_TYPE_P(delimiter) != IS_STRING)) {
		zend_error(E_WARNING, "Invalid arguments supplied for explode()");
		RETURN_EMPTY_STRING();
	}

	array_init(return_value);
	php_explode(Z_STR_P(delimiter), Z_STR_P(str), return_value, limit);
}

/** explode wrapper */
void zephir_fast_explode_str(zval *return_value, const char *delimiter, int delimiter_length, zval *str, long limit)
{
	zend_string *delimiter_str;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		zend_error(E_WARNING, "Invalid arguments supplied for explode()");
		RETURN_EMPTY_STRING();
	}
	delimiter_str = zend_string_init(delimiter, delimiter_length, 0);
	array_init(return_value);
	php_explode(delimiter_str, Z_STR_P(str), return_value, limit);
	zend_string_release(delimiter_str);
}

/** addslashes wrapper */
void zephir_addslashes(zval *return_value, zval *str)
{
	zval copy;
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}
	ZVAL_STR(return_value, php_addslashes(Z_STR_P(str), 0));

	if (unlikely(use_copy)) {
		zval_dtor(&copy);
	}
}

/* stripslashes wrapper */
void zephir_stripslashes(zval *return_value, zval *str)
{
	zval copy;
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	ZVAL_STR(return_value, Z_STR_P(str));
	php_stripslashes(Z_STR_P(return_value));

	if (unlikely(use_copy)) {
		zval_dtor(&copy);
	}
}

/**
 * PHP_IMPORTED_BLOCK
 * taken from php-src: {{{ php_str_replace_in_subject + dependencies
 * since it's all marked static
 * see: https://github.com/php/php-src/ext/standard/string.c
 */
static zend_string *php_str_to_str_ex(zend_string *haystack,
	char *needle, size_t needle_len, char *str, size_t str_len, zend_long *replace_count)
{
	zend_string *new_str;

	if (needle_len < haystack->len) {
		char *end;
		char *e, *s, *p, *r;

		if (needle_len == str_len) {
			new_str = NULL;
			end = haystack->val + haystack->len;
			for (p = haystack->val; (r = (char*)php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				if (!new_str) {
					new_str = zend_string_init(haystack->val, haystack->len, 0);
				}
				memcpy(new_str->val + (r - haystack->val), str, str_len);
				(*replace_count)++;
			}
			if (!new_str) {
				goto nothing_todo;
			}
			return new_str;
		} else {
			size_t count = 0;
			char *o = haystack->val;
			char *n = needle;
			char *endp = o + haystack->len;

			while ((o = (char*)php_memnstr(o, n, needle_len, endp))) {
				o += needle_len;
				count++;
			}
			if (count == 0) {
				/* Needle doesn't occur, shortcircuit the actual replacement. */
				goto nothing_todo;
			}
			new_str = zend_string_alloc(count * (str_len - needle_len) + haystack->len, 0);

			e = s = new_str->val;
			end = haystack->val + haystack->len;
			for (p = haystack->val; (r = (char*)php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				memcpy(e, p, r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
				(*replace_count)++;
			}

			if (p < end) {
				memcpy(e, p, end - p);
				e += end - p;
			}

			*e = '\0';
			return new_str;
		}
	} else if (needle_len > haystack->len || memcmp(haystack->val, needle, haystack->len)) {
nothing_todo:
		return zend_string_copy(haystack);
	} else {
		new_str = zend_string_init(str, str_len, 0);
		(*replace_count)++;
		return new_str;
	}
}
static zend_string *php_str_to_str_i_ex(zend_string *haystack, char *lc_haystack,
	zend_string *needle, char *str, size_t str_len, zend_long *replace_count)
{
	zend_string *new_str = NULL;
	zend_string *lc_needle;

	if (needle->len < haystack->len) {
		char *end;
		char *e, *s, *p, *r;

		if (needle->len == str_len) {
			lc_needle = php_string_tolower(needle);
			end = lc_haystack + haystack->len;
			for (p = lc_haystack; (r = (char*)php_memnstr(p, lc_needle->val, lc_needle->len, end)); p = r + lc_needle->len) {
				if (!new_str) {
					new_str = zend_string_init(haystack->val, haystack->len, 0);
				}
				memcpy(new_str->val + (r - lc_haystack), str, str_len);
				(*replace_count)++;
			}
			zend_string_release(lc_needle);

			if (!new_str) {
				goto nothing_todo;
			}
			return new_str;
		} else {
			size_t count = 0;
			char *o = lc_haystack;
			char *n;
			char *endp = o + haystack->len;

			lc_needle = php_string_tolower(needle);
			n = lc_needle->val;

			while ((o = (char*)php_memnstr(o, n, lc_needle->len, endp))) {
				o += lc_needle->len;
				count++;
			}
			if (count == 0) {
				/* Needle doesn't occur, shortcircuit the actual replacement. */
				zend_string_release(lc_needle);
				goto nothing_todo;
			}

			new_str = zend_string_alloc(count * (str_len - lc_needle->len) + haystack->len, 0);

			e = s = new_str->val;
			end = lc_haystack + haystack->len;

			for (p = lc_haystack; (r = (char*)php_memnstr(p, lc_needle->val, lc_needle->len, end)); p = r + lc_needle->len) {
				memcpy(e, haystack->val + (p - lc_haystack), r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
				(*replace_count)++;
			}

			if (p < end) {
				memcpy(e, haystack->val + (p - lc_haystack), end - p);
				e += end - p;
			}
			*e = '\0';

			zend_string_release(lc_needle);

			return new_str;
		}
	} else if (needle->len > haystack->len) {
nothing_todo:
		return zend_string_copy(haystack);
	} else {
		lc_needle = php_string_tolower(needle);

		if (memcmp(lc_haystack, lc_needle->val, lc_needle->len)) {
			zend_string_release(lc_needle);
			goto nothing_todo;
		}
		zend_string_release(lc_needle);

		new_str = zend_string_init(str, str_len, 0);

		(*replace_count)++;
		return new_str;
	}
}
static zend_string* php_char_to_str_ex(zend_string *str, char from, char *to, size_t to_len, int case_sensitivity, zend_long *replace_count)
{
	zend_string *result;
	size_t char_count = 0;
	char lc_from = 0;
	char *source, *target, *source_end= str->val + str->len;

	if (case_sensitivity) {
		char *p = str->val, *e = p + str->len;
		while ((p = memchr(p, from, (e - p)))) {
			char_count++;
			p++;
		}
	} else {
		lc_from = tolower(from);
		for (source = str->val; source < source_end; source++) {
			if (tolower(*source) == lc_from) {
				char_count++;
			}
		}
	}

	if (char_count == 0) {
		return zend_string_copy(str);
	}

	if (to_len > 0) {
		result = zend_string_safe_alloc(char_count, to_len - 1, str->len, 0);
	} else {
		result = zend_string_alloc(str->len - char_count, 0);
	}
	target = result->val;

	if (case_sensitivity) {
		char *p = str->val, *e = p + str->len, *s = str->val;
		while ((p = memchr(p, from, (e - p)))) {
			memcpy(target, s, (p - s));
			target += p - s;
			memcpy(target, to, to_len);
			target += to_len;
			p++;
			s = p;
			if (replace_count) {
				*replace_count += 1;
			}
		}
		if (s < e) {
			memcpy(target, s, (e - s));
			target += e - s;
		}
	} else {
		for (source = str->val; source < source_end; source++) {
			if (tolower(*source) == lc_from) {
				if (replace_count) {
					*replace_count += 1;
				}
				memcpy(target, to, to_len);
				target += to_len;
			} else {
				*target = *source;
				target++;
			}
		}
	}
	*target = 0;
	return result;
}
static zend_long php_str_replace_in_subject(zval *search, zval *replace, zval *subject, zval *result, int case_sensitivity)
{
	zval		*search_entry,
				*replace_entry = NULL;
	zend_string	*tmp_result,
				*replace_entry_str = NULL;
	char		*replace_value = NULL;
	size_t		 replace_len = 0;
	zend_long	 replace_count = 0;
	zend_string	*subject_str;
	zend_string *lc_subject_str = NULL;
	uint32_t     replace_idx;

	/* Make sure we're dealing with strings. */
	subject_str = zval_get_string(subject);
	if (subject_str->len == 0) {
		zend_string_release(subject_str);
		ZVAL_EMPTY_STRING(result);
		return 0;
	}

	/* If search is an array */
	if (Z_TYPE_P(search) == IS_ARRAY) {
		/* Duplicate subject string for repeated replacement */
		ZVAL_STR_COPY(result, subject_str);

		if (Z_TYPE_P(replace) == IS_ARRAY) {
			replace_idx = 0;
		} else {
			/* Set replacement value to the passed one */
			replace_value = Z_STRVAL_P(replace);
			replace_len = Z_STRLEN_P(replace);
		}

		/* For each entry in the search array, get the entry */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(search), search_entry) {
			/* Make sure we're dealing with strings. */
			ZVAL_DEREF(search_entry);
			SEPARATE_ZVAL_NOREF(search_entry);
			convert_to_string(search_entry);
			if (Z_STRLEN_P(search_entry) == 0) {
				if (Z_TYPE_P(replace) == IS_ARRAY) {
					replace_idx++;
				}
				continue;
			}

			/* If replace is an array. */
			if (Z_TYPE_P(replace) == IS_ARRAY) {
				/* Get current entry */
				while (replace_idx < Z_ARRVAL_P(replace)->nNumUsed) {
					replace_entry = &Z_ARRVAL_P(replace)->arData[replace_idx].val;
					if (Z_TYPE_P(replace_entry) != IS_UNDEF) {
						break;
					}
					replace_idx++;
				}
				if (replace_idx < Z_ARRVAL_P(replace)->nNumUsed) {
					/* Make sure we're dealing with strings. */
					replace_entry_str = zval_get_string(replace_entry);

					/* Set replacement value to the one we got from array */
					replace_value = replace_entry_str->val;
					replace_len = replace_entry_str->len;

					replace_idx++;
				} else {
					/* We've run out of replacement strings, so use an empty one. */
					replace_value = "";
					replace_len = 0;
				}
			}

			if (Z_STRLEN_P(search_entry) == 1) {
				zend_long old_replace_count = replace_count;

				tmp_result = php_char_to_str_ex(Z_STR_P(result),
								Z_STRVAL_P(search_entry)[0],
								replace_value,
								replace_len,
								case_sensitivity,
								&replace_count);
				if (lc_subject_str && replace_count != old_replace_count) {
					zend_string_release(lc_subject_str);
					lc_subject_str = NULL;
				}
			} else if (Z_STRLEN_P(search_entry) > 1) {
				if (case_sensitivity) {
					tmp_result = php_str_to_str_ex(Z_STR_P(result),
							Z_STRVAL_P(search_entry), Z_STRLEN_P(search_entry),
							replace_value, replace_len, &replace_count);
				} else {
					zend_long old_replace_count = replace_count;

					if (!lc_subject_str) {
						lc_subject_str = php_string_tolower(Z_STR_P(result));
					}
					tmp_result = php_str_to_str_i_ex(Z_STR_P(result), lc_subject_str->val,
							Z_STR_P(search_entry), replace_value, replace_len, &replace_count);
					if (replace_count != old_replace_count) {
						zend_string_release(lc_subject_str);
						lc_subject_str = NULL;
					}
				}
			}

			if (replace_entry_str) {
				zend_string_release(replace_entry_str);
				replace_entry_str = NULL;
			}
			zend_string_release(Z_STR_P(result));
			ZVAL_STR(result, tmp_result);

			if (Z_STRLEN_P(result) == 0) {
				if (lc_subject_str) {
					zend_string_release(lc_subject_str);
				}
				zend_string_release(subject_str);
				return replace_count;
			}
		} ZEND_HASH_FOREACH_END();
		if (lc_subject_str) {
			zend_string_release(lc_subject_str);
		}
	} else {
		if (Z_STRLEN_P(search) == 1) {
			ZVAL_STR(result,
				php_char_to_str_ex(subject_str,
							Z_STRVAL_P(search)[0],
							Z_STRVAL_P(replace),
							Z_STRLEN_P(replace),
							case_sensitivity,
							&replace_count));
		} else if (Z_STRLEN_P(search) > 1) {
			if (case_sensitivity) {
				ZVAL_STR(result, php_str_to_str_ex(subject_str,
						Z_STRVAL_P(search), Z_STRLEN_P(search),
						Z_STRVAL_P(replace), Z_STRLEN_P(replace), &replace_count));
			} else {
				lc_subject_str = php_string_tolower(Z_STR_P(subject));
				ZVAL_STR(result, php_str_to_str_i_ex(subject_str, lc_subject_str->val,
						Z_STR_P(search),
						Z_STRVAL_P(replace), Z_STRLEN_P(replace), &replace_count));
				zend_string_release(lc_subject_str);
			}
		} else {
			ZVAL_STR_COPY(result, subject_str);
		}
	}
	zend_string_release(subject_str);
	return replace_count;
}
/* }}} END_PHP_IMPORTED_BLOCK */


/* str_replace wrapper based on php-src, unfortunately marked as static */
void zephir_fast_str_replace(zval *return_value, zval *search, zval *replace, zval *subject)
{
	int case_sensitivity = 1;
	zval *subject_entry;
	zval result;
	zend_string *string_key;
	zend_ulong num_key;
	zend_long count = 0;

	/* Make sure we're dealing with strings and do the replacement. */
	if (Z_TYPE_P(search) != IS_ARRAY) {
		convert_to_string_ex(search);
		if (Z_TYPE_P(replace) != IS_STRING) {
			convert_to_string_ex(replace);
		}
	} else if (Z_TYPE_P(replace) != IS_ARRAY) {
		convert_to_string_ex(replace);
	}

	/* if subject is an array */
	if (Z_TYPE_P(subject) == IS_ARRAY) {
		array_init(return_value);

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(subject), num_key, string_key, subject_entry) {
			if (Z_TYPE_P(subject_entry) != IS_ARRAY && Z_TYPE_P(subject_entry) != IS_OBJECT) {
				count += php_str_replace_in_subject(search, replace, subject_entry, &result, case_sensitivity);
			} else {
				ZVAL_COPY(&result, subject_entry);
			}
			/* Add to return array */
			if (string_key) {
				zend_hash_add_new(Z_ARRVAL_P(return_value), string_key, &result);
			} else {
				zend_hash_index_add_new(Z_ARRVAL_P(return_value), num_key, &result);
			}
		} ZEND_HASH_FOREACH_END();
	} else {	/* if subject is not an array */
		count = php_str_replace_in_subject(search, replace, subject, return_value, case_sensitivity);
	}
}

/**
 * Check if a string is contained into another
 */
int zephir_memnstr_str(const zval *haystack, char *needle, unsigned int needle_length ZEPHIR_DEBUG_PARAMS) {

	if (Z_TYPE_P(haystack) != IS_STRING) {
		#ifndef ZEPHIR_RELEASE
		zend_error(E_WARNING, "Invalid arguments supplied for memnstr in %s on line %d", file, line);
		#else
		zend_error(E_WARNING, "Invalid arguments supplied for memnstr()");
		#endif
		return 0;
	}

	if (Z_STRLEN_P(haystack) >= needle_length) {
		return php_memnstr(Z_STRVAL_P(haystack), needle, needle_length, Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack)) ? 1 : 0;
	}

	return 0;
}

/**
 * Checks if a zval string starts with a string
 */
int zephir_start_with_str(const zval *str, char *compared, unsigned int compared_length)
{
	if (Z_TYPE_P(str) != IS_STRING || compared_length > Z_STRLEN_P(str)) {
		return 0;
	}

	return !memcmp(Z_STRVAL_P(str), compared, compared_length);
}

/**
 * Fast call to php strlen
 */
void zephir_fast_strtolower(zval *return_value, zval *str)
{
	zval copy;
	int use_copy = 0;
	char *lower_str;
	unsigned int length;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	length = Z_STRLEN_P(str);
	lower_str = estrndup(Z_STRVAL_P(str), length);
	php_strtolower(lower_str, length);

	if (use_copy) {
		zval_dtor(str);
	}

	ZVAL_STRINGL(return_value, lower_str, length);
}

/**
 * Fast call to PHP strtoupper() function
 */
void zephir_fast_strtoupper(zval *return_value, zval *str)
{
	zval copy;
	int use_copy = 0;
	char *lower_str;
	unsigned int length;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	length = Z_STRLEN_P(str);
	lower_str = estrndup(Z_STRVAL_P(str), length);
	php_strtoupper(lower_str, length);

	if (use_copy) {
		zval_dtor(str);
	}

	ZVAL_STRINGL(return_value, lower_str, length);
}

/**
 * Convert dash/underscored texts returning camelized
 */
void zephir_uncamelize(zval *return_value, const zval *str)
{
	unsigned int i;
	smart_str uncamelize_str = {0};
	char *marker, ch;

	if (Z_TYPE_P(str) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for camelize()");
		return;
	}

	marker = Z_STRVAL_P(str);
	for (i = 0; i < Z_STRLEN_P(str); i++) {
		ch = *marker;
		if (ch == '\0') {
			break;
		}
		if (ch >= 'A' && ch <= 'Z') {
			if (i > 0) {
				smart_str_appendc(&uncamelize_str, '_');
			}
			smart_str_appendc(&uncamelize_str, (*marker) + 32);
		} else {
			smart_str_appendc(&uncamelize_str, (*marker));
		}
		marker++;
	}
	smart_str_0(&uncamelize_str);

	if (uncamelize_str.s) {
		RETURN_STR(uncamelize_str.s);
	} else {
		RETURN_EMPTY_STRING();
	}
}
