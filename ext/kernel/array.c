#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include "php_ext.h"
#include <ext/standard/php_array.h>
#include <Zend/zend_hash.h>

#include "kernel/main.h"
#include "kernel/memory.h"
//#include "kernel/debug.h"
#include "kernel/array.h"
#include "kernel/operators.h"
//#include "kernel/hash.h"
//#include "kernel/backtrace.h"

int zephir_array_update_long(zval *arr, unsigned long index, zval *value, int flags ZEPHIR_DEBUG_PARAMS){

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array in %s on line %d", file, line);
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval new_zv;
		if (Z_REFCOUNTED_P(value)) Z_DELREF_P(value);
		ZVAL_NULL(&new_zv);
		ZVAL_COPY_VALUE(&new_zv, value);
		value = &new_zv;
		//zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		if (Z_REFCOUNTED_P(value)) Z_ADDREF_P(value);
	}

	return zend_hash_index_update(Z_ARRVAL_P(arr), index, value) != NULL;
}

zval *zephir_array_update_string(zval *arr, const char *index, size_t index_length, zval *value, int flags)
{
	if (Z_REFCOUNTED_P(arr) && Z_ISREF_P(arr)) {
		ZVAL_DEREF(arr);
	}
	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (%d)", Z_TYPE_P(arr));
		return NULL;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval new_zv;
		if (Z_REFCOUNTED_P(value)) Z_DELREF_P(value);
		ZVAL_NULL(&new_zv);
		ZVAL_COPY_VALUE(&new_zv, value);
		value = &new_zv;
		//zval_copy_ctor(new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		if (Z_REFCOUNTED_P(value)) Z_ADDREF_P(value);
	}
	return zend_symtable_str_update(Z_ARRVAL_P(arr), index, index_length, value);
}

int ZEPHIR_FASTCALL zephir_array_unset(zval *arr, zval *index, int flags) {

	HashTable *ht;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	ht = Z_ARRVAL_P(arr);

	switch (Z_TYPE_P(index)) {
		//case IS_NULL:
		//	return (zend_hash_del(ht, "", 1) == SUCCESS);

		case IS_DOUBLE:
			return (zend_hash_index_del(ht, (ulong)Z_DVAL_P(index)) == SUCCESS);

		case IS_TRUE:
		case IS_FALSE:
			return (zend_hash_index_del(ht, Z_TYPE_P(index) == IS_TRUE ? 1 : 0) == SUCCESS);

		case IS_LONG:
		case IS_RESOURCE:
			return (zend_hash_index_del(ht, Z_LVAL_P(index)) == SUCCESS);

		case IS_STRING:
			return (zend_symtable_del(ht, Z_STR_P(index)) == SUCCESS);

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}
}

int ZEPHIR_FASTCALL zephir_array_unset_long(zval *arr, unsigned long index, int flags) {

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return 0;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	return zend_hash_index_del(Z_ARRVAL_P(arr), index);
}

int zephir_array_fetch(zval *return_value, zval *arr, zval *index, int flags ZEPHIR_DEBUG_PARAMS) {

	zval *zv;
	HashTable *ht;
	int result;
	ulong uidx = 0;

	if (Z_TYPE_P(arr) == IS_ARRAY) {
		ht = Z_ARRVAL_P(arr);
		switch (Z_TYPE_P(index)) {
			/* TODO case IS_NULL:
				result = zephir_hash_find(ht, SS(""), (void**) &zv);
				sidx   = "";
				break;*/

			case IS_DOUBLE:
				uidx   = (zend_ulong) Z_DVAL_P(index);
				zv = zend_hash_index_find(ht, uidx);
				result = zv != NULL ? SUCCESS : FAILURE;
				break;

			case IS_TRUE:
			case IS_FALSE:
				zv = zend_hash_index_find(ht, Z_TYPE_P(arr) == IS_TRUE ? 1 : 0);
				result = zv != NULL ? SUCCESS : FAILURE;
				break;

			case IS_LONG:
			case IS_RESOURCE:
				uidx   = Z_LVAL_P(index);
				zv = zend_hash_index_find(ht, uidx);
				result = zv != NULL ? SUCCESS : FAILURE;
				break;

			case IS_STRING:
				zv = zend_symtable_find(ht, Z_STR_P(index));
				result = zv != NULL ? SUCCESS : FAILURE;
				break;

			default:
				//if ((flags & PH_NOISY) == PH_NOISY) {
				zend_error(E_WARNING, "Illegal offset type in %s on line %d", file, line);
				//}
				result = FAILURE;
				break;
		}

		if (result != FAILURE) {
			ZVAL_COPY_VALUE(return_value, zv);
			if ((flags & PH_READONLY) != PH_READONLY) {
				if (Z_REFCOUNTED_P(return_value)) Z_ADDREF_P(return_value);
			}
			return SUCCESS;
		}
	}
	ZVAL_NULL(return_value);
	return FAILURE;
}

int zephir_array_fetch_long(zval *return_value, zval *arr, unsigned long index, int flags ZEPHIR_DEBUG_PARAMS)
{
	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_hash_index_find(Z_ARRVAL_P(arr), index)) != NULL) {
			ZVAL_COPY_VALUE(return_value, zv);
			if ((flags & PH_READONLY) != PH_READONLY) {
				if (Z_REFCOUNTED_P(return_value)) Z_ADDREF_P(return_value);
			}
			return SUCCESS;
		}

		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %lu in %s on line %d", index, file, line);
		}
	}
	else {
		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array in %s on line %d", file, line);
		}
	}

	ZVAL_NULL(return_value);
	if ((flags & PH_READONLY) != PH_READONLY) {
		if (Z_REFCOUNTED_P(return_value)) Z_ADDREF_P(return_value);
	}
	return FAILURE;
}

int zephir_array_fetch_string(zval *return_value, zval *arr, char *index, size_t length, int flags ZEPHIR_DEBUG_PARAMS)
{
	zval *zv;

	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		if ((zv = zend_symtable_str_find(Z_ARRVAL_P(arr), index, length)) != NULL) {
			ZVAL_COPY_VALUE(return_value, zv);
			if ((flags & PH_READONLY) != PH_READONLY) {
				if (Z_REFCOUNTED_P(return_value)) Z_ADDREF_P(return_value);
			}
			return SUCCESS;
		}

		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Undefined index: %lu in %s on line %d", index, file, line);
		}
	}
	else {
		if ((flags & PH_NOISY) == PH_NOISY) {
			zend_error(E_NOTICE, "Cannot use a scalar value as an array in %s on line %d", file, line);
		}
	}

	ZVAL_NULL(return_value);
	if ((flags & PH_READONLY) != PH_READONLY) {
		if (Z_REFCOUNTED_P(return_value)) Z_ADDREF_P(return_value);
	}
	return FAILURE;
}

int zephir_array_append(zval *arr, zval *value, int flags) {

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array in %s on line %d", __FILE__, __LINE__);
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if (Z_REFCOUNTED_P(value)) Z_ADDREF_P(value);
	return add_next_index_zval(arr, value);
}

/**
 * Multiple array-offset update
 */
int zephir_array_update_multi_ex(zval *arr, zval *value, const char *types, int types_length, int types_count, va_list ap)
{
	zval *fetched = NULL;
	zval *p = arr;
	int i;
	size_t len = 0;

	SEPARATE_ZVAL_IF_NOT_REF(arr);

	for (i = 0; i < types_length; ++i)
	{
		char *arg = NULL;
		zend_bool must_free = 0;

		switch (types[i]) {
			case 's':
				arg = va_arg(ap, char*);
				len = va_arg(ap, int);
				break;
			case 'l':
				len = spprintf(&arg, 0, "%ld\0", va_arg(ap, long));
				must_free = 1;
				break;
			//case 'z':
			case 'a':
				arg = NULL;
				break;
			default:
				zend_error(E_ERROR, "type %c not supported zephir_array_update_multi yet", types[i]);
				break;
		}
		if (i == types_length - 1 && arg == NULL) {
			zephir_array_append(p, value, 0);
			continue;
		}
		if (i == types_length - 1) {
			zephir_array_update_string(p, arg, len, value, PH_SEPARATE);
			if (must_free) efree(arg);
			continue;
		}
		if ((fetched = zend_symtable_str_find(Z_ARRVAL_P(p), arg, len)) == NULL)
		{
			zval subarr;

			array_init(&subarr);
			Z_ADDREF_P(&subarr);
			p = zephir_array_update_string(p, arg, len, &subarr, PH_SEPARATE);
			zval_ptr_dtor(&subarr);
		} else {
			p = fetched;
		}
		if (must_free) efree(arg);
	}
	return 0;
}

int zephir_array_update_multi(zval *arr, zval *value, const char *types, int types_length, int types_count, ...)
{
	va_list ap;

	va_start(ap, types_count);
	zephir_array_update_multi_ex(arr, value, types, types_length, types_count, ap);
	va_end(ap);
	return 0;
}

int ZEPHIR_FASTCALL zephir_array_isset(const zval *arr, zval *index) {

	HashTable *h;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return 0;
	}

	h = Z_ARRVAL_P(arr);
	switch (Z_TYPE_P(index)) {
		//case IS_NULL:
			//TODO: return zephir_hash_exists(h, SS(""));

		case IS_DOUBLE:
			return zend_hash_index_exists(h, (ulong)Z_DVAL_P(index));

		case IS_TRUE:
		case IS_FALSE:
			return zend_hash_index_exists(h, Z_TYPE_P(index) == IS_TRUE ? 1 : 0);

		case IS_LONG:
		case IS_RESOURCE:
			return zend_hash_index_exists(h, Z_LVAL_P(index));

		case IS_STRING:
			return zend_symtable_exists(h, Z_STR_P(index));

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return 0;
	}
}

int ZEPHIR_FASTCALL zephir_array_isset_long(const zval *arr, unsigned long index)
{
	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_index_exists(Z_ARRVAL_P(arr), index);
	}

	return 0;
}

int ZEPHIR_FASTCALL zephir_array_isset_string(const zval *arr, const char *index, uint32_t index_length)
{
	if (likely(Z_TYPE_P(arr) == IS_ARRAY)) {
		return zend_hash_str_exists(Z_ARRVAL_P(arr), index, index_length);
	}
	return 0;
}

int zephir_array_isset_string_fetch(zval *fetched, zval *arr, char *index, uint32_t index_length, int readonly)
{
	ZVAL_NULL(fetched);
	return zephir_array_fetch_string(fetched, arr, index, index_length, readonly ZEPHIR_DEBUG_PARAMS_DUMMY);
}

int zephir_array_update_zval(zval *arr, zval *index, zval *value, int flags) {

	HashTable *ht;

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array (2)");
		return FAILURE;
	}

	if ((flags & PH_CTOR) == PH_CTOR) {
		zval new_zv;
		Z_DELREF_P(value);
		ZVAL_NULL(&new_zv);
		value = &new_zv;
		zval_copy_ctor(&new_zv);
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if ((flags & PH_COPY) == PH_COPY) {
		Z_ADDREF_P(value);
	}

	ht = Z_ARRVAL_P(arr);

	switch (Z_TYPE_P(index)) {
		/*TODO case IS_NULL:
			return zend_symtable_update(ht, "", 1, value, sizeof(zval*), NULL);
			*/

		case IS_DOUBLE:
			return zend_hash_index_update(ht, (ulong)Z_DVAL_P(index), value) != NULL;

		case IS_TRUE:
			return zend_hash_index_update(ht, 1, value) != NULL;

		case IS_FALSE:
			return zend_hash_index_update(ht, 0, value) != NULL;

		case IS_LONG:
		case IS_RESOURCE:
			return zend_hash_index_update(ht, Z_LVAL_P(index), value) != NULL;

		case IS_STRING:
			return zend_symtable_update(ht, Z_STR_P(index), value) != NULL;

		default:
			zend_error(E_WARNING, "Illegal offset type");
			return FAILURE;
	}
}

int ZEPHIR_FASTCALL zephir_array_unset_string(zval *arr, const char *index, uint32_t index_length, int flags)
{
	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return 0;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	return zend_hash_str_del(Z_ARRVAL_P(arr), index, index_length);
}

/* wrapper of array_merge */
void zephir_fast_array_merge(zval *return_value, zval *array1, zval *array2)
{
	int init_size, num;

	if (Z_TYPE_P(array1) != IS_ARRAY) {
		zend_error(E_WARNING, "First argument is not an array");
		RETURN_NULL();
	}

	if (Z_TYPE_P(array2) != IS_ARRAY) {
		zend_error(E_WARNING, "Second argument is not an array");
		RETURN_NULL();
	}

	init_size = zend_hash_num_elements(Z_ARRVAL_P(array1));
	num = zend_hash_num_elements(Z_ARRVAL_P(array2));
	if (num > init_size) {
		init_size = num;
	}

	array_init_size(return_value, init_size);
	php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_P(array1));
	php_array_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_P(array2));
}

void zephir_array_keys(zval *return_value, zval *input)
{
	zval
		 *search_value = NULL,	/* Value to search for */
		 *entry,				/* An entry in the input array */
		   res,					/* Result of comparison */
		   new_val;				/* New value */
	zend_bool strict = 0;		/* do strict comparison */
	zend_ulong num_idx;
	zend_string *str_idx;

	/* Initialize return array */
	if (search_value != NULL) {
		array_init(return_value);

		if (strict) {
			ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(input), num_idx, str_idx, entry) {
				fast_is_identical_function(&res, search_value, entry);
				if (Z_TYPE(res) == IS_TRUE) {
					if (str_idx) {
						ZVAL_STR_COPY(&new_val, str_idx);
					} else {
						ZVAL_LONG(&new_val, num_idx);
					}
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &new_val);
				}
			} ZEND_HASH_FOREACH_END();
		} else {
			ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(input), num_idx, str_idx, entry) {
				if (fast_equal_check_function(search_value, entry)) {
					if (str_idx) {
						ZVAL_STR_COPY(&new_val, str_idx);
					} else {
						ZVAL_LONG(&new_val, num_idx);
					}
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &new_val);
				}
			} ZEND_HASH_FOREACH_END();
		}
	} else {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(input)));
		zend_hash_real_init(Z_ARRVAL_P(return_value), 1);
		ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(return_value)) {
			/* Go through input array and add keys to the return array */
			ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(input), num_idx, str_idx, entry) {
				if (str_idx) {
					ZVAL_STR_COPY(&new_val, str_idx);
				} else {
					ZVAL_LONG(&new_val, num_idx);
				}
				ZEND_HASH_FILL_ADD(&new_val);
			} ZEND_HASH_FOREACH_END();
		} ZEND_HASH_FILL_END();
	}
}
