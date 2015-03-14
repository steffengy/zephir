
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
		zephir_throw_exception_string(spl_ce_BadMethodCallException, SL("Wrong number of parameters"));
		return FAILURE;
	}

	if (num_args > arg_count) {
		zephir_throw_exception_string(spl_ce_BadMethodCallException, SL("Could not obtain parameters for parsing"));
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

int zephir_fast_count_int(zval *array)
{
	zend_long cnt;

	switch (Z_TYPE_P(array)) {
			case IS_NULL:
				return 0;
			case IS_ARRAY:
				/* For now we only support NORMAL_MODE not RECURSIVE as below:
					cnt = zend_hash_num_elements(Z_ARRVAL_P(array));
					if (mode == COUNT_RECURSIVE) {
						ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array), element) {
							ZVAL_DEREF(element);
							cnt += php_count_recursive(element, COUNT_RECURSIVE);
						} ZEND_HASH_FOREACH_END();
				}*/
				return zend_hash_num_elements(Z_ARRVAL_P(array));
			case IS_OBJECT: {
	#ifdef HAVE_SPL
				zval retval;
	#endif
				/* first, we check if the handler is defined */
				if (Z_OBJ_HT_P(array)->count_elements) {
					if (SUCCESS == Z_OBJ_HT(*array)->count_elements(array, &cnt)) {
						return cnt;
					}
					return 1;
				}
	#ifdef HAVE_SPL
				/* if not and the object implements Countable we call its count() method */
				if (instanceof_function(Z_OBJCE_P(array), spl_ce_Countable)) {
					zend_call_method_with_0_params(array, NULL, NULL, "count", &retval);
					if (Z_TYPE(retval) != IS_UNDEF) {
						return zval_get_long(&retval);
						zval_ptr_dtor(&retval);
					}
					return 0;
				}
	#endif
			}
			default:
				return 1;
		}
}

/**
 * Checks if a zval is callable
 */
int zephir_is_callable(zval *var) {

	char *error = NULL;
	zend_bool retval;

	retval = zend_is_callable_ex(var, NULL, 0, NULL, NULL, &error);
	if (error) {
		efree(error);
	}

	return (int) retval;
}

/**
 * Returns the type of a variable as a string
 */
void zephir_gettype(zval *return_value, zval *arg_in)
{
	/* Make sure we handle references properly */
	zval *arg = arg_in;
	if (Z_REFCOUNTED_P(arg) && Z_ISREF_P(arg)) {
		arg = Z_REFVAL_P(arg_in);
	}
	switch (Z_TYPE_P(arg)) {

		case IS_NULL:
			RETVAL_STRING("NULL");
			break;

		case IS_TRUE:
		case IS_FALSE:
			RETVAL_STRING("boolean");
			break;

		case IS_LONG:
			RETVAL_STRING("integer");
			break;

		case IS_DOUBLE:
			RETVAL_STRING("double");
			break;

		case IS_STRING:
			RETVAL_STRING("string");
			break;

		case IS_ARRAY:
			RETVAL_STRING("array");
			break;

		case IS_OBJECT:
			RETVAL_STRING("object");
			break;

		case IS_RESOURCE:
			{
				const char *type_name = zend_rsrc_list_get_rsrc_type(Z_RES_P(arg));

				if (type_name) {
					RETVAL_STRING("resource");
					break;
				}
			}

		default:
			RETVAL_STRING("unknown type");
	}
}

/**
 * Gets the global zval into PG macro
 */
int zephir_get_global(zval *arr, const char *global, unsigned int global_length) {

	zval *gv;

	zend_bool jit_initialization = PG(auto_globals_jit);
	zend_string *global_str;

	ZVAL_UNDEF(arr);
	if (jit_initialization) {
		global_str = zend_string_init(global, global_length, 0);
		zend_is_auto_global(global_str);
		zend_string_release(global_str);
	}

	if (&EG(symbol_table)) {
		if ((gv = zend_hash_str_find(&EG(symbol_table), global, global_length)) != NULL) {
			if (Z_TYPE_P(gv) == IS_ARRAY) {
				ZVAL_DUP(arr, gv);
				if (Z_TYPE_P(arr) == IS_UNDEF) {
					array_init(arr);
				}
			} else {
				array_init(arr);
			}
			return SUCCESS;
		}
	}

	array_init(arr);

	return SUCCESS;
}

int ZEPHIR_FASTCALL zephir_get_constant(zval *return_value, char *name, size_t len)
{
	zval *const_zval;
	zend_string *const_str = zend_string_init(name, len, 0);
	const_zval = zend_get_constant(const_str);
	zend_string_release(const_str);
	if (const_zval == NULL) {
		return FAILURE;
	}
	ZVAL_DUP(return_value, const_zval);
	return SUCCESS;
}

zend_class_entry* zephir_get_internal_ce(const char *class_name, unsigned int class_name_len)
{
    zend_class_entry* temp_ce;

    if ((temp_ce = zend_hash_str_find_ptr(CG(class_table), class_name, class_name_len)) == NULL) {
        zend_error(E_ERROR, "Class '%s' not found", class_name);
        return NULL;
    }

    return temp_ce;
}
