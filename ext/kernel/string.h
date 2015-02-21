#ifndef ZEPHIR_KERNEL_STRING_H
#define ZEPHIR_KERNEL_STRING_H

#include <Zend/zend.h>

#define ZEPHIR_TRIM_LEFT  1
#define ZEPHIR_TRIM_RIGHT 2
#define ZEPHIR_TRIM_BOTH  3

/** Function replacement */
int zephir_json_encode(zval *return_value, zval *v, int opts);
int zephir_json_decode(zval *return_value, zval *v, zend_bool assoc);

#endif
