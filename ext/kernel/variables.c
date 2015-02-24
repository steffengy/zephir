#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"

#include "ext/standard/php_smart_string.h"
#include "ext/standard/php_var.h"

zend_always_inline void zephir_smart_str_0(smart_str *str) {
	if (str->s) {
		str->s->val[str->s->len] = '\0';
	}
}

/**
 * var_export returns php variables without using the PHP userland
 */
void zephir_var_export_ex(zval *return_value, zval *var) {

    smart_str buf = { NULL };

    php_var_export_ex(var, 1, &buf TSRMLS_CC);
    zephir_smart_str_0(&buf);
    ZVAL_STR(return_value, buf.s);
}
