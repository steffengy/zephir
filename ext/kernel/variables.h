#ifndef ZEPHIR_KERNEL_VARIABLES_H
#define ZEPHIR_KERNEL_VARIABLES_H

#include "ext/standard/php_var.h"

#define zephir_var_dump(var) php_var_dump(var, 1)
#define zephir_var_export(var) php_var_export(var, 1)

void zephir_var_export_ex(zval *return_value, zval *var);

#endif
