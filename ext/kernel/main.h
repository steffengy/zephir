#ifndef ZEPHIR_KERNEL_MAIN_H
#define ZEPHIR_KERNEL_MAIN_H

#include "Zend/zend_interfaces.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/spl/spl_iterators.h"

/** Main macros */
#define PH_DEBUG 0

#define PH_NOISY 256
#define PH_SILENT 1024
#define PH_READONLY 4096

#define PH_NOISY_CC PH_NOISY TSRMLS_CC
#define PH_SILENT_CC PH_SILENT TSRMLS_CC

#define PH_SEPARATE 256
#define PH_COPY 1024
#define PH_CTOR 4096

#include "Zend/zend_constants.h"
#include "kernel/exception.h"

#define SS(str) ZEND_STRS(str)
#define SL(str) ZEND_STRL(str)

/** Check if an array is iterable or not */
#define zephir_is_iterable(var, duplicate, reverse, file, line) \
	if (!var || !zephir_is_iterable_ex(var, duplicate, reverse)) { \
		ZEPHIR_THROW_EXCEPTION_DEBUG_STRW(zend_exception_get_default(), "The argument is not initialized or iterable()", file, line); \
		ZEPHIR_MM_RESTORE(); \
		return; \
	}
int zephir_is_iterable_ex(zval *arr, int duplicate, int reverse);

/** Return null restoring memory frame */
#define RETURN_MM_NULL()            { RETVAL_NULL(); ZEPHIR_MM_RESTORE(); return; }

/** class/interface registering */
#define ZEPHIR_REGISTER_CLASS(ns, class_name, lower_ns, name, methods, flags) \
	{ \
		zend_class_entry ce; \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_NS_CLASS_ENTRY(ce, #ns, #class_name, methods); \
		lower_ns## _ ##name## _ce = zend_register_internal_class(&ce TSRMLS_CC); \
		lower_ns## _ ##name## _ce->ce_flags |= flags;  \
	}

/** Low overhead parse/fetch parameters */
#define zephir_fetch_params(memory_grow, required_params, optional_params, ...) \
	if (zephir_fetch_parameters(ZEND_NUM_ARGS(), required_params, optional_params, __VA_ARGS__) == FAILURE) { \
		if (memory_grow) { \
			RETURN_MM_NULL(); \
		} else { \
			RETURN_NULL(); \
		} \
	}

/* Fetch Parameters */
int zephir_fetch_parameters(int num_args, int required_args, int optional_args, ...);

/* TODO: It's anyways only implemented for linux, implement win32+linux */
#ifndef zephir_print_backtrace
#define zephir_print_backtrace()
#endif

#endif
