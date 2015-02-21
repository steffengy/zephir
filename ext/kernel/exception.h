#ifndef ZEPHIR_KERNEL_EXCEPTIONS_H
#define ZEPHIR_KERNEL_EXCEPTIONS_H

#include "Zend/zend.h"

#define ZEPHIR_THROW_EXCEPTION_DEBUG_STRW(class_entry, message, file, line) \
	zephir_throw_exception_string_debug(class_entry, message, strlen(message), file, line)

void zephir_throw_exception_string_debug(zend_class_entry *ce, const char *message, uint32_t message_len, const char *file, uint32_t line);
void zephir_throw_exception_string(zend_class_entry *ce, const char *message, uint32_t message_len);
void zephir_throw_exception_format(zend_class_entry *ce TSRMLS_DC, const char *format, ...);

#endif
