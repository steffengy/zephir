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
//#include "kernel/array.h"
#include "kernel/operators.h"

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


