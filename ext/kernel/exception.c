
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"
#include "php_main.h"
#include "ext/standard/php_string.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

#include "Zend/zend_exceptions.h"

/**
 * Throws an exception with a single string parameter + debug info
 */
void zephir_throw_exception_string_debug(zend_class_entry *ce, const char *message, uint32_t message_len, const char *file, uint32_t line) {

	zval object, msg;
	int ZEPHIR_LAST_CALL_STATUS = 0;
	zend_class_entry *default_exception_ce;

	object_init_ex(&object, ce);
	ZVAL_STRINGL(&msg, message, message_len);

	ZEPHIR_CALL_METHOD(NULL, &object, "__construct", NULL, &msg);
	zephir_check_call_status();

	if (line > 0) {
		default_exception_ce = zend_exception_get_default();
		zend_update_property_string(default_exception_ce, &object, "file", sizeof("file")-1, file);
		zend_update_property_long(default_exception_ce, &object, "line", sizeof("line")-1, line);
	}

	zend_throw_exception_object(&object);

	zval_ptr_dtor(&msg);
}

/**
 * Throws an exception with a single string parameter
 */
void zephir_throw_exception_string(zend_class_entry *ce, const char *message, uint32_t message_len){

	zval object, msg;
	int ZEPHIR_LAST_CALL_STATUS = 0;

	object_init_ex(&object, ce);

	ZVAL_STRINGL(&msg, message, message_len);

	ZEPHIR_CALL_METHOD(NULL, &object, "__construct", NULL, &msg);
	zephir_check_call_status();

	zend_throw_exception_object(&object);

	zval_ptr_dtor(&msg);
}

/**
 * Throws an exception with a string format as parameter
 */
void zephir_throw_exception_format(zend_class_entry *ce, const char *format, ...) {

	zval object, msg;
	int ZEPHIR_LAST_CALL_STATUS = 0, len;
	char *buffer;
	va_list args;

	object_init_ex(&object, ce);

	va_start(args, format);
	len = vspprintf(&buffer, 0, format, args);
	va_end(args);

	ZVAL_STRINGL(&msg, buffer, len);

	ZEPHIR_CALL_METHOD(NULL, &object, "__construct", NULL, &msg);
	zephir_check_call_status();

	zend_throw_exception_object(&object);

	zval_ptr_dtor(&msg);
}

/**
 * Throws a zval object as exception
 */
void zephir_throw_exception_debug(zval *object, const char *file, uint32_t line) {

	zend_class_entry *default_exception_ce;
	int ZEPHIR_LAST_CALL_STATUS = 0;
	zval curline;

	ZEPHIR_MM_GROW();
	ZVAL_UNDEF(&curline);

	if (Z_REFCOUNTED_P(object)) Z_ADDREF_P(object);

	if (line > 0) {
		ZEPHIR_CALL_METHOD(&curline, object, "getline", NULL);
		zephir_check_call_status();
		if (ZEPHIR_IS_LONG(&curline, 0)) {
			default_exception_ce = zend_exception_get_default();
			zend_update_property_string(default_exception_ce, object, "file", sizeof("file")-1, file);
			zend_update_property_long(default_exception_ce, object, "line", sizeof("line")-1, line);
		}
	}

	zend_throw_exception_object(object);
	ZEPHIR_MM_RESTORE();
}
