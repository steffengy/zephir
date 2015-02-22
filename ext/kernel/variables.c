#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"

#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_var.h"

/**
 * var_export returns php variables without using the PHP userland
 */
void zephir_var_export_ex(zval *return_value, zval *var) {

    smart_str buf = { NULL };

    php_var_export_ex(var, 1, &buf TSRMLS_CC);
    smart_str_0(&buf);
    ZVAL_STR(return_value, buf.s);
}