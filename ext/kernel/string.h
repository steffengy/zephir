#ifndef ZEPHIR_KERNEL_STRING_H
#define ZEPHIR_KERNEL_STRING_H

#include <Zend/zend.h>

#define ZEPHIR_TRIM_LEFT  1
#define ZEPHIR_TRIM_RIGHT 2
#define ZEPHIR_TRIM_BOTH  3

void zephir_uncamelize(zval *return_value, const zval *str);

/** Function replacement */
int zephir_fast_strlen_ev(zval *str);
void zephir_fast_join_ex(zval *result, zend_string *glue, zval *pieces);
void ZEPHIR_FASTCALL zephir_fast_join_str(zval *return_value, char *glue, unsigned int glue_length, zval *pieces);
void zephir_fast_explode(zval *return_value, zval *delimiter, zval *str, long limit);
void zephir_fast_explode_str(zval *return_value, const char *delimiter, int delimiter_length, zval *str, long limit);
void zephir_fast_trim(zval *return_value, zval *str, zval *charlist, int where);
void zephir_fast_strpos(zval *return_value, const zval *haystack, const zval *needle, unsigned int offset);
void zephir_fast_strtolower(zval *return_value, zval *str);
void zephir_fast_strtoupper(zval *return_value, zval *str);
void zephir_fast_str_replace(zval *return_value, zval *search, zval *replace, zval *subject);
void zephir_addslashes(zval *return_value, zval *str);
void zephir_stripslashes(zval *return_value, zval *str);
int zephir_memnstr_str(const zval *haystack, char *needle, unsigned int needle_length ZEPHIR_DEBUG_PARAMS);

int zephir_start_with_str(const zval *str, char *compared, unsigned int compared_length);

int zephir_json_encode(zval *return_value, zval *v, int opts);
int zephir_json_decode(zval *return_value, zval *v, zend_bool assoc);

#define zephir_fast_join(result, glue, pieces) \
	if (Z_TYPE_P(glue) != IS_STRING) zend_error(E_ERROR, "Invalid arguments supplied for join(glue)"); \
	zephir_fast_join_ex(result, Z_STR_P(glue), pieces);

#endif
