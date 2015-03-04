
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"
#include "ext/standard/php_string.h"
#include "ext.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/concat.h"

void zephir_concat_sv(zval *result, const char *op1, uint32_t op1_len, zval *op2, int self_var){

	zval result_copy, op2_copy;
	int use_copy = 0, use_copy2 = 0;
	zend_string *tmp_str;
	uint offset = 0, length;

	if (Z_TYPE_P(op2) != IS_STRING) {
	   use_copy2 = zend_make_printable_zval(op2, &op2_copy);
	   if (use_copy2) {
	       op2 = &op2_copy;
	   }
	}

	length = op1_len + Z_STRLEN_P(op2);
	if (self_var) {
		if (Z_TYPE_P(result) != IS_STRING) {
			use_copy = zend_make_printable_zval(result, &result_copy);
			if (use_copy) {
				ZEPHIR_CPY_WRT_CTOR(result, &result_copy);
			}
		} 
	}

	if (self_var) {
		tmp_str = strpprintf(length + Z_STRLEN_P(result), "%s%s%s", Z_STRVAL_P(result), op1, Z_STRVAL_P(op2));
	} else {
		tmp_str = strpprintf(length, "%s%s", op1, Z_STRVAL_P(op2));
	}
	ZVAL_NEW_STR(result, tmp_str);
	zend_string_release(tmp_str);
	if (use_copy2) {
	   zval_dtor(op2);
	}

	if (use_copy) {
	   zval_dtor(&result_copy);
	}

}

void zephir_concat_svs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, int self_var){

	zval result_copy, op2_copy;
	int use_copy = 0, use_copy2 = 0;
	zend_string *tmp_str;
	uint offset = 0, length;

	if (Z_TYPE_P(op2) != IS_STRING) {
	   use_copy2 = zend_make_printable_zval(op2, &op2_copy);
	   if (use_copy2) {
	       op2 = &op2_copy;
	   }
	}

	length = op1_len + Z_STRLEN_P(op2) + op3_len;
	if (self_var) {
		if (Z_TYPE_P(result) != IS_STRING) {
			use_copy = zend_make_printable_zval(result, &result_copy);
			if (use_copy) {
				ZEPHIR_CPY_WRT_CTOR(result, &result_copy);
			}
		} 
	}

	if (self_var) {
		tmp_str = strpprintf(length + Z_STRLEN_P(result), "%s%s%s%s", Z_STRVAL_P(result), op1, Z_STRVAL_P(op2), op3);
	} else {
		tmp_str = strpprintf(length, "%s%s%s", op1, Z_STRVAL_P(op2), op3);
	}
	ZVAL_NEW_STR(result, tmp_str);
	zend_string_release(tmp_str);
	if (use_copy2) {
	   zval_dtor(op2);
	}

	if (use_copy) {
	   zval_dtor(&result_copy);
	}

}

void zephir_concat_vs(zval *result, zval *op1, const char *op2, uint32_t op2_len, int self_var){

	zval result_copy, op1_copy;
	int use_copy = 0, use_copy1 = 0;
	zend_string *tmp_str;
	uint offset = 0, length;

	if (Z_TYPE_P(op1) != IS_STRING) {
	   use_copy1 = zend_make_printable_zval(op1, &op1_copy);
	   if (use_copy1) {
	       op1 = &op1_copy;
	   }
	}

	length = Z_STRLEN_P(op1) + op2_len;
	if (self_var) {
		if (Z_TYPE_P(result) != IS_STRING) {
			use_copy = zend_make_printable_zval(result, &result_copy);
			if (use_copy) {
				ZEPHIR_CPY_WRT_CTOR(result, &result_copy);
			}
		} 
	}

	if (self_var) {
		tmp_str = strpprintf(length + Z_STRLEN_P(result), "%s%s%s", Z_STRVAL_P(result), Z_STRVAL_P(op1), op2);
	} else {
		tmp_str = strpprintf(length, "%s%s", Z_STRVAL_P(op1), op2);
	}
	ZVAL_NEW_STR(result, tmp_str);
	zend_string_release(tmp_str);
	if (use_copy1) {
	   zval_dtor(op1);
	}

	if (use_copy) {
	   zval_dtor(&result_copy);
	}

}

void zephir_concat_vv(zval *result, zval *op1, zval *op2, int self_var){

	zval result_copy, op1_copy, op2_copy;
	int use_copy = 0, use_copy1 = 0, use_copy2 = 0;
	zend_string *tmp_str;
	uint offset = 0, length;

	if (Z_TYPE_P(op1) != IS_STRING) {
	   use_copy1 = zend_make_printable_zval(op1, &op1_copy);
	   if (use_copy1) {
	       op1 = &op1_copy;
	   }
	}

	if (Z_TYPE_P(op2) != IS_STRING) {
	   use_copy2 = zend_make_printable_zval(op2, &op2_copy);
	   if (use_copy2) {
	       op2 = &op2_copy;
	   }
	}

	length = Z_STRLEN_P(op1) + Z_STRLEN_P(op2);
	if (self_var) {
		if (Z_TYPE_P(result) != IS_STRING) {
			use_copy = zend_make_printable_zval(result, &result_copy);
			if (use_copy) {
				ZEPHIR_CPY_WRT_CTOR(result, &result_copy);
			}
		} 
	}

	if (self_var) {
		tmp_str = strpprintf(length + Z_STRLEN_P(result), "%s%s%s", Z_STRVAL_P(result), Z_STRVAL_P(op1), Z_STRVAL_P(op2));
	} else {
		tmp_str = strpprintf(length, "%s%s", Z_STRVAL_P(op1), Z_STRVAL_P(op2));
	}
	ZVAL_NEW_STR(result, tmp_str);
	zend_string_release(tmp_str);
	if (use_copy1) {
	   zval_dtor(op1);
	}

	if (use_copy2) {
	   zval_dtor(op2);
	}

	if (use_copy) {
	   zval_dtor(&result_copy);
	}

}

void zephir_concat_vvv(zval *result, zval *op1, zval *op2, zval *op3, int self_var){

	zval result_copy, op1_copy, op2_copy, op3_copy;
	int use_copy = 0, use_copy1 = 0, use_copy2 = 0, use_copy3 = 0;
	zend_string *tmp_str;
	uint offset = 0, length;

	if (Z_TYPE_P(op1) != IS_STRING) {
	   use_copy1 = zend_make_printable_zval(op1, &op1_copy);
	   if (use_copy1) {
	       op1 = &op1_copy;
	   }
	}

	if (Z_TYPE_P(op2) != IS_STRING) {
	   use_copy2 = zend_make_printable_zval(op2, &op2_copy);
	   if (use_copy2) {
	       op2 = &op2_copy;
	   }
	}

	if (Z_TYPE_P(op3) != IS_STRING) {
	   use_copy3 = zend_make_printable_zval(op3, &op3_copy);
	   if (use_copy3) {
	       op3 = &op3_copy;
	   }
	}

	length = Z_STRLEN_P(op1) + Z_STRLEN_P(op2) + Z_STRLEN_P(op3);
	if (self_var) {
		if (Z_TYPE_P(result) != IS_STRING) {
			use_copy = zend_make_printable_zval(result, &result_copy);
			if (use_copy) {
				ZEPHIR_CPY_WRT_CTOR(result, &result_copy);
			}
		} 
	}

	if (self_var) {
		tmp_str = strpprintf(length + Z_STRLEN_P(result), "%s%s%s%s", Z_STRVAL_P(result), Z_STRVAL_P(op1), Z_STRVAL_P(op2), Z_STRVAL_P(op3));
	} else {
		tmp_str = strpprintf(length, "%s%s%s", Z_STRVAL_P(op1), Z_STRVAL_P(op2), Z_STRVAL_P(op3));
	}
	ZVAL_NEW_STR(result, tmp_str);
	zend_string_release(tmp_str);
	if (use_copy1) {
	   zval_dtor(op1);
	}

	if (use_copy2) {
	   zval_dtor(op2);
	}

	if (use_copy3) {
	   zval_dtor(op3);
	}

	if (use_copy) {
	   zval_dtor(&result_copy);
	}

}

void zephir_concat_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
    zval tmp;
    ZVAL_UNDEF(&tmp);
    
    /* Make sure that we do not corrupt data */
    if (result == op1 || result == op2) {
        concat_function(&tmp, op1, op2);
        ZVAL_COPY_VALUE(result, &tmp);
    } else {
        concat_function(result, op1, op2);
    }
}