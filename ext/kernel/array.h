#ifndef ZEPHIR_KERNEL_ARRAY_H
#define ZEPHIR_KERNEL_ARRAY_H
#define ZEPHIR_MAX_ARRAY_LEVELS 16

zval *zephir_array_update_string(zval *arr, const char *index, size_t index_length, zval *value, int flags);
int zephir_array_update_multi(zval *arr, zval *value, const char *types, int types_length, int types_count, ...);

#define zephir_array_fast_append(arr, value) \
  Z_ADDREF_P(&value); \
  zend_hash_next_index_insert(Z_ARRVAL_P(arr), &value);

#endif
