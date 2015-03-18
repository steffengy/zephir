#ifndef ZEPHIR_KERNEL_OPERATORS_H
#define ZEPHIR_KERNEL_OPERATORS_H

/** Strict comparing */
#define ZEPHIR_IS_LONG(op1, op2)   ((Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) == op2) || zephir_compare_strict_long(op1, op2))
#define ZEPHIR_IS_DOUBLE(op1, op2) ((Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) == op2) || zephir_compare_strict_double(op1, op2))
#define ZEPHIR_IS_STRING(op1, op2) zephir_compare_strict_string(op1, op2, strlen(op2))

#define ZEPHIR_IS_LONG_IDENTICAL(op1, op2)   (Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) == op2)
#define ZEPHIR_IS_DOUBLE_IDENTICAL(op1, op2) (Z_TYPE_P(op1) == IS_DOUBLE && Z_DVAL_P(op1) == op2)
#define ZEPHIR_IS_STRING_IDENTICAL(op1, op2) (Z_TYPE_P(op1) == IS_STRING && zephir_compare_strict_string(op1, op2, strlen(op2)))

/** strict boolean comparison */
#define ZEPHIR_IS_FALSE(var)       ((Z_TYPE_P(var) == IS_FALSE) || zephir_compare_strict_bool(var, 0))
#define ZEPHIR_IS_TRUE(var)        ((Z_TYPE_P(var) == IS_TRUE) || zephir_compare_strict_bool(var, 1))
#define ZEPHIR_IS_FALSE_IDENTICAL(var)       (Z_TYPE_P(var) == IS_FALSE)
#define ZEPHIR_IS_TRUE_IDENTICAL(var)        (Z_TYPE_P(var) == IS_TRUE)

/** Equals/Identical */
#define ZEPHIR_IS_EQUAL(op1, op2)      zephir_is_equal(op1, op2)
#define ZEPHIR_IS_IDENTICAL(op1, op2)  zephir_is_identical(op1, op2)

/** SQL null empty **/
#define ZEPHIR_IS_EMPTY(var)       (Z_TYPE_P(var) == IS_NULL || ZEPHIR_IS_FALSE(var) || (Z_TYPE_P(var) == IS_STRING && !Z_STRLEN_P(var)) || !zend_is_true(var))
#define ZEPHIR_IS_NOT_EMPTY(var)   (!ZEPHIR_IS_EMPTY(var))

#define zephir_is_true(value) \
	(Z_TYPE_P(value) == IS_TRUE ? 1 : ( \
		Z_TYPE_P(value) == IS_FALSE ? 0 : ( \
			Z_TYPE_P(value) == IS_LONG ? Z_LVAL_P(value) : \
				zend_is_true(value) \
		) \
	))

/* Increment/Decrement */
#define zephir_increment(var) increment_function(var)
#define zephir_decrement(var) decrement_function(var)

/** greater/smaller comparision */
#define ZEPHIR_GE(op1, op2)       zephir_greater_equal(op1, op2)
#define ZEPHIR_GE_LONG(op1, op2)  zephir_greater_equal_long(op1, op2)
#define ZEPHIR_LE(op1, op2)       zephir_less_equal(op1, op2)
#define ZEPHIR_LE_LONG(op1, op2)  ((Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) <= op2) || zephir_less_equal_long(op1, op2))
#define ZEPHIR_LT(op1, op2)       ((Z_TYPE_P(op1) == IS_LONG && Z_TYPE_P(op2) == IS_LONG) ? Z_LVAL_P(op1) < Z_LVAL_P(op2) : zephir_less(op1, op2))
#define ZEPHIR_LT_LONG(op1, op2)  ((Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) < op2) || zephir_less_long(op1, op2))
#define ZEPHIR_GT(op1, op2)       zephir_greater(op1, op2)
#define ZEPHIR_GT_LONG(op1, op2)  ((Z_TYPE_P(op1) == IS_LONG && Z_LVAL_P(op1) > op2) || zephir_greater_long(op1, op2))

#define ZEPHIR_STRING_OFFSET(op1, index) ((index >= 0 && index < Z_STRLEN_P(op1)) ? Z_STRVAL_P(op1)[index] : '\0')

#define zephir_get_strval(left, right) \
	{ \
		int use_copy_right; \
		zval right_tmp; \
		if (Z_TYPE_P(right) == IS_STRING) { \
			ZVAL_COPY(left, right); \
		} else { \
			ZVAL_NULL(&right_tmp); \
			use_copy_right = zephir_make_printable_zval(right, &right_tmp); \
			if (use_copy_right) { \
				ZVAL_COPY_VALUE(left, &right_tmp); \
			} \
		} \
	}

#define zephir_get_arrval(returnValue, passValue) \
	{ \
		ZVAL_NULL(returnValue); \
		if (Z_TYPE_P(passValue) == IS_ARRAY) { \
			ZEPHIR_CPY_WRT(returnValue, passValue); \
		} else { \
			ZEPHIR_INIT_NVAR(returnValue); \
			array_init_size(returnValue, 0); \
		} \
	}

#define ZEPHIR_ADD_ASSIGN(z, v)  \
	{  \
		zval tmp;  \
		SEPARATE_ZVAL(z);  \
		add_function(&tmp, z, v TSRMLS_CC);  \
		if (Z_TYPE(tmp) == IS_LONG) {  \
			Z_LVAL_P(z) = Z_LVAL(tmp);  \
		} else {  \
			if (Z_TYPE(tmp) == IS_DOUBLE) {  \
				Z_DVAL_P(z) = Z_DVAL(tmp);  \
			}  \
		}  \
	}

#define ZEPHIR_SUB_ASSIGN(z, v)  \
	{  \
		zval tmp;  \
		SEPARATE_ZVAL(z);  \
		sub_function(&tmp, z, v);  \
		if (Z_TYPE(tmp) == IS_LONG) {  \
			Z_LVAL_P(z) = Z_LVAL(tmp);  \
		} else {  \
			if (Z_TYPE(tmp) == IS_DOUBLE) {  \
				Z_DVAL_P(z) = Z_DVAL(tmp);  \
			}  \
		}  \
	}

#define ZEPHIR_MUL_ASSIGN(z, v)  \
	{  \
		zval tmp;  \
		SEPARATE_ZVAL(z);  \
		mul_function(&tmp, z, v);  \
		if (Z_TYPE(tmp) == IS_LONG) {  \
			Z_LVAL_P(z) = Z_LVAL(tmp);  \
		} else {  \
			if (Z_TYPE(tmp) == IS_DOUBLE) {  \
				Z_DVAL_P(z) = Z_DVAL(tmp);  \
			}  \
		}  \
	}

/* zephir_make_printable_zval_ex: returns use_copy */
#define zephir_make_printable_zval(expr, expr_copy) zend_make_printable_zval(expr, expr_copy)

int zephir_is_numeric_ex(const zval *op);
int zephir_is_equal(zval *op1, zval *op2);
int zephir_is_identical(zval *op1, zval *op2);
#define zephir_is_numeric(value) (Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE || zephir_is_numeric_ex(value))

zend_bool zephir_get_boolval_ex(const zval *op_in);
double zephir_get_doubleval_ex(const zval *op);
long zephir_get_intval_ex(const zval *op);

/* Strict compare */
int zephir_compare_strict_bool(zval *op1, zend_bool op2);
int zephir_compare_strict_long(zval *op1, long op2);
int zephir_compare_strict_double(zval *op1, double op2);
int zephir_compare_strict_string(zval *op1, const char *op2, int op2_length);

/* greater/smaller */
int zephir_greater_long(zval *op1, long op2);
int zephir_greater_equal(zval *op1, zval *op2);
int zephir_greater_equal_long(zval *op1, long op2);
int zephir_less(zval *op1, zval *op2);
int zephir_less_long(zval *op1, long op2);
int zephir_less_equal_long(zval *op1, long op2);
int zephir_less_equal(zval *op1, zval *op2);
int zephir_greater(zval *op1, zval *op2);

double zephir_safe_div_long_long(long op1, long op2);
double zephir_safe_div_long_zval(long op1, zval *op2);
double zephir_safe_div_zval_long(zval *op1, long op2);
double zephir_safe_div_long_double(long op1, double op2);
double zephir_safe_div_double_long(double op1, long op2);
double zephir_safe_div_double_zval(double op1, zval *op2);
double zephir_safe_div_zval_double(zval *op1, double op2);

#define zephir_get_boolval(z) (Z_TYPE_P(z) == IS_TRUE ? 1 : (Z_TYPE_P(z) == IS_FALSE ? 0 : zephir_get_boolval_ex(z)))
#define zephir_get_doubleval(z) (Z_TYPE_P(z) == IS_DOUBLE ? Z_DVAL_P(z) : zephir_get_doubleval_ex(z))
#define zephir_get_numberval(z) (Z_TYPE_P(z) == IS_LONG ? Z_LVAL_P(z) : zephir_get_doubleval(z))
#define zephir_get_intval(z) (Z_TYPE_P(z) == IS_LONG ? Z_LVAL_P(z) : zephir_get_intval_ex(z))
#define zephir_add_function_ex(result, op1, op2) add_function(result, op1, op2)
#define zephir_add_function zephir_add_function_ex
#define zephir_convert_to_object(op) convert_to_object(op)

int zephir_bitwise_or_function(zval *result, zval *op1, zval *op2);
int zephir_bitwise_and_function(zval *result, zval *op1, zval *op2);

void zephir_negate(zval *z);

void zephir_concat_self_str(zval *left, const char *right, int right_length);
void zephir_concat_self_char(zval *left, char right);
void zephir_concat_self(zval *left, zval *right);

#endif
