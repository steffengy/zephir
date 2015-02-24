#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include "php_ext.h"
#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
//#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/operators.h"

inline zend_class_entry *zephir_fetch_class_str_ex(char *class_name, size_t length, int fetch_type)
{
	zend_class_entry *retval;
	zend_string *str = zend_string_init(class_name, length, 0);
	retval = zephir_fetch_class(str, fetch_type);
	zend_string_release(str);
	return retval;
}

int zephir_unset_property(zval* object, const char* name)
{
	if (Z_TYPE_P(object) == IS_OBJECT) {
		zval member;
		zend_class_entry *old_scope;

		ZVAL_STRING(&member, name);
		old_scope = EG(scope);
		EG(scope) = Z_OBJCE_P(object);

		Z_OBJ_HT_P(object)->unset_property(object, &member, NULL);

		EG(scope) = old_scope;

		return SUCCESS;
	}

	return FAILURE;
}

/**
 * Returns the called in class in the current scope
 */
void zephir_get_called_class(zval *return_value) {

	if (EG(current_execute_data)->called_scope) {
		RETURN_STR(EG(current_execute_data)->called_scope->name);
	}

	if (!EG(scope))  {
		php_error_docref(NULL, E_WARNING, "zephir_get_called_class() called from outside a class");
	}
}

/*
 * Lookup the exact class where a property is defined
 *
 */
inline zend_class_entry *zephir_lookup_class_ce(zend_class_entry *ce, const char *property_name, size_t len) {

	zend_class_entry *original_ce = ce;

	while (ce) {
		if (zend_hash_str_exists(&ce->properties_info, property_name, len)) {
			return ce;
		}
		ce = ce->parent;
	}
	return original_ce;
}

int zephir_instance_of_ev(const zval *object, const zend_class_entry *ce) {

	if (Z_TYPE_P(object) != IS_OBJECT) {
		php_error_docref(NULL, E_WARNING, "instanceof expects an object instance");
		return 0;
	}

	return instanceof_function(Z_OBJCE_P(object), ce);
}

zval *zephir_read_property(zval *target, zval *src, const char *name, size_t length)
{
	zval tmp;
	zval *ret;

	ZVAL_UNDEF(&tmp);
	ret = zend_read_property(Z_OBJCE_P(src), src, name, length, 0, &tmp);
	ZEPHIR_CPY_WRT_CTOR(target, ret);

	return ret;
}

/**
 * Reads a property from an object
 */
int zephir_read_property_zval(zval *result, zval *object, zval *property, int flags)
{
	if (unlikely(Z_TYPE_P(property) != IS_STRING)) {

		if ((flags & PH_NOISY) == PH_NOISY) {
			php_error_docref(NULL, E_NOTICE, "Cannot access empty property %d", Z_TYPE_P(property));
		}

		ZVAL_NULL(result);
		Z_ADDREF_P(result);
		return FAILURE;
	}

	return zephir_read_property(result, object, WRAP_ARG(Z_STRVAL_P(property), Z_STRLEN_P(property))) != NULL;
}

/**
 * Updates an array property
 */
int zephir_update_property_array(zval *object, const char *property, uint32_t property_length, const zval *index, zval *value) {

	zval tmp;
	ZVAL_UNDEF(&tmp);

	if (Z_TYPE_P(object) == IS_OBJECT) {
		zephir_read_property(&tmp, object, property, property_length);

		SEPARATE_ZVAL(&tmp);

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) != IS_ARRAY) {
			convert_to_array(&tmp);
		}

		if (Z_REFCOUNTED_P(value)) Z_ADDREF_P(value);

		if (Z_TYPE_P(index) == IS_STRING) {
			zend_symtable_str_update(Z_ARRVAL(tmp), Z_STRVAL_P(index), Z_STRLEN_P(index) + 1, value);
		} else if (Z_TYPE_P(index) == IS_LONG) {
			zend_hash_index_update(Z_ARRVAL(tmp), Z_LVAL_P(index), value);
		} else if (Z_TYPE_P(index) == IS_NULL) {
			zend_hash_next_index_insert(Z_ARRVAL(tmp), value);
		}

		zephir_update_property_zval(object, WRAP_ARG(property, property_length), &tmp);
		zval_ptr_dtor(&tmp);
	}

	return SUCCESS;
}

/**
 * Appends a zval value to an array property
 */
int zephir_update_property_array_append(zval *object, char *property, unsigned int property_length, zval *value) {

	zval tmp;
	int separated = 0;

	if (Z_TYPE_P(object) != IS_OBJECT) {
		return SUCCESS;
	}
	zephir_read_property(&tmp, object, WRAP_ARG(property, property_length));

	SEPARATE_ZVAL(&tmp);

	/** Convert the value to array if not is an array */
	if (Z_TYPE(tmp) != IS_ARRAY) {
		convert_to_array(&tmp);
	}

	add_next_index_zval(&tmp, value);

	zephir_update_property_zval(object, WRAP_ARG(property, property_length), &tmp);

	return SUCCESS;
}

int zephir_update_property_array_multi(zval *object, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...) {
	va_list ap;
	zval tmp;
	ZVAL_UNDEF(&tmp);

	if (Z_TYPE_P(object) == IS_OBJECT) {
		zephir_read_property(&tmp, object, WRAP_ARG(property, property_length));

		SEPARATE_ZVAL(&tmp);

		/** Convert the value to array if not is an array */
		if (Z_TYPE(tmp) != IS_ARRAY) {
			convert_to_array(&tmp);
		}

		va_start(ap, types_count);

		//int zephir_array_update_multi_ex(zval *arr, zval *value, const char *types, int types_length, int types_count, va_list ap);
		zephir_array_update_multi_ex(&tmp, value, types, types_length, types_count, ap);
		zephir_update_property_zval(object, WRAP_ARG(property, property_length), &tmp);

		va_end(ap);
		zval_ptr_dtor(&tmp);
	}
	return SUCCESS;
}
