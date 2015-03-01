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

/** Pass multiple parameters as one */
#define WRAP_ARG(a, b) a, b

/** Check if an array is iterable or not */
#define zephir_is_iterable(var, duplicate, reverse, file, line) \
	if (!var || !zephir_is_iterable_ex(var, duplicate, reverse)) { \
		ZEPHIR_THROW_EXCEPTION_DEBUG_STRW(zend_exception_get_default(), "The argument is not initialized or iterable()", file, line); \
		ZEPHIR_MM_RESTORE(); \
		return; \
	}
int zephir_is_iterable_ex(zval *arr, int duplicate, int reverse);
int zephir_is_callable(zval *var);
void zephir_gettype(zval *return_value, zval *arg);

int zephir_fast_count_int(zval *value TSRMLS_DC);


/** Return without change return_value */
#define RETURN_MM()                 { ZEPHIR_MM_RESTORE(); return; }
/** Return null restoring memory frame */
#define RETURN_MM_NULL()            { RETVAL_NULL(); ZEPHIR_MM_RESTORE(); return; }
/** Return null restoring memory frame */
#define RETURN_MM_BOOL(value)       { RETVAL_BOOL(value); ZEPHIR_MM_RESTORE(); return; }
/** Return string restoring memory frame */
#define RETURN_MM_STRING(str) { RETVAL_STRING(str); ZEPHIR_MM_RESTORE(); return; }
/* Return long */
#define RETURN_MM_LONG(value) { RETVAL_LONG(value); ZEPHIR_MM_RESTORE(); return; }
/* Return double */
#define RETURN_MM_DOUBLE(value)     { RETVAL_DOUBLE(value); ZEPHIR_MM_RESTORE(); return; }
/** Returns a zval in an object member */
#define RETURN_MM_MEMBER(object, member_name) \
  zephir_return_property(object, SL(member_name)); \
  RETURN_MM();

#define RETURN_MM_ON_FAILURE(what) \
	do { \
		if (what == FAILURE) { \
			ZEPHIR_MM_RESTORE(); \
			return; \
		} \
	} while (0)

/** Returns a zval in an object member  */
#define RETURN_MEMBER(object, member_name) \
	zephir_return_property(object, SL(member_name)); \
	return;

/** Return this pointer */
#define RETURN_THIS() { \
		RETVAL_ZVAL(this_ptr, 1, 0); \
	} \
	ZEPHIR_MM_RESTORE(); \
	return;

/** Return zval with always ctor, without restoring the memory stack */
#define RETURN_THISW() \
	RETURN_ZVAL(this_ptr, 1, 0);

/** Return zval with always ctor */
#define RETURN_CTOR(var) { \
		RETVAL_ZVAL(var, 1, 0); \
	} \
	ZEPHIR_MM_RESTORE(); \
	return;

/** Return zval with always ctor, without restoring the memory stack */
#define RETURN_CTORW(var) { \
		RETVAL_ZVAL(var, 1, 0); \
	} \
	return;

/** Return zval checking if it's needed to ctor */
#define RETURN_CCTOR(var) { \
		RETVAL_ZVAL(var, 1, 0); \
	} \
	ZEPHIR_MM_RESTORE(); \
	return;

#define RETURN_ON_FAILURE(what) \
	do { \
		if (what == FAILURE) { \
			return; \
		} \
	} while (0)

/** class/interface registering */
#define ZEPHIR_REGISTER_CLASS(ns, class_name, lower_ns, name, methods, flags) \
	{ \
		zend_class_entry ce; \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_NS_CLASS_ENTRY(ce, #ns, #class_name, methods); \
		lower_ns## _ ##name## _ce = zend_register_internal_class(&ce TSRMLS_CC); \
		lower_ns## _ ##name## _ce->ce_flags |= flags;  \
	}

#define ZEPHIR_REGISTER_CLASS_EX(ns, class_name, lower_ns, lcname, parent_ce, methods, flags) \
	{ \
		zend_class_entry ce; \
		if (!parent_ce) { \
			fprintf(stderr, "Can't register class %s::%s with null parent\n", #ns, #class_name); \
			return FAILURE; \
		} \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_NS_CLASS_ENTRY(ce, #ns, #class_name, methods); \
		lower_ns## _ ##lcname## _ce = zend_register_internal_class_ex(&ce, parent_ce); \
		if (!lower_ns## _ ##lcname## _ce) { \
			fprintf(stderr, "Zephir Error: Class to extend '%s' was not found when registering class '%s'\n", (parent_ce ? parent_ce->name->val : "(null)"), ZEND_NS_NAME(#ns, #class_name)); \
			return FAILURE; \
		} \
		lower_ns## _ ##lcname## _ce->ce_flags |= flags;  \
	}

#define ZEPHIR_REGISTER_INTERFACE(ns, classname, lower_ns, name, methods) \
	{ \
		zend_class_entry ce; \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_NS_CLASS_ENTRY(ce, #ns, #classname, methods); \
		lower_ns## _ ##name## _ce = zend_register_internal_interface(&ce TSRMLS_CC); \
	}

#define ZEPHIR_REGISTER_INTERFACE_EX(ns, classname, lower_ns, lcname, parent_ce, methods) \
	{ \
		zend_class_entry ce; \
		if (!parent_ce) { \
			fprintf(stderr, "Can't register interface %s with null parent\n", ZEND_NS_NAME(#ns, #classname)); \
			return FAILURE; \
		} \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_NS_CLASS_ENTRY(ce, #ns, #classname, methods); \
		lower_ns## _ ##lcname## _ce = zephir_register_internal_interface_ex(&ce, parent_ce TSRMLS_CC); \
		if (!lower_ns## _ ##lcname## _ce) { \
			fprintf(stderr, "Can't register interface %s with parent %s\n", ZEND_NS_NAME(#ns, #classname), (parent_ce ? parent_ce->name : "(null)")); \
			return FAILURE; \
		} \
	}

#define ZEPHIR_SET_SYMBOL(symbol_table, name, value) { \
	zval _set_symbol_t; \
	ZVAL_UNDEF(&_set_symbol_t); \
	ZEPHIR_CPY_WRT(&_set_symbol_t, value); \
	_zend_hash_str_update(symbol_table, SL(name), &_set_symbol_t ZEND_FILE_LINE_CC); \
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

inline int zephir_get_constant(zval *return_value, char *name, size_t len);
#define ZEPHIR_GET_CONSTANT(return_value, const_name) RETURN_MM_ON_FAILURE(zephir_get_constant(return_value, SL(const_name)));

/* Fetch Parameters */
int zephir_fetch_parameters(int num_args, int required_args, int optional_args, ...);

int zephir_get_global(zval *arr, const char *global, unsigned int global_length);

/* TODO: It's anyways only implemented for linux, implement win32+linux */
#ifndef zephir_print_backtrace
#define zephir_print_backtrace()
#endif

#ifndef ZEPHIR_RELEASE
	#define ZEPHIR_DEBUG_PARAMS , const char *file, int line
	#define ZEPHIR_DEBUG_PARAMS_DUMMY , "", 0
#else
	#define ZEPHIR_DEBUG_PARAMS , const char *file, int line
	#define ZEPHIR_DEBUG_PARAMS_DUMMY , "", 0
#endif

#endif
