#ifndef ZEPHIR_KERNEL_ARRAY_H
#define ZEPHIR_KERNEL_ARRAY_H
#define ZEPHIR_MAX_ARRAY_LEVELS 16

zval *zephir_array_update_string(zval *arr, const char *index, size_t index_length, zval *value, int flags);
int ZEPHIR_FASTCALL zephir_array_unset(zval *arr, zval *index, int flags);
int ZEPHIR_FASTCALL zephir_array_unset_long(zval *arr, unsigned long index, int flags);

int zephir_array_fetch(zval *return_value, zval *arr, zval *index, int flags ZEPHIR_DEBUG_PARAMS);
int zephir_array_fetch_long(zval *return_value, zval *arr, unsigned long index, int flags ZEPHIR_DEBUG_PARAMS);
int zephir_array_fetch_string(zval *return_value, zval *arr, char *index, size_t length, int flags ZEPHIR_DEBUG_PARAMS);
int zephir_array_isset_string_fetch(zval *fetched, zval *arr, char *index, uint32_t index_length, int readonly);

int zephir_array_update_zval(zval *arr, zval *index, zval *value, int flags);
int zephir_array_update_multi(zval *arr, zval *value, const char *types, int types_length, int types_count, ...);
int zephir_array_update_multi_ex(zval *arr, zval *value, const char *types, int types_length, int types_count, va_list ap);

int zephir_array_append(zval *arr, zval *value, int flags ZEPHIR_DEBUG_PARAMS);
void zephir_fast_array_merge(zval *return_value, zval *array1, zval *array2);
void zephir_array_keys(zval *return_value, zval *input);

int ZEPHIR_FASTCALL zephir_array_unset_string(zval *arr, const char *index, uint32_t index_length, int flags);

int ZEPHIR_FASTCALL zephir_array_isset(const zval *arr, zval *index);
#define zephir_array_isset_fetch(return_value, arr, index, flags) zephir_array_fetch(return_value, arr, index, flags ZEPHIR_DEBUG_PARAMS_DUMMY)
#define zephir_array_fast_append(arr, value) \
  Z_ADDREF_P(&value); \
  zend_hash_next_index_insert(Z_ARRVAL_P(arr), &value);

#endif
