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
		//*value = new_zv;
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

int zephir_array_append(zval *arr, zval *value, int flags) {

	if (Z_TYPE_P(arr) != IS_ARRAY) {
		zend_error(E_WARNING, "Cannot use a scalar value as an array in %s on line %d", __FILE__, __LINE__);
		return FAILURE;
	}

	if ((flags & PH_SEPARATE) == PH_SEPARATE) {
		SEPARATE_ZVAL_IF_NOT_REF(arr);
	}

	if (Z_REFCOUNTED_P(value))  Z_ADDREF_P(value);
	return add_next_index_zval(arr, value);
}

/**
 * Multiple array-offset update
 */
int zephir_array_update_multi(zval *arr, zval *value, const char *types, int types_length, int types_count, ...)
{
	va_list ap;
	zval *fetched = NULL;
	zval *p = arr;
	int i;
	size_t len = 0;

	SEPARATE_ZVAL_IF_NOT_REF(arr);

	va_start(ap, types_count);
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
	va_end(ap);
	return 0;
}

