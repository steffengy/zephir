#ifndef ZEPHIR_KERNEL_OPERATORS_H
#define ZEPHIR_KERNEL_OPERATORS_H

/** strict boolean comparison */
#define ZEPHIR_IS_FALSE(var)       (Z_TYPE_P(var) == IS_FALSE) || zephir_compare_strict_bool(var, 0))
#define ZEPHIR_IS_TRUE(var)        (Z_TYPE_P(var) == IS_TRUE) || zephir_compare_strict_bool(var, 1))

#define zephir_get_strval(left, right) \
	{ \
		int use_copy_right; \
		zval right_tmp; \
		if (Z_TYPE_P(right) == IS_STRING) { \
			ZVAL_DUP(left, right); \
		} else { \
			ZVAL_NULL(&right_tmp); \
			zephir_make_printable_zval(right, &right_tmp, &use_copy_right); \
			if (use_copy_right) { \
				ZVAL_COPY_VALUE(left, &right_tmp); \
			} \
		} \
	}

/* TODO: zephir_make_printable_zval: maybe use copy is neede */
#define zephir_make_printable_zval(expr, expr_copy, use_copy) zend_make_printable_zval(expr, expr_copy)

int zephir_is_equal(zval *op1, zval *op2);

#endif
