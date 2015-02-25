
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"
#include "php_main.h"

#include "kernel/main.h"
#include "kernel/exit.h"

void zephir_exit_empty() {
	zend_bailout();
}

void zephir_exit(zval *ptr)  {
	if (Z_TYPE_P(ptr) == IS_LONG) {
		EG(exit_status) = Z_LVAL_P(ptr);
	} else {
		zend_print_variable(ptr);
	}
	zephir_exit_empty();
}
