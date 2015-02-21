
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"
#include "php_main.h"
#include "ext/spl/spl_exceptions.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"

#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"

/**
 * Parses method parameters with minimum overhead
 */
int zephir_fetch_parameters(int num_args, int required_args, int optional_args, ...)
{
	va_list va;
	int arg_count = ZEND_CALL_NUM_ARGS(EG(current_execute_data));
	zval *arg, *p;
	int i;

	if (num_args < required_args || (num_args > (required_args + optional_args))) {
		zephir_throw_exception_string(spl_ce_BadMethodCallException, SL("Wrong number of parameters") TSRMLS_CC);
		return FAILURE;
	}

	if (num_args > arg_count) {
		zephir_throw_exception_string(spl_ce_BadMethodCallException, SL("Could not obtain parameters for parsing") TSRMLS_CC);
		return FAILURE;
	}

	if (!num_args) {
		return SUCCESS;
	}

	va_start(va, optional_args);

	i = 0;
	while (num_args-- > 0) {

		arg = ZEND_CALL_ARG(EG(current_execute_data), i + 1);

		p = va_arg(va, zval *);
		//*p = arg
		ZVAL_DUP(p, arg);
		i++;
	}

	va_end(va);

	return SUCCESS;
}

/**
 * Initialize an array to start an iteration over it
 */
int zephir_is_iterable_ex(zval *arr, int duplicate, int reverse) {

	if (unlikely(Z_TYPE_P(arr) != IS_ARRAY)) {
		return 0;
	}

	/*	TODO duplicate? reverse should be done already
		if (duplicate) {
		ALLOC_HASHTABLE(*arr_hash);
		zend_hash_init(*arr_hash, 0, NULL, NULL, 0);
		zend_hash_copy(*arr_hash, Z_ARRVAL_P(arr), NULL, NULL, sizeof(zval*));
	} else {
		*arr_hash = Z_ARRVAL_P(arr);
	}

	if (reverse) {
		if (hash_position) {
			*hash_position = (*arr_hash)->pListTail;
		} else {
			(*arr_hash)->pInternalPointer = (*arr_hash)->pListTail;
		}
	} else {
		if (hash_position) {
			*hash_position = (*arr_hash)->pListHead;
		} else {
			(*arr_hash)->pInternalPointer = (*arr_hash)->pListHead;
		}
	}*/

	return 1;
}
