#ifndef ZEPHIR_KERNEL_FCALL_H
#define ZEPHIR_KERNEL_FCALL_H

#include "php_ext.h"
#include "kernel/main.h"
#include "kernel/memory.h"
#include <Zend/zend_hash.h>
#include <Zend/zend.h>

typedef enum _zephir_call_type {
	zephir_fcall_parent,
	zephir_fcall_self,
	zephir_fcall_static,
	zephir_fcall_ce,
	zephir_fcall_method,
	zephir_fcall_function
} zephir_call_type;

typedef zend_function zephir_fcall_cache_entry;

#define ZEPHIR_ZEND_CALL_FUNCTION_WRAPPER zend_call_function
#define zephir_eval_php(str, retval_ptr, context) zend_eval_string_ex(Z_STRVAL_P(str), retval_ptr, context, 0)

#ifdef _MSC_VER
	#define ZEPHIR_PASS_CALL_PARAMS(x) x + 1
	#define ZEPHIR_CALL_NUM_PARAMS(x) ((sizeof(x) - sizeof(x[0]))/sizeof(x[0]))
	#define ZEPHIR_FETCH_VA_ARGS NULL,

#else
	#define ZEPHIR_PASS_CALL_PARAMS(x) x
	#define ZEPHIR_CALL_NUM_PARAMS(x) sizeof(x)/sizeof(zval*)
	#define ZEPHIR_FETCH_VA_ARGS
#endif

#define zephir_return_call_function     zephir_call_func_aparams
#define zephir_return_call_class_method zephir_call_class_method_aparams

/* Function call default */
#define ZEPHIR_CALL_FUNCTION(return_value_ptr, func_name, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(func_name)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_func_aparams(return_value_ptr, func_name, sizeof(func_name)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_func_aparams(return_value_ptr, func_name, strlen(func_name), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

/* Method call */
#define ZEPHIR_CALL_METHOD(return_value_ptr, object, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, Z_TYPE_P(object) == IS_OBJECT ? Z_OBJCE_P(object) : NULL, zephir_fcall_method, object, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, Z_TYPE_P(object) == IS_OBJECT ? Z_OBJCE_P(object) : NULL, zephir_fcall_method, object, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_CALL_SELF(return_value_ptr, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, NULL, zephir_fcall_self, NULL, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, NULL, zephir_fcall_self, NULL, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_CALL_STATIC(return_value_ptr, method, cache, ...) \
	do { \
		zval *params_[] = {ZEPHIR_FETCH_VA_ARGS __VA_ARGS__}; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, NULL, zephir_fcall_static, NULL, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, NULL, zephir_fcall_static, NULL, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_CALL_CE_STATIC(return_value_ptr, class_entry, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, class_entry, zephir_fcall_ce, NULL, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, class_entry, zephir_fcall_ce, NULL, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_FUNCTION(func_name, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(func_name)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_function(return_value, func_name, sizeof(func_name)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_function(return_value, func_name, strlen(func_name), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_METHOD(object, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, Z_TYPE_P(object) == IS_OBJECT ? Z_OBJCE_P(object) : NULL, zephir_fcall_method, object, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, Z_TYPE_P(object) == IS_OBJECT ? Z_OBJCE_P(object) : NULL, zephir_fcall_method, object, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_STATIC(method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, NULL, zephir_fcall_static, NULL, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, NULL, zephir_fcall_static, NULL, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_CE_STATIC(class_entry, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, class_entry, zephir_fcall_ce, NULL, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, class_entry, zephir_fcall_ce, NULL, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_SELF(method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, NULL, zephir_fcall_self, NULL, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, NULL, zephir_fcall_self, NULL, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_CALL_PARENT(return_value_ptr, class_entry, this_ptr, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, class_entry, zephir_fcall_parent, this_ptr, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_class_method_aparams(return_value_ptr, class_entry, zephir_fcall_parent, this_ptr, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_PARENT(class_entry, this_ptr, method, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(method)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, class_entry, zephir_fcall_parent, this_ptr, method, sizeof(method)-1, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_return_call_class_method(return_value, class_entry, zephir_fcall_parent, this_ptr, method, strlen(method), cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_RETURN_CALL_ZVAL_FUNCTION(func_name, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		if (__builtin_constant_p(func_name)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_zval_func_aparams(return_value, func_name, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_zval_func_aparams(return_value, func_name, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

#define ZEPHIR_CALL_ZVAL_FUNCTION(return_value_ptr, func_name, cache, ...) \
	do { \
		zval *params_[] = { ZEPHIR_FETCH_VA_ARGS __VA_ARGS__ }; \
		ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(return_value_ptr); \
		if (__builtin_constant_p(func_name)) { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_zval_func_aparams(return_value_ptr, func_name, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
		else { \
			ZEPHIR_LAST_CALL_STATUS = zephir_call_zval_func_aparams(return_value_ptr, func_name, cache, ZEPHIR_CALL_NUM_PARAMS(params_), ZEPHIR_PASS_CALL_PARAMS(params_)); \
		} \
	} while (0)

/** Use these functions to call functions in the PHP userland using an arbitrary zval as callable */
#define ZEPHIR_CALL_USER_FUNC(return_value, handler) ZEPHIR_CALL_USER_FUNC_ARRAY(return_value, handler, NULL)
#define ZEPHIR_CALL_USER_FUNC_ARRAY(return_value, handler, params) \
	do { \
		ZEPHIR_LAST_CALL_STATUS = zephir_call_user_func_array(return_value, handler, params); \
	} while (0)

/** Fast call_user_func_array/call_user_func */
int zephir_call_user_func_array_noex(zval *return_value, zval *handler, zval *params) ZEPHIR_ATTR_WARN_UNUSED_RESULT;

/**
 * Replaces call_user_func_array avoiding function lookup
 */
ZEPHIR_ATTR_WARN_UNUSED_RESULT static inline int zephir_call_user_func_array(zval *return_value, zval *handler, zval *params TSRMLS_DC)
{
	int status = zephir_call_user_func_array_noex(return_value, handler, params TSRMLS_CC);
	return (EG(exception)) ? FAILURE : status;
}

#define zephir_check_call_status() \
	do \
		if (ZEPHIR_LAST_CALL_STATUS == FAILURE) { \
			ZEPHIR_MM_RESTORE(); \
			return; \
	} \
	while(0)

#ifdef ZEPHIR_RELEASE
	#define ZEPHIR_TEMP_PARAM_COPY 0
	#define zephir_check_temp_parameter(param) do { if (Z_REFCOUNTED_P(param) && Z_REFCOUNT_P(param) > 1) zval_copy_ctor(param); else ZVAL_NULL(param); } while(0)
#else
	#define ZEPHIR_TEMP_PARAM_COPY 1
	#define zephir_check_temp_parameter(param)
#endif

#define zephir_check_call_status_or_jump(label) \
	if (ZEPHIR_LAST_CALL_STATUS == FAILURE) { \
		if (EG(exception)) { \
			goto label; \
		} else { \
			ZEPHIR_MM_RESTORE(); \
			return; \
		} \
	}

#define RETURN_ON_FAILURE(what) \
	do { \
		if (what == FAILURE) { \
			return; \
		} \
	} while (0)

/**
 * @brief Checks if the class defines a constructor
 * @param ce Class entry
 * @return Whether the class defines a constructor
 */
int zephir_has_constructor_ce(const zend_class_entry *ce) ZEPHIR_ATTR_PURE ZEPHIR_ATTR_NONNULL;

/**
 * @brief Checks if an object has a constructor
 * @param object Object to check
 * @return Whether @a object has a constructor
 * @retval 0 @a object is not an object or does not have a constructor
 * @retval 1 @a object has a constructor
 */
ZEPHIR_ATTR_WARN_UNUSED_RESULT ZEPHIR_ATTR_NONNULL static inline int zephir_has_constructor(const zval *object)
{
	return Z_TYPE_P(object) == IS_OBJECT ? zephir_has_constructor_ce(Z_OBJCE_P(object)) : 0;
}

int zephir_call_class_method_aparams(zval *return_value_ptr, zend_class_entry *ce, zephir_call_type type, zval *object,
	const char *method_name, uint32_t method_len,
	zephir_fcall_cache_entry **cache_entry,
	uint32_t param_count, zval *params[]) ZEPHIR_ATTR_WARN_UNUSED_RESULT;

int zephir_call_func_aparams(zval *return_value_ptr, const char *func_name, uint32_t func_length,
	zephir_fcall_cache_entry **cache_entry,
	uint32_t param_count, zval *params[]);

int zephir_call_zval_func_aparams(zval *return_value_ptr, zval *func_name,
	zephir_fcall_cache_entry **cache_entry,
	uint32_t param_count, zval *params[]);

#endif
