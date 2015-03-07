#include <php.h>
#include "php_ext.h"

#include <Zend/zend_API.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_execute.h>

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/memory.h"
//#include "kernel/hash.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
//#include "kernel/backtrace.h"

#if PHP_VERSION_ID >= 50500
static const unsigned char tolower_map[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};
#endif

int zephir_has_constructor_ce(const zend_class_entry *ce)
{
	while (ce) {
		if (ce->constructor) {
			return 1;
		}
		ce = ce->parent;
	}
	return 0;
}

ZEPHIR_ATTR_NONNULL static void zephir_fcall_populate_fci_cache(zend_fcall_info_cache *fcic, zend_fcall_info *fci, zephir_call_type type)
{
	switch (type) {
		case zephir_fcall_parent:
			if (EG(scope) && EG(scope)->parent) {
				fcic->calling_scope = EG(scope)->parent;
				fcic->called_scope  = EG(current_execute_data)->called_scope;
				fcic->object    = fci->object ? fci->object : Z_OBJ(EG(current_execute_data)->This);
				fcic->initialized   = 1;
			}

			break;

		case zephir_fcall_self:
			if (EG(scope)) {
				fcic->calling_scope = EG(scope);
				fcic->called_scope  = EG(current_execute_data)->called_scope;
				fcic->object        = fci->object ? fci->object : Z_OBJ(EG(current_execute_data)->This);
				fcic->initialized   = 1;
			}

			break;

		case zephir_fcall_static:
			if (EG(current_execute_data)->called_scope) {
				fcic->calling_scope = EG(current_execute_data)->called_scope;
				fcic->called_scope  = EG(current_execute_data)->called_scope;
				fcic->object        = fci->object ? fci->object : Z_OBJ(EG(current_execute_data)->This);
				fcic->initialized   = 1;
			}

			break;

		case zephir_fcall_function:
			fcic->calling_scope = NULL;
			fcic->called_scope  = NULL;
			fcic->object        = NULL;
			fcic->initialized   = 1;
			break;

		case zephir_fcall_ce: {
			zend_class_entry *scope = EG(current_execute_data)->func ? EG(current_execute_data)->func->op_array.scope : NULL;

			fcic->initialized      = 1;
			fcic->calling_scope    = EG(scope);
			fcic->object           = NULL;

			if (scope && &(EG(current_execute_data)->This) &&
					instanceof_function(Z_OBJCE(EG(current_execute_data)->This), scope) &&
					instanceof_function(scope, fcic->calling_scope))
		    {
				fcic->object   = Z_OBJ(EG(current_execute_data)->This);
				fcic->called_scope = fcic->object->ce;
			}
			else {
				fcic->called_scope = fcic->calling_scope;
			}

			break;
		}

		case zephir_fcall_method:
			fcic->initialized      = 1;
			fcic->calling_scope    = EG(scope);
			fcic->object           = fci->object;
			if (fci->object) {
				fcic->called_scope = fci->object->ce;
			}
			else if (EG(scope) && !(EG(current_execute_data)->called_scope && instanceof_function(EG(current_execute_data)->called_scope, EG(scope)))) {
				fcic->called_scope = EG(scope);
			}
			else {
				fcic->called_scope = EG(current_execute_data)->called_scope;
			}

			break;

		default:
#ifndef ZEPHIR_RELEASE
			fprintf(stderr, "%s: unknown call type (%d)\n", __func__, (int)type);
			abort();
#endif
			fcic->initialized = 0; /* not strictly necessary but just to be safe */
			break;
	}

}

/**
 * Creates a unique key to cache the current method/function call address for the current scope
 */
static ulong zephir_make_fcall_key(char **result, size_t *length, const zend_class_entry *obj_ce, zephir_call_type type, zval *function_name)
{
	const zend_class_entry *calling_scope = EG(scope);
	char *buf = NULL, *c;
	size_t l = 0, len = 0;
	const size_t ppzce_size = sizeof(zend_class_entry**);
	ulong hash = 5381;

	*result = NULL;
	*length = 0;

	if (calling_scope && type == zephir_fcall_parent) {
		calling_scope = calling_scope->parent;
		if (UNEXPECTED(!calling_scope)) {
			return 0;
		}
	}
	else if (type == zephir_fcall_static) {
		calling_scope = EG(current_execute_data)->called_scope;
		if (UNEXPECTED(!calling_scope)) {
			return 0;
		}
	}

	if (
		    calling_scope
		 && obj_ce
		 && calling_scope != obj_ce
		 && !instanceof_function(obj_ce, calling_scope TSRMLS_CC)
		 && !instanceof_function(calling_scope, obj_ce TSRMLS_CC)
	) {
		calling_scope = NULL;
	}

	if (Z_TYPE_P(function_name) == IS_STRING) {
		l   = (size_t)(Z_STRLEN_P(function_name)) + 1;
		c   = Z_STRVAL_P(function_name);
		len = 2 * ppzce_size + l;
		buf = ecalloc(1, len);

		memcpy(buf,                  c,               l);
		memcpy(buf + l,              &calling_scope,  ppzce_size);
		memcpy(buf + l + ppzce_size, &obj_ce,         ppzce_size);
	}
	else if (Z_TYPE_P(function_name) == IS_ARRAY) {
		zval *method;
		HashTable *function_hash = Z_ARRVAL_P(function_name);
		if (
			    function_hash->nNumOfElements == 2
			 && ((method = zend_hash_index_find(function_hash, 1)) != NULL)
			 && Z_TYPE_P(method) == IS_STRING
		) {
			l   = (size_t)(Z_STRLEN_P(method)) + 1;
			c   = Z_STRVAL_P(method);
			len = 2 * ppzce_size + l;
			buf = ecalloc(1, len);

			memcpy(buf,                  c,               l);
			memcpy(buf + l,              &calling_scope,  ppzce_size);
			memcpy(buf + l + ppzce_size, &obj_ce,         ppzce_size);
		}
	}
	else if (Z_TYPE_P(function_name) == IS_OBJECT) {
		/*if (Z_OBJ_HANDLER_P(function_name, get_closure)) {
			l   = sizeof("__invoke");
			len = 2 * ppzce_size + l;
			buf = ecalloc(1, len);

			memcpy(buf,                  "__invoke",     l);
			memcpy(buf + l,              &calling_scope, ppzce_size);
			memcpy(buf + l + ppzce_size, &obj_ce,        ppzce_size);
		}*/
	}

	if (EXPECTED(buf != NULL)) {
		size_t i;

		for (i = 0; i < l; ++i) {
			char c = buf[i];
			c = tolower_map[(unsigned char)c];
			buf[i] = c;
			hash   = (hash << 5) + hash + c;
		}

		for (i = l; i < len; ++i) {
			char c = buf[i];
			hash = (hash << 5) + hash + c;
		}
	}

	*result = buf;
	*length = len;
	return hash;
}

/**
 * Calls a function/method in the PHP userland
 */
int zephir_call_user_function(zval *object_p, zend_class_entry *obj_ce, zephir_call_type type,
	zval *function_name, zval *retval_ptr, zephir_fcall_cache_entry **cache_entry, uint32_t param_count,
	zval *params[])
{
	zval local_retval_ptr;
	int status, arg;
	zval *param_arr;
	zend_fcall_info fci;
	zend_fcall_info_cache fcic;
	zend_zephir_globals_def *zephir_globals_ptr = ZEPHIR_VGLOBAL;
	char *fcall_key = NULL;
	size_t fcall_key_len;
	zephir_fcall_cache_entry *temp_cache_entry = NULL;
	zend_class_entry *old_scope = EG(scope);
	ulong fcall_key_hash;
	zend_bool call_constructor = 0;

	assert(obj_ce || !object_p);

	if (retval_ptr) {
		zval_ptr_dtor(retval_ptr);
		ZVAL_UNDEF(retval_ptr);
	} else {
		ZVAL_UNDEF(&local_retval_ptr);
	}

	++zephir_globals_ptr->recursive_lock;

	if (UNEXPECTED(zephir_globals_ptr->recursive_lock > 2048)) {
		zend_error(E_ERROR, "Maximum recursion depth exceeded");
		return FAILURE;
	}

	call_constructor = Z_TYPE_P(function_name) == IS_UNDEF;
	if (type != zephir_fcall_function && !object_p) {
		object_p = &(EG(current_execute_data)->This);
		if (!obj_ce && object_p) {
			obj_ce = Z_OBJCE_P(object_p);
		}
	}

	if (obj_ce) {
		EG(scope) = obj_ce;
	}

	if ((!cache_entry || !*cache_entry)) {
		if (zephir_globals_ptr->cache_enabled) {
			fcall_key_hash = zephir_make_fcall_key(&fcall_key, &fcall_key_len, (object_p && type != zephir_fcall_ce ? Z_OBJCE_P(object_p) : obj_ce), type, function_name);
		}
	}

	fci.size           = sizeof(fci);
	fci.function_table = obj_ce ? &obj_ce->function_table : EG(function_table);
	fci.object     = object_p ? Z_OBJ_P(object_p) : NULL;
	fci.function_name  = *function_name;
	fci.retval = retval_ptr ? retval_ptr : &local_retval_ptr;
	fci.param_count    = 0;
	fci.params         = NULL;
	fci.no_separation  = 1;
	fci.symbol_table   = NULL;

	fcic.initialized = 0;
	fcic.function_handler = NULL;
	if (!cache_entry || !*cache_entry) {
		if (fcall_key && (temp_cache_entry = zend_hash_str_find_ptr(zephir_globals_ptr->fcache, fcall_key, fcall_key_len)) != NULL) {
			zephir_fcall_populate_fci_cache(&fcic, &fci, type);
			fcic.function_handler = temp_cache_entry;
		}
	} else {
		zephir_fcall_populate_fci_cache(&fcic, &fci, type);
		fcic.function_handler = *cache_entry;
	}

	if (call_constructor) {
		if (fcic.initialized == 0) {
			zephir_fcall_populate_fci_cache(&fcic, &fci, type);
		}
		assert(fcic.initialized);
		/* We call the CE constructor */
		fcic.function_handler = type == zephir_fcall_parent ? fcic.calling_scope->constructor : fcic.called_scope->constructor;
		if (!fcic.function_handler) {
			zend_error(E_ERROR, "Trying to call constructor, when none exists! Maybe corrupt functionname zval!");
		}
		assert(object_p);
	} else if (!cache_entry || !*cache_entry) {
		zend_string *callable_name;
		char *error = NULL;

		if (!zend_is_callable_ex(&fci.function_name, fci.object, IS_CALLABLE_CHECK_SILENT, &callable_name, &fcic, &error)) {
			if (error) {
				zend_error(E_WARNING, "Invalid callback %s, %s", callable_name->val, error);
				efree(error);
			}
			if (callable_name) {
				zend_string_release(callable_name);
			}
			return FAILURE;
		} else if (error) {
			zend_error(E_STRICT, "%s", error);
			efree(error);
		}
		zend_string_release(callable_name);
	}

	if (param_count) {
		param_arr = emalloc(param_count * sizeof(zval));
		for (arg = 0; arg < param_count; ++arg) {
			if (ARG_SHOULD_BE_SENT_BY_REF(fcic.function_handler, arg + 1)) {
				ZVAL_NEW_REF(param_arr + arg, params[arg]);
				ZVAL_REF(params[arg], Z_REF_P(param_arr + arg));
				if (Z_REFCOUNTED_P(params[arg])) {
					Z_ADDREF_P(params[arg]);
				}
			} else if(params[arg] != NULL) {
				ZVAL_COPY_VALUE(param_arr + arg, params[arg]);
			}
		}
		fci.params = param_arr;
		fci.param_count = param_count;
	}

	status = ZEPHIR_ZEND_CALL_FUNCTION_WRAPPER(&fci, &fcic);
	if (param_count) efree(param_arr);

	EG(scope) = old_scope;

	if ((!cache_entry || !*cache_entry)) {
		if (EXPECTED(status != FAILURE) && fcall_key && !temp_cache_entry && fcic.initialized) {
			zephir_fcall_cache_entry *temp_cache_entry = fcic.function_handler;
			if (NULL == zend_hash_str_add_ptr(zephir_globals_ptr->fcache, fcall_key, fcall_key_len, temp_cache_entry)) {
#ifndef ZEPHIR_RELEASE
				free(temp_cache_entry);
#endif
			} else {
#ifdef ZEPHIR_RELEASE
				if (cache_entry) {
					*cache_entry = temp_cache_entry;
				}
#endif
			}
		}
	}

	if (fcall_key) {
		efree(fcall_key);
	}

	if (!retval_ptr) {
		if (retval_ptr != NULL) {
			zval_ptr_dtor(retval_ptr);
			ZVAL_UNDEF(retval_ptr);
		}
		zval_ptr_dtor(&local_retval_ptr);
		ZVAL_UNDEF(&local_retval_ptr);
	}

	--zephir_globals_ptr->recursive_lock;
	return status;
}


static char *zephir_fcall_possible_method(zend_class_entry *ce, const char *wrong_name)
{
	HashTable *methods;
	zend_function *method;
	char *possible_method = NULL;
	zval *left = NULL, *right = NULL, method_name;
	zval *params[1];
	int count;

	count = zend_hash_num_elements(&ce->function_table);
	if (count > 0) {

		ZVAL_STRING(&method_name, wrong_name);
		//efree(wrong_name);

		params[0] = &method_name;
		// TODO zephir_call_func_aparams(&right, SL("metaphone"), NULL, 1, params);

		methods = &ce->function_table;

		ZEND_HASH_FOREACH_PTR(methods, method) {
			//TODO: ZVAL_STRING(&method_name, method->common.function_name);
			efree(method->common.function_name);

			if (left) {
				zval_ptr_dtor(left);
			}
			ZVAL_UNDEF(left);
			// left = NULL;

			params[0] = &method_name;
			// TODO zephir_call_func_aparams(&left, SL("metaphone"), NULL, 1, params TSRMLS_CC);

			if (zephir_is_equal(left, right)) {
				possible_method = (char *) method->common.function_name;
				break;
			}
		} ZEND_HASH_FOREACH_END();

		/*
		zend_hash_internal_pointer_reset_ex(methods, &pos);
		while (zend_hash_get_current_data_ex(methods, (void **) &method, &pos) == SUCCESS) {

			zend_hash_move_forward_ex(methods, &pos);
		}*/

		if (left) {
			zval_ptr_dtor(left);
		}

		if (right) {
			zval_ptr_dtor(right);
		}
	}

	return possible_method;
}

int zephir_call_class_method_aparams(zval *return_value_ptr, zend_class_entry *ce, zephir_call_type type, zval *object,
	const char *method_name, uint32_t method_len,
	zephir_fcall_cache_entry **cache_entry,
	uint32_t param_count, zval *params[])
{
	char *possible_method;
	zval fn;
	zval mn;
	int status;

	if (object) {
		if (Z_TYPE_P(object) != IS_OBJECT) {
			zephir_throw_exception_format(spl_ce_RuntimeException TSRMLS_CC, "Trying to call method %s on a non-object", method_name);
			if (return_value_ptr) {
				ZVAL_UNDEF(return_value_ptr);
			}
			return FAILURE;
		}
	}


	if (method_name == "__construct") {
		ZVAL_UNDEF(&fn); /* We call the constructor */
	} else {
		if ((!cache_entry || !*cache_entry)) {
			array_init_size(&fn, 2);
			switch (type) {
				case zephir_fcall_parent: add_next_index_stringl(&fn, ZEND_STRL("parent")); break;
				case zephir_fcall_self:   assert(!ce); add_next_index_stringl(&fn, ZEND_STRL("self")); break;
				case zephir_fcall_static: assert(!ce); add_next_index_stringl(&fn, ZEND_STRL("static")); break;

				case zephir_fcall_ce:
					assert(ce != NULL);
					add_next_index_str(&fn, ce->name);
					break;

				case zephir_fcall_method:
				default:
					assert(object != NULL);
					Z_ADDREF_P(object);
					add_next_index_zval(&fn, object);
					break;
			}

			ZVAL_STRINGL(&mn, method_name, method_len);
			add_next_index_zval(&fn, &mn);
		} else {
			ZVAL_STRINGL(&fn, "undefined", sizeof("undefined")-1);
		}
	}

	status = zephir_call_user_function(object, ce, type, &fn, return_value_ptr, cache_entry, param_count, params);
	if (status == FAILURE && !EG(exception)) {

		if (ce) {
			//TODO possible_method = zephir_fcall_possible_method(ce, method_name);
			possible_method = "TODO!! zephir_fcall_possible_method: Not ported";
		}

		switch (type) {

			case zephir_fcall_parent:
				if (possible_method) {
					zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method parent::%s(), did you mean '%s'?", method_name, possible_method);
				} else {
					zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method parent::%s()", method_name);
				}
				break;

			case zephir_fcall_self:
				if (possible_method) {
					zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method self::%s(), did you mean '%s'?", method_name, possible_method);
				} else {
					zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method self::%s()", method_name);
				}
				break;

			case zephir_fcall_static:
				zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method static::%s()", method_name);
				break;

			case zephir_fcall_ce:
				zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method %s::%s()", ce->name, method_name);
				break;

			case zephir_fcall_method:
				if (possible_method) {
					zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method %s::%s(), did you mean '%s'?", ce->name, method_name, possible_method);
				} else {
					zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method %s::%s()", ce->name, method_name);
				}
				break;

			default:
				zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined method ?::%s()", method_name);
		}

		if (return_value_ptr) {
			ZVAL_NULL(return_value_ptr);
		}
	} else {
		if (EG(exception)) {
			status = FAILURE;
			if (return_value_ptr) {
				ZVAL_NULL(return_value_ptr);
			}
		}
	}

	zval_ptr_dtor(&fn);

	return status;
}

int zephir_call_func_aparams(zval *return_value_ptr, const char *func_name, uint32_t func_length,
	zephir_fcall_cache_entry **cache_entry,
	uint32_t param_count, zval *params[])
{
	int status;
	zval func;

	ZVAL_STRINGL(&func, func_name, func_length);
	status = zephir_call_user_function(NULL, NULL, zephir_fcall_function, &func, return_value_ptr, cache_entry, param_count, params);

	if (status == FAILURE && !EG(exception)) {
		zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined function %s()", func_name);
		if (return_value_ptr) {
			ZVAL_NULL(return_value_ptr);
		}
	} else {
		if (EG(exception)) {
			status = FAILURE;
			if (return_value_ptr) {
				ZVAL_NULL(return_value_ptr);
			}
		}
	}

	/* TODO: Do we need to care about some references here? */
	zval_ptr_dtor(&func);
	ZVAL_UNDEF(&func);

	return status;
}

int zephir_call_zval_func_aparams(zval *return_value_ptr, zval *func_name,
	zephir_fcall_cache_entry **cache_entry,
	uint32_t param_count, zval *params[])
{
	int status;

	status = zephir_call_user_function(NULL, NULL, zephir_fcall_function, func_name, return_value_ptr, cache_entry, param_count, params);

	if (status == FAILURE && !EG(exception)) {
		zephir_throw_exception_format(spl_ce_RuntimeException, "Call to undefined function %s()", Z_TYPE_P(func_name) ? Z_STRVAL_P(func_name) : "undefined");
		ZVAL_NULL(return_value_ptr);
	} else {
		if (EG(exception)) {
			status = FAILURE;
			if (return_value_ptr) {
				ZVAL_NULL(return_value_ptr);
			}
		}
	}

	return status;
}
