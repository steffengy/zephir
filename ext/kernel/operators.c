
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
	is_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE;
}

/**
 * Check if two zvals are identical
 */
int zephir_is_identical(zval *op1, zval *op2)
{
	zval result;
	is_identical_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE;
}

/**
 * Returns the long value of a zval
 */
int zephir_is_numeric_ex(const zval *op)
{
	int type;

	switch (Z_TYPE_P(op)) {
		case IS_LONG:
			return 1;

		case IS_TRUE:
		case IS_FALSE:
			return 0;

		case IS_DOUBLE:
			return 1;

		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), NULL, NULL, 0))) {
				if (type == IS_LONG || type == IS_DOUBLE) {
					return 1;
				}
			}
	}

	return 0;
}

/**
 * Natural compare with bool operandus on right
 */
int zephir_compare_strict_bool(zval *op1, zend_bool op2)
{
	int bool_result;

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return (Z_LVAL_P(op1) ? 1 : 0) == op2;
		case IS_DOUBLE:
			return (Z_DVAL_P(op1) ? 1 : 0) == op2;
		case IS_NULL:
			return 0 == op2;
		case IS_TRUE:
			return 1 == op2;
		case IS_FALSE:
			return 0 == op2;
		default:
			{
				zval result, op2_tmp;
				ZVAL_BOOL(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp);
				bool_result = Z_TYPE(result) == IS_TRUE;
				return bool_result;
			}
	}

	return 0;
}

/**
 * Natural compare with long operandus on right
 */
int zephir_compare_strict_long(zval *op1, long op2) {

	int bool_result;

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return Z_LVAL_P(op1) == op2;
		case IS_DOUBLE:
			return Z_DVAL_P(op1) == (double) op2;
		case IS_FALSE:
		case IS_NULL:
			return 0 == op2;
		case IS_TRUE:
			return 1 == op2;
		default:
			{
				zval result, op2_tmp;
				ZVAL_LONG(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp TSRMLS_CC);
				bool_result = Z_TYPE(result) == IS_TRUE;
				return bool_result;
			}
	}

	return 0;
}

/**
 * Natural compare with double operandus on right
 */
int zephir_compare_strict_double(zval *op1, double op2) {

	int bool_result;

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return Z_LVAL_P(op1) == (long) op2;
		case IS_DOUBLE:
			return Z_DVAL_P(op1) == op2;
		case IS_FALSE:
		case IS_NULL:
			return 0 == op2;
		case IS_TRUE:
			return 1 == op2;
		default:
			{
				zval result, op2_tmp;
				ZVAL_DOUBLE(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp);
				bool_result = Z_TYPE(result) == IS_TRUE;
				return bool_result;
			}
	}

	return 0;
}

/**
 * Natural compare with string operandus on right
 */
int zephir_compare_strict_string(zval *op1, const char *op2, int op2_length)
{
	switch (Z_TYPE_P(op1)) {

		case IS_STRING:
			if (!Z_STRLEN_P(op1) && !op2_length) {
				return 1;
			}
			if (Z_STRLEN_P(op1) != op2_length) {
				return 0;
			}
			return !zend_binary_strcmp(Z_STRVAL_P(op1), Z_STRLEN_P(op1), op2, op2_length);

		case IS_NULL:
			return !zend_binary_strcmp("", 0, op2, op2_length);

		case IS_TRUE:
			return !zend_binary_strcmp("1", strlen("1"), op2, op2_length);
		case IS_FALSE:
			return !zend_binary_strcmp("0", strlen("0"), op2, op2_length);
		default:
			zend_error(E_WARNING, "Invalid type: zephir_compare_strict_string");
	}

	return 0;
}

/**
 * Check if a zval is greater than a long value
 */
int zephir_greater_long(zval *op1, long op2) {
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_or_equal_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_FALSE;
}

/**
 * Check if a zval is greater/equal than other
 */
int zephir_greater_equal(zval *op1, zval *op2) {
	zval result;
	is_smaller_function(&result, op1, op2);
	return Z_TYPE(result) == IS_FALSE;
}

/**
 * Check for greater/equal
 */
int zephir_greater_equal_long(zval *op1, long op2) {
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_FALSE;
}

/**
 * Check if a zval is less than other
 */
int zephir_less(zval *op1, zval *op2) {
	zval result;
	is_smaller_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE;
}

int zephir_less_equal(zval *op1, zval *op2) {
	zval result;
	is_smaller_or_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_TRUE;
}

/**
 * Check if a zval is greater than other
 */
int zephir_greater(zval *op1, zval *op2) {
	zval result;
	is_smaller_or_equal_function(&result, op1, op2);
	return Z_TYPE(result) == IS_FALSE;
}

/**
 * Returns the long value of a zval
 */
zend_bool zephir_get_boolval_ex(const zval *op_in) {

	int type;
	long long_value = 0;
	double double_value = 0;

	/* Make sure we read the reference value */
	zval *op;
	if (Z_REFCOUNTED_P(op_in) && Z_ISREF_P(op_in)) op = Z_REFVAL_P(op_in);
	else op = op_in;

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? (zend_bool) 1 : 0;
            break;
	    case IS_CALLABLE:
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return (zend_bool) 1;
		case IS_LONG:
			return (Z_LVAL_P(op) ? (zend_bool) 1 : 0);
		case IS_TRUE:
			return 1;
		case IS_FALSE:
			return 0;
		case IS_DOUBLE:
			return (Z_DVAL_P(op) ? (zend_bool) 1 : 0);
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0))) {
				if (type == IS_LONG) {
					return (long_value ? (zend_bool) 1 : 0);
				} else {
					if (type == IS_DOUBLE) {
						return (double_value ? (zend_bool) 1 : 0);
					} else {
						return 0;
					}
				}
			}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
double zephir_get_doubleval_ex(const zval *op) {

	int type;
	long long_value = 0;
	double double_value = 0;

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? (double) 1 : 0;
            break;
	    case IS_CALLABLE:
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return (double) 1;
		case IS_LONG:
			return (double) Z_LVAL_P(op);
		case IS_TRUE:
			return 1;
		case IS_FALSE:
			return 0;
		case IS_DOUBLE:
			return Z_DVAL_P(op);
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0))) {
				if (type == IS_LONG) {
					return (double) long_value;
				} else {
					if (type == IS_DOUBLE) {
						return double_value;
					} else {
						return 0;
					}
				}
			}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
long zephir_get_intval_ex(const zval *op) {

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? 1 : 0;
            break;

	    case IS_CALLABLE:
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return 1;

		case IS_LONG:
			return Z_LVAL_P(op);

		case IS_TRUE:
			return 1;

		case IS_FALSE:
			return 0;

		case IS_DOUBLE:
			return (long) Z_DVAL_P(op);

		case IS_STRING: {
			long long_value = 0;
			double double_value = 0;
			zend_uchar type;

			ASSUME(Z_STRVAL_P(op) != NULL);
			type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0);
			if (type == IS_LONG) {
				return long_value;
			}
			if (type == IS_DOUBLE) {
				return (long) double_value;
			}
			return 0;
		}
	}

	return 0;
}

/**
 * Do safe divisions between two long/zval
 */
double zephir_safe_div_long_zval(long op1, zval *op2)
{
	if (!zephir_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return (double) op1 / ((double) zephir_get_numberval(op2));
}

/**
 * Do safe divisions between two zval/long
 */
double zephir_safe_div_zval_long(zval *op1, long op2)
{
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((double) zephir_get_numberval(op1)) / (double) op2;
}

/**
 * Do safe divisions between two long/double
 */
double zephir_safe_div_long_double(long op1, double op2)
{
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (double) op1 / op2;
}

/**
 * Do safe divisions between two double/long
 */
double zephir_safe_div_double_long(double op1, long op2)
{
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 / (double) op2;
}

/**
 * Do safe divisions between two longs
 */
double zephir_safe_div_long_long(long op1, long op2)
{
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (double) op1 / (double) op2;
}

void zephir_negate(zval *z)
{
	while (1) {
		switch (Z_TYPE_P(z)) {
			case IS_LONG:
				ZVAL_LONG(z, -Z_LVAL_P(z));
				return;

			case IS_TRUE:
				ZVAL_LONG(z, -1);
				return;

			case IS_FALSE:
				ZVAL_LONG(z, 0);
				return;

			case IS_DOUBLE:
				ZVAL_DOUBLE(z, -Z_DVAL_P(z));
				return;

			case IS_NULL:
				ZVAL_LONG(z, 0);
				return;

			default:
				convert_scalar_to_number(z);
				assert(Z_TYPE_P(z) == IS_LONG || Z_TYPE_P(z) == IS_DOUBLE);
		}
	}
}

/**
 * Check if a zval is less than a long value
 */
int zephir_less_long(zval *op1, long op2)
{
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_TRUE;
}

int zephir_less_equal_long(zval *op1, long op2)
{
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_or_equal_function(&result, op1, &op2_zval);
	return Z_TYPE(result) == IS_TRUE;
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self_str(zval *left, const char *right, int right_length)
{
	zval left_copy;
	zend_string *tmp_str;
	int use_copy = 0;

	/* If the left operator is not initialized, just build a new string */
	if (Z_TYPE_P(left) == IS_NULL || Z_TYPE_P(left) == IS_UNDEF) {
		tmp_str = zend_string_init(right, right_length, 0);
		ZVAL_STR(left, tmp_str);
		zend_string_release(tmp_str);
		return;
	}

	/* Convert to string, if other type */
	if (Z_TYPE_P(left) != IS_STRING) {
		use_copy = zend_make_printable_zval(left, &left_copy);
		if (use_copy) {
			ZEPHIR_CPY_WRT_CTOR(left, &left_copy);
		}
	}

	/*SEPARATE_ZVAL_IF_NOT_REF(left);*/

	tmp_str = strpprintf(Z_STRLEN_P(left) + right_length, "%s%s", Z_STRVAL_P(left), right);
	zval_ptr_dtor(left);
	ZVAL_STR(left, tmp_str);

	if (use_copy) {
		zval_dtor(&left_copy);
	}
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self_char(zval *left, char right)
{
	char tmp[2] = { right };
	tmp[1] = '\0';
	zephir_concat_self_str(left, tmp, 2);
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self(zval *left, zval *right)
{
	int use_copy = 0;
	zval right_copy;

	/* Convert to string, if other type */
	if (Z_TYPE_P(right) != IS_STRING) {
		use_copy = zend_make_printable_zval(right, &right_copy);
		if (use_copy) {
			ZEPHIR_CPY_WRT_CTOR(right, &right_copy);
		}
	}

	zephir_concat_self_str(left, Z_STRVAL_P(right), Z_STRLEN_P(right));
	if (use_copy) {
		zval_dtor(&right_copy);
	}
}

/**
 * Do safe divisions between two double/zval
 */
double zephir_safe_div_double_zval(double op1, zval *op2)
{
	if (!zephir_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return op1 / ((double) zephir_get_numberval(op2));
}

/**
 * Do safe divisions between two zval/double
 */
double zephir_safe_div_zval_double(zval *op1, double op2)
{
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((double) zephir_get_numberval(op1)) / op2;
}

/**
 * Do bitwise_and function keeping ref_count and is_ref
 */
int zephir_bitwise_and_function(zval *result, zval *op1, zval *op2)
{
	int status;
	//int ref_count = Z_REFCOUNT_P(result);
	//int is_ref = Z_ISREF_P(result);
	status = bitwise_and_function(result, op1, op2);
	//Z_SET_REFCOUNT_P(result, ref_count);
	//Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

/**
 * Do bitwise_or function keeping ref_count and is_ref
 */
int zephir_bitwise_or_function(zval *result, zval *op1, zval *op2)
{
	int status;
	//int ref_count = Z_REFCOUNT_P(result);
	//int is_ref = Z_ISREF_P(result);
	status = bitwise_or_function(result, op1, op2);
	//Z_SET_REFCOUNT_P(result, ref_count);
	//Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}
