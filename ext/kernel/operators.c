
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/php_string.h"
#include "php_ext.h"
#include "kernel/main.h"
#include "kernel/memory.h"
//#include "kernel/string.h"
#include "kernel/operators.h"

#include "Zend/zend_operators.h"

int zephir_is_equal(zval *op1, zval *op2)
{
	zval result;
	fast_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE;
}
