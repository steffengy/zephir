
/*
  +------------------------------------------------------------------------+
  | Zephir Language                                                        |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Zephir Team (http://www.zephir-lang.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@zephir-lang.com so we can send you a copy immediately.      |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@zephir-lang.com>                     |
  +------------------------------------------------------------------------+
*/

#include <php.h>
#include "php_ext.h"

#include <Zend/zend_API.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_execute.h>

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/memory.h"
#include "kernel/hash.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/backtrace.h"

#if PHP_VERSION_ID >= 50600


#if ZEND_MODULE_API_NO >= 20141001
void zephir_clean_and_cache_symbol_table(zend_array *symbol_table)
{
	if (EG(symtable_cache_ptr) >= EG(symtable_cache_limit)) {
		zend_array_destroy(symbol_table);
	} else {
		zend_symtable_clean(symbol_table);
		*(++EG(symtable_cache_ptr)) = symbol_table;
	}
}
#else
void zephir_clean_and_cache_symbol_table(HashTable *symbol_table TSRMLS_DC)
{
	if (EG(symtable_cache_ptr) >= EG(symtable_cache_limit)) {
		zend_hash_destroy(symbol_table);
		FREE_HASHTABLE(symbol_table);
	} else {
		zend_hash_clean(symbol_table);
		*(++EG(symtable_cache_ptr)) = symbol_table;
	}
}
#endif

/**
 * Latest version of zend_throw_exception_internal
 */
static void zephir_throw_exception_internal(zval *exception TSRMLS_DC)
{
	if (exception != NULL) {
#if PHP_VERSION_ID >= 70000
		zend_object *previous;
#else
		zval *previous;
#endif
		previous = EG(exception);
#if PHP_VERSION_ID >= 70000
		zend_exception_set_previous(Z_OBJ_P(exception), EG(exception) TSRMLS_CC);
		EG(exception) = Z_OBJ_P(exception);
#else
		zend_exception_set_previous(exception, EG(exception) TSRMLS_CC);
		EG(exception) = exception;
#endif
		
		if (previous) {
			return;
		}
	}

	if (!EG(current_execute_data)) {
		if (EG(exception)) {
			zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
		}
		zend_error(E_ERROR, "Exception thrown without a stack frame");
	}

	if (zend_throw_exception_hook) {
		zend_throw_exception_hook(exception TSRMLS_CC);
	}

	if (EG(current_execute_data)->opline == NULL ||
		(EG(current_execute_data)->opline + 1)->opcode == ZEND_HANDLE_EXCEPTION) {
		/* no need to rethrow the exception */
		return;
	}

	EG(opline_before_exception) = EG(current_execute_data)->opline;
	EG(current_execute_data)->opline = EG(exception_op);
}

static int zephir_is_callable_check_class(const char *name, int name_len, zend_fcall_info_cache *fcc, int *strict_class, char **error TSRMLS_DC) /* {{{ */
{
	int ret = 0;
#if PHP_VERSION_ID >= 70000
	zend_class_entry *pce;
#else
	zend_class_entry **pce;
#endif
	char *lcname = zend_str_tolower_dup(name, name_len);

	*strict_class = 0;
	if (name_len == sizeof("self") - 1 &&
		!memcmp(lcname, "self", sizeof("self") - 1)) {
		if (!EG(scope)) {
			if (error) *error = estrdup("cannot access self:: when no class scope is active");
		} else {
#if PHP_VERSION_ID >= 70000
			fcc->called_scope = EG(current_execute_data)->called_scope;
			if (!fcc->object) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
			}
#else
			fcc->called_scope = EG(called_scope);
			if (!fcc->object_ptr) {
				fcc->object_ptr = EG(This);
			}
#endif
			fcc->calling_scope = EG(scope);
			ret = 1;
		}
	} else if (name_len == sizeof("parent") - 1 &&
			   !memcmp(lcname, "parent", sizeof("parent") - 1)) {
		if (!EG(scope)) {
			if (error) *error = estrdup("cannot access parent:: when no class scope is active");
		} else if (!EG(scope)->parent) {
			if (error) *error = estrdup("cannot access parent:: when current class scope has no parent");
		} else {
#if PHP_VERSION_ID >= 70000
			fcc->called_scope = EG(current_execute_data)->called_scope;
			if (!fcc->object) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
			}
#else
			fcc->called_scope = EG(called_scope);
			if (!fcc->object_ptr) {
				fcc->object_ptr = EG(This);
			}
#endif
			fcc->calling_scope = EG(scope)->parent;
			*strict_class = 1;
			ret = 1;
		}
	} else if (name_len == sizeof("static") - 1 &&
			   !memcmp(lcname, "static", sizeof("static") - 1)) {
#if PHP_VERSION_ID >= 70000
		if (!EG(current_execute_data)->called_scope) {
#else
		if (!EG(called_scope)) {
#endif
			if (error) *error = estrdup("cannot access static:: when no class scope is active");
		} else {
#if PHP_VERSION_ID >= 70000
			fcc->called_scope = EG(current_execute_data)->called_scope;
			if (!fcc->object) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
			}
#else
			fcc->called_scope = EG(called_scope);
			if (!fcc->object_ptr) {
				fcc->object_ptr = EG(This);
			}
#endif
			fcc->calling_scope = fcc->called_scope;
			*strict_class = 1;
			ret = 1;
		}

	} else {
#if PHP_VERSION_ID >= 70000
		zend_string *class_name;
		class_name = zend_string_init(name, name_len, 0);
		if ((pce = zend_lookup_class_ex(name, NULL, 1)) != NULL) {
			zend_class_entry *scope = EG(current_execute_data) ? EG(current_execute_data)->func->common.scope : NULL;
			fcc->calling_scope = pce;
			if (scope && !fcc->object && EG(current_execute_data) &&
				instanceof_function(Z_OBJCE(EG(current_execute_data)->This), scope TSRMLS_CC) &&
				instanceof_function(scope, fcc->calling_scope TSRMLS_CC)) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
				fcc->called_scope = fcc->object->ce;
			} else {
				fcc->called_scope = fcc->object ? fcc->object->ce : fcc->calling_scope;
			}
#else
		if (zend_lookup_class_ex(name, name_len, NULL, 1, &pce TSRMLS_CC) == SUCCESS) {
			zend_class_entry *scope = EG(active_op_array) ? EG(active_op_array)->scope : NULL;
			fcc->calling_scope = *pce;
			if (scope && !fcc->object_ptr && EG(This) &&
				instanceof_function(Z_OBJCE_P(EG(This)), scope TSRMLS_CC) &&
				instanceof_function(scope, fcc->calling_scope TSRMLS_CC)) {
				fcc->object_ptr = EG(This);
				fcc->called_scope = Z_OBJCE_P(fcc->object_ptr);
			} else {
				fcc->called_scope = fcc->object_ptr ? Z_OBJCE_P(fcc->object_ptr) : fcc->calling_scope;
			}
#endif		
			*strict_class = 1;
			ret = 1;
		} else {
			if (error) zephir_spprintf(error, 0, "class '%.*s' not found", name_len, name);
		}
#if PHP_VERSION_ID >= 70000
		zend_string_free(class_name);
#endif
	}
	efree(lcname);
	return ret;
}

static int zephir_is_callable_check_func(int check_flags, zval *callable, zend_fcall_info_cache *fcc, int strict_class, char **error TSRMLS_DC) /* {{{ */
{
	zend_class_entry *ce_org = fcc->calling_scope;
	int retval = 0;
	char *mname, *lmname;
#if PHP_VERSION_ID >= 70000
	zend_string *zs_mname;
#endif
	const char *colon;
	int clen, mlen;
	zend_class_entry *last_scope;
	HashTable *ftable;
	int call_via_handler = 0;

	if (error) {
		*error = NULL;
	}

	fcc->calling_scope = NULL;
	fcc->function_handler = NULL;

	if (!ce_org) {
		/* Skip leading \ */
		if (Z_STRVAL_P(callable)[0] == '\\') {
			mlen = Z_STRLEN_P(callable) - 1;
			lmname = zend_str_tolower_dup(Z_STRVAL_P(callable) + 1, mlen);
		} else {
			mlen = Z_STRLEN_P(callable);
			lmname = zend_str_tolower_dup(Z_STRVAL_P(callable), mlen);
		}
		/* Check if function with given name exists.
		 * This may be a compound name that includes namespace name */
#if PHP_VERSION_ID >= 70000
		if (EXPECTED((fcc->function_handler = zend_hash_str_find_ptr(EG(function_table), lmname, mlen)) != NULL)) {
#else
		if (zend_hash_find(EG(function_table), lmname, mlen+1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
			efree(lmname);
			return 1;
		}
		efree(lmname);
	}

	/* Split name into class/namespace and method/function names */
	if ((colon = zend_memrchr(Z_STRVAL_P(callable), ':', Z_STRLEN_P(callable))) != NULL &&
		colon > Z_STRVAL_P(callable) &&
		*(colon-1) == ':'
	) {
		colon--;
		clen = colon - Z_STRVAL_P(callable);
		mlen = Z_STRLEN_P(callable) - clen - 2;

		if (colon == Z_STRVAL_P(callable)) {
			if (error) zephir_spprintf(error, 0, "invalid function name");
			return 0;
		}

		/* This is a compound name.
		 * Try to fetch class and then find static method. */
		last_scope = EG(scope);
		if (ce_org) {
			EG(scope) = ce_org;
		}

		if (!zephir_is_callable_check_class(Z_STRVAL_P(callable), clen, fcc, &strict_class, error TSRMLS_CC)) {
			EG(scope) = last_scope;
			return 0;
		}
		EG(scope) = last_scope;

		ftable = &fcc->calling_scope->function_table;
		if (ce_org && !instanceof_function(ce_org, fcc->calling_scope TSRMLS_CC)) {
			if (error) zephir_spprintf(error, 0, "class '%s' is not a subclass of '%s'", ce_org->name, fcc->calling_scope->name);
			return 0;
		}
		mname = Z_STRVAL_P(callable) + clen + 2;
	} else if (ce_org) {
		/* Try to fetch find static method of given class. */
		mlen = Z_STRLEN_P(callable);
		mname = Z_STRVAL_P(callable);
		ftable = &ce_org->function_table;
		fcc->calling_scope = ce_org;
	} else {
		/* We already checked for plain function before. */
		if (error && !(check_flags & IS_CALLABLE_CHECK_SILENT)) {
			zephir_spprintf(error, 0, "function '%s' not found or invalid function name", Z_STRVAL_P(callable));
		}
		return 0;
	}

#if PHP_VERSION_ID >= 70000
	zs_mname = zend_string_init(mname, mlen, 0);
#endif
	lmname = zend_str_tolower_dup(mname, mlen);
	if (strict_class &&
		fcc->calling_scope &&
		mlen == sizeof(ZEND_CONSTRUCTOR_FUNC_NAME)-1 &&
		!memcmp(lmname, ZEND_CONSTRUCTOR_FUNC_NAME, sizeof(ZEND_CONSTRUCTOR_FUNC_NAME) - 1)) {
		fcc->function_handler = fcc->calling_scope->constructor;
		if (fcc->function_handler) {
			retval = 1;
		}
#if PHP_VERSION_ID >= 70000
	} else if ((fcc->function_handler = zend_hash_str_find_ptr(ftable, lmname, mlen)) != NULL) {
#else
	} else if (zend_hash_find(ftable, lmname, mlen+1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
		retval = 1;
		if ((fcc->function_handler->op_array.fn_flags & ZEND_ACC_CHANGED) &&
			!strict_class && EG(scope) &&
			instanceof_function(fcc->function_handler->common.scope, EG(scope) TSRMLS_CC)) {
			zend_function *priv_fbc;

#if PHP_VERSION_ID >= 70000
			if ((priv_fbc = zend_hash_str_find_ptr(&EG(scope)->function_table, lmname, mlen)) != NULL
#else
			if (zend_hash_find(&EG(scope)->function_table, lmname, mlen+1, (void **) &priv_fbc)==SUCCESS
#endif
				&& priv_fbc->common.fn_flags & ZEND_ACC_PRIVATE
				&& priv_fbc->common.scope == EG(scope)) {
				fcc->function_handler = priv_fbc;
			}
		}
	} else {
		
#if PHP_VERSION_ID >= 70000
		if (fcc->object && fcc->calling_scope == ce_org) {
			if (strict_class && ce_org->__call) {
				fcc->function_handler = zend_get_call_trampoline_func(ce_org, zs_mname, 0);
#else
		if (fcc->object_ptr && fcc->calling_scope == ce_org) {
			if (strict_class && ce_org->__call) {
				fcc->function_handler = emalloc(sizeof(zend_internal_function));
				fcc->function_handler->internal_function.type = ZEND_INTERNAL_FUNCTION;
				fcc->function_handler->internal_function.module = (ce_org->type == ZEND_INTERNAL_CLASS) ? ce_org->info.internal.module : NULL;
				fcc->function_handler->internal_function.handler = zend_std_call_user_call;
				fcc->function_handler->internal_function.arg_info = NULL;
				fcc->function_handler->internal_function.num_args = 0;
				fcc->function_handler->internal_function.scope = ce_org;
				fcc->function_handler->internal_function.fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
				fcc->function_handler->internal_function.function_name = estrndup(mname, mlen);
#endif
				call_via_handler = 1;
				retval = 1;
#if PHP_VERSION_ID >= 70000
			} else if (fcc->object->handlers->get_method) {
				fcc->function_handler = fcc->object->handlers->get_method(&fcc->object, zs_mname, NULL);
#else
			} else if (Z_OBJ_HT_P(fcc->object_ptr)->get_method) {
				fcc->function_handler = Z_OBJ_HT_P(fcc->object_ptr)->get_method(&fcc->object_ptr, mname, mlen, NULL TSRMLS_CC);
#endif
				if (fcc->function_handler) {
					if (strict_class &&
						(!fcc->function_handler->common.scope ||
						 !instanceof_function(ce_org, fcc->function_handler->common.scope TSRMLS_CC))) {
						if ((fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0) {
							if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
							}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
						}
					} else {
						retval = 1;
						call_via_handler = (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0;
					}
				}
			}
		} else if (fcc->calling_scope) {
			if (fcc->calling_scope->get_static_method) {
#if PHP_VERSION_ID >= 70000
				fcc->function_handler = fcc->calling_scope->get_static_method(fcc->calling_scope, zs_mname);
			} else {
				fcc->function_handler = zend_std_get_static_method(fcc->calling_scope, zs_mname, NULL);
			}
#else
				fcc->function_handler = fcc->calling_scope->get_static_method(fcc->calling_scope, mname, mlen TSRMLS_CC);
			} else {
				fcc->function_handler = zend_std_get_static_method(fcc->calling_scope, mname, mlen, NULL TSRMLS_CC);
			}
#endif
			
			if (fcc->function_handler) {
				retval = 1;
				call_via_handler = (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0;
#if PHP_VERSION_ID >= 70000
				if (call_via_handler && !fcc->object && EG(current_execute_data) && Z_OBJ(EG(current_execute_data)->This) &&
				    instanceof_function(Z_OBJCE(EG(current_execute_data)->This), fcc->calling_scope)) {
					fcc->object = Z_OBJ(EG(current_execute_data)->This);
				}
#else
				if (call_via_handler && !fcc->object_ptr && EG(This) &&
					Z_OBJ_HT_P(EG(This))->get_class_entry &&
					instanceof_function(Z_OBJCE_P(EG(This)), fcc->calling_scope TSRMLS_CC)) {
					fcc->object_ptr = EG(This);
				}
#endif
			}
		}
	}

	if (retval) {
		if (fcc->calling_scope && !call_via_handler) {
#if PHP_VERSION_ID >= 70000
				if (!fcc->object && (fcc->function_handler->common.fn_flags & ZEND_ACC_ABSTRACT)) {
#else
		    	if (!fcc->object_ptr && (fcc->function_handler->common.fn_flags & ZEND_ACC_ABSTRACT)) {
#endif
				if (error) {
					zephir_spprintf(error, 0, "cannot call abstract method %s::%s()", fcc->calling_scope->name, fcc->function_handler->common.function_name);
					retval = 0;
				} else {
					zend_error(E_ERROR, "Cannot call abstract method %s::%s()", fcc->calling_scope->name, fcc->function_handler->common.function_name);
				}
#if PHP_VERSION_ID >= 70000
				} else if (!fcc->object && !(fcc->function_handler->common.fn_flags & ZEND_ACC_STATIC)) {
#else
				} else if (!fcc->object_ptr && !(fcc->function_handler->common.fn_flags & ZEND_ACC_STATIC)) {
#endif
				int severity;
				char *verb;
				if (fcc->function_handler->common.fn_flags & ZEND_ACC_ALLOW_STATIC) {
					severity = E_STRICT;
					verb = "should not";
				} else {
					/* An internal function assumes $this is present and won't check that. So PHP would crash by allowing the call. */
					severity = E_ERROR;
					verb = "cannot";
				}
				if ((check_flags & IS_CALLABLE_CHECK_IS_STATIC) != 0) {
					retval = 0;
				}
#if PHP_VERSION_ID < 70000
				if (EG(This) && instanceof_function(Z_OBJCE_P(EG(This)), fcc->calling_scope TSRMLS_CC)) {

					fcc->object_ptr = EG(This);

					if (error) {
						zephir_spprintf(error, 0, "non-static method %s::%s() %s be called statically, assuming $this from compatible context %s", fcc->calling_scope->name, fcc->function_handler->common.function_name, verb, Z_OBJCE_P(EG(This))->name);
						if (severity == E_ERROR) {
							retval = 0;
						}
					} else if (retval) {
						zend_error(severity, "Non-static method %s::%s() %s be called statically, assuming $this from compatible context %s", fcc->calling_scope->name, fcc->function_handler->common.function_name, verb, Z_OBJCE_P(EG(This))->name);
					}
				} else {
#endif
					if (error) {
						zephir_spprintf(error, 0, "non-static method %s::%s() %s be called statically", fcc->calling_scope->name, fcc->function_handler->common.function_name, verb);
						if (severity == E_ERROR) {
							retval = 0;
						}
					} else if (retval) {
						zend_error(severity, "Non-static method %s::%s() %s be called statically", fcc->calling_scope->name, fcc->function_handler->common.function_name, verb);
					}
#if PHP_VERSION_ID < 70000
				}
#endif
			}
		}
	} else if (error && !(check_flags & IS_CALLABLE_CHECK_SILENT)) {
		if (fcc->calling_scope) {
			if (error) zephir_spprintf(error, 0, "class '%s' does not have a method '%s'", fcc->calling_scope->name, mname);
		} else {
			if (error) zephir_spprintf(error, 0, "function '%s' does not exist", mname);
		}
	}
	efree(lmname);
#if PHP_VERSION_ID >= 70000
	zend_string_free(zs_mname);
	if (fcc->object) {
		fcc->called_scope = fcc->object->ce;
	}
#else
	if (fcc->object_ptr) {
		fcc->called_scope = Z_OBJCE_P(fcc->object_ptr);
	}
#endif

	
	if (retval) {
		fcc->initialized = 1;
	}
	return retval;
}

#if PHP_VERSION_ID >= 70000
static zend_bool zephir_is_callable_ex(zval *callable, zend_object *object, uint check_flags, char **callable_name, int *callable_name_len, zend_fcall_info_cache *fcc, char **error TSRMLS_DC) /* {{{ */
#else
static zend_bool zephir_is_callable_ex(zval *callable, zval *object_ptr, uint check_flags, char **callable_name, int *callable_name_len, zend_fcall_info_cache *fcc, char **error TSRMLS_DC) /* {{{ */
#endif
{
	zend_bool ret;
	int callable_name_len_local;
	zend_fcall_info_cache fcc_local;

	if (callable_name) {
		*callable_name = NULL;
	}
	if (callable_name_len == NULL) {
		callable_name_len = &callable_name_len_local;
	}
	if (fcc == NULL) {
		fcc = &fcc_local;
	}
	if (error) {
		*error = NULL;
	}

	fcc->initialized = 0;
	fcc->calling_scope = NULL;
	fcc->called_scope = NULL;
	fcc->function_handler = NULL;

#if PHP_VERSION_ID >= 70000
	fcc->object = NULL;
	if (object &&
	    (!EG(objects_store).object_buckets ||
	     !IS_OBJ_VALID(EG(objects_store).object_buckets[object->handle]))) {
		return 0;
	}
#else
	if (object_ptr && Z_TYPE_P(object_ptr) != IS_OBJECT) {
		object_ptr = NULL;
	}
	fcc->object_ptr = NULL;
	if (object_ptr &&
		(!EG(objects_store).object_buckets ||
		 !EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(object_ptr)].valid)) {
		return 0;
	}
#endif

	switch (Z_TYPE_P(callable)) {

		case IS_STRING:
			
#if PHP_VERSION_ID >= 70000
		if (object) {
				fcc->object = object;
				fcc->calling_scope = object->ce;
#else
		if (object_ptr) {
				fcc->object_ptr = object_ptr;
				fcc->calling_scope = Z_OBJCE_P(object_ptr);
#endif
				
				if (callable_name) {
					char *ptr;
#if PHP_VERSION_ID >= 70000
					*callable_name_len = fcc->calling_scope->name->len + Z_STRLEN_P(callable) + sizeof("::") - 1;
					ptr = *callable_name = emalloc(*callable_name_len + 1);
					memcpy(ptr, fcc->calling_scope->name->val, fcc->calling_scope->name->len);
					ptr += fcc->calling_scope->name->len;
#else
					*callable_name_len = fcc->calling_scope->name_length + Z_STRLEN_P(callable) + sizeof("::") - 1;
					ptr = *callable_name = emalloc(*callable_name_len + 1);
					memcpy(ptr, fcc->calling_scope->name, fcc->calling_scope->name_length);
					ptr += fcc->calling_scope->name_length;
#endif
					memcpy(ptr, "::", sizeof("::") - 1);
					ptr += sizeof("::") - 1;
					memcpy(ptr, Z_STRVAL_P(callable), Z_STRLEN_P(callable) + 1);
					
				}
			} else if (callable_name) {
				*callable_name = estrndup(Z_STRVAL_P(callable), Z_STRLEN_P(callable));
				*callable_name_len = Z_STRLEN_P(callable);
			}
			if (check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY) {
				fcc->called_scope = fcc->calling_scope;
				return 1;
			}

			ret = zephir_is_callable_check_func(check_flags, callable, fcc, 0, error TSRMLS_CC);
			if (fcc == &fcc_local &&
				fcc->function_handler &&
				((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
				  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
				 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
				 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
				if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
				}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
			}
			return ret;

		case IS_ARRAY:
			{
				int strict_class = 0;

#if PHP_VERSION_ID >= 70000
				zval *method = NULL;
				zval *obj = NULL;
				if (zend_hash_num_elements(Z_ARRVAL_P(callable)) == 2) {
					obj = zend_hash_index_find(Z_ARRVAL_P(callable), 0);
					method = zend_hash_index_find(Z_ARRVAL_P(callable), 1);
				}
#else
				zval **method = NULL;
				zval **obj = NULL;
#endif
				do {
#if PHP_VERSION_ID >= 70000
					if (obj == NULL || method == NULL) {
						break;
					}
					ZVAL_DEREF(method);
					if (Z_TYPE_P(method) != IS_STRING) {
						break;
					}

					ZVAL_DEREF(obj);
					if (Z_TYPE_P(obj) == IS_STRING) {
#else
					
					if (zend_hash_num_elements(Z_ARRVAL_P(callable)) == 2) {
						zend_hash_index_find(Z_ARRVAL_P(callable), 0, (void **) &obj);
						zend_hash_index_find(Z_ARRVAL_P(callable), 1, (void **) &method);
					}
					if (!obj || !method ||
						(Z_TYPE_PP(obj) != IS_OBJECT && Z_TYPE_PP(obj) != IS_STRING) 
						|| Z_TYPE_PP(method) != IS_STRING) {
						break;
					}
					if (Z_TYPE_PP(obj) == IS_STRING) {
#endif
						if (callable_name) {
							char *ptr;

#if PHP_VERSION_ID >= 70000
							*callable_name_len = Z_STRLEN_P(obj) + Z_STRLEN_P(method) + sizeof("::") - 1;
							ptr = *callable_name = emalloc(*callable_name_len + 1);
							memcpy(ptr, Z_STRVAL_P(obj), Z_STRLEN_P(obj));
							ptr += Z_STRLEN_P(obj);
							memcpy(ptr, "::", sizeof("::") - 1);
							ptr += sizeof("::") - 1;
							memcpy(ptr, Z_STRVAL_P(method), Z_STRLEN_P(method) + 1);
#else
							*callable_name_len = Z_STRLEN_PP(obj) + Z_STRLEN_PP(method) + sizeof("::") - 1;
							ptr = *callable_name = emalloc(*callable_name_len + 1);
							memcpy(ptr, Z_STRVAL_PP(obj), Z_STRLEN_PP(obj));
							ptr += Z_STRLEN_PP(obj);
							memcpy(ptr, "::", sizeof("::") - 1);
							ptr += sizeof("::") - 1;
							memcpy(ptr, Z_STRVAL_PP(method), Z_STRLEN_PP(method) + 1);
#endif	
						}

						if (check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY) {
							return 1;
						}

#if PHP_VERSION_ID >= 70000
						if (!zephir_is_callable_check_class(Z_STRVAL_P(obj), Z_STRLEN_P(obj), fcc, &strict_class, error TSRMLS_CC)) {
#else
						if (!zephir_is_callable_check_class(Z_STRVAL_PP(obj), Z_STRLEN_PP(obj), fcc, &strict_class, error TSRMLS_CC)) {
#endif
							return 0;
						}

					} else {
#if PHP_VERSION_ID >= 70000
						if (!EG(objects_store).object_buckets ||
						    !IS_OBJ_VALID(EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(obj)])) {
							return 0;
						}
						fcc->calling_scope = Z_OBJCE_P(obj); /* TBFixed: what if it's overloaded? */
						fcc->object = Z_OBJ_P(obj);
#else
						if (!EG(objects_store).object_buckets ||
							!EG(objects_store).object_buckets[Z_OBJ_HANDLE_PP(obj)].valid) {
							return 0;
						}
						fcc->calling_scope = Z_OBJCE_PP(obj); /* TBFixed: what if it's overloaded? */
						fcc->object_ptr = *obj;
#endif	
						
						if (callable_name) {
							char *ptr;
#if PHP_VERSION_ID >= 70000
							*callable_name_len = fcc->calling_scope->name->len + Z_STRLEN_P(method) + sizeof("::") - 1;
							ptr = *callable_name = emalloc(*callable_name_len + 1);
							memcpy(ptr, fcc->calling_scope->name->val, fcc->calling_scope->name->len);
							ptr += fcc->calling_scope->name->len;
							memcpy(ptr, "::", sizeof("::") - 1);
							ptr += sizeof("::") - 1;
							memcpy(ptr, Z_STRVAL_P(method), Z_STRLEN_P(method) + 1);
#else
							*callable_name_len = fcc->calling_scope->name_length + Z_STRLEN_PP(method) + sizeof("::") - 1;
							ptr = *callable_name = emalloc(*callable_name_len + 1);
							memcpy(ptr, fcc->calling_scope->name, fcc->calling_scope->name_length);
							ptr += fcc->calling_scope->name_length;
							memcpy(ptr, "::", sizeof("::") - 1);
							ptr += sizeof("::") - 1;
							memcpy(ptr, Z_STRVAL_PP(method), Z_STRLEN_PP(method) + 1);
#endif	
						}

						if (check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY) {
							fcc->called_scope = fcc->calling_scope;
							return 1;
						}
					}
#if PHP_VERSION_ID >= 70000
					ret = zephir_is_callable_check_func(check_flags, method, fcc, strict_class, error TSRMLS_CC);
#else
					ret = zephir_is_callable_check_func(check_flags, *method, fcc, strict_class, error TSRMLS_CC);
#endif
					
					if (fcc == &fcc_local &&
						fcc->function_handler &&
						((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
						  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
						 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
						 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
						if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
						}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
					}
					return ret;
				} while (0);
				if (zend_hash_num_elements(Z_ARRVAL_P(callable)) == 2) {
#if PHP_VERSION_ID >= 70000
					if (!obj || (!Z_ISREF_P(obj)?
								(Z_TYPE_P(obj) != IS_STRING && Z_TYPE_P(obj) != IS_OBJECT) :
								(Z_TYPE_P(Z_REFVAL_P(obj)) != IS_STRING && Z_TYPE_P(Z_REFVAL_P(obj)) != IS_OBJECT))) {
#else
					if (!obj || (Z_TYPE_PP(obj) != IS_STRING && Z_TYPE_PP(obj) != IS_OBJECT)) {
#endif
						if (error) zephir_spprintf(error, 0, "first array member is not a valid class name or object");
					} else {
						if (error) zephir_spprintf(error, 0, "second array member is not a valid method");
					}
				} else {
					if (error) zephir_spprintf(error, 0, "array must have exactly two members");
				}
				if (callable_name) {
					*callable_name = estrndup("Array", sizeof("Array")-1);
					*callable_name_len = sizeof("Array") - 1;
				}
			}
			return 0;

		case IS_OBJECT:
#if PHP_VERSION_ID >= 70000
			if (Z_OBJ_HANDLER_P(callable, get_closure) && Z_OBJ_HANDLER_P(callable, get_closure)(callable, &fcc->calling_scope, &fcc->function_handler, &fcc->object) == SUCCESS)
#else
			if (Z_OBJ_HANDLER_P(callable, get_closure) && Z_OBJ_HANDLER_P(callable, get_closure)(callable, &fcc->calling_scope, &fcc->function_handler, &fcc->object_ptr TSRMLS_CC) == SUCCESS)
#endif
			{
				fcc->called_scope = fcc->calling_scope;
				if (callable_name) {
					zend_class_entry *ce = Z_OBJCE_P(callable); /* TBFixed: what if it's overloaded? */
#if PHP_VERSION_ID >= 70000
					*callable_name_len = ce->name->len + sizeof("::__invoke") - 1;
					*callable_name = emalloc(*callable_name_len + 1);
					memcpy(*callable_name, ce->name->val, ce->name->len);
					memcpy((*callable_name) + ce->name->len, "::__invoke", sizeof("::__invoke"));
#else
					*callable_name_len = ce->name_length + sizeof("::__invoke") - 1;
					*callable_name = emalloc(*callable_name_len + 1);
					memcpy(*callable_name, ce->name, ce->name_length);
					memcpy((*callable_name) + ce->name_length, "::__invoke", sizeof("::__invoke"));
#endif
					
				}
				return 1;
			}
			/* break missing intentionally */

		default:
			if (callable_name) {
				zval expr_copy;
				int use_copy;
#if PHP_VERSION_ID >= 70000
				use_copy = zend_make_printable_zval(callable, &expr_copy);
#else
				zend_make_printable_zval(callable, &expr_copy, &use_copy);
#endif
				
				*callable_name = estrndup(Z_STRVAL(expr_copy), Z_STRLEN(expr_copy));
				*callable_name_len = Z_STRLEN(expr_copy);
				zval_dtor(&expr_copy);
			}
			if (error) zephir_spprintf(error, 0, "no array or string given");
			return 0;
	}
}

static zend_bool zephir_is_info_dynamic_callable(zephir_fcall_info *info, zend_fcall_info_cache *fcc, zend_class_entry *ce_org, int strict_class TSRMLS_DC)
{
	int call_via_handler = 0, retval = 0;
	
#if PHP_VERSION_ID >= 70000
	zend_string *zs_lcname = zend_string_alloc(info->func_length, 0);
	zend_str_tolower_copy(zs_lcname->val, info->func_name, info->func_length);
#else
	char *lcname = zend_str_tolower_dup(info->func_name, info->func_length);
#endif


#if PHP_VERSION_ID >= 70000
	if (fcc->object && fcc->calling_scope == ce_org) {
#else
	if (fcc->object_ptr && fcc->calling_scope == ce_org) {
#endif
		if (strict_class && ce_org->__call) {

#if PHP_VERSION_ID >= 70000
			fcc->function_handler = zend_get_call_trampoline_func(ce_org, zs_lcname, 0);
#else
			fcc->function_handler = emalloc(sizeof(zend_internal_function));
			fcc->function_handler->internal_function.type = ZEND_INTERNAL_FUNCTION;
			fcc->function_handler->internal_function.module = (ce_org->type == ZEND_INTERNAL_CLASS) ? ce_org->info.internal.module : NULL;
			fcc->function_handler->internal_function.handler = zend_std_call_user_call;
			fcc->function_handler->internal_function.arg_info = NULL;
			fcc->function_handler->internal_function.num_args = 0;
			fcc->function_handler->internal_function.scope = ce_org;
			fcc->function_handler->internal_function.fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
			fcc->function_handler->internal_function.function_name = estrndup(lcname, info->func_length);
#endif
			call_via_handler = 1;
			retval = 1;
#if PHP_VERSION_ID >= 70000
		} else if (fcc->object->handlers->get_method) {
			fcc->function_handler = fcc->object->handlers->get_method(&fcc->object, zs_lcname, NULL);
#else
		} else if (Z_OBJ_HT_P(fcc->object_ptr)->get_method) {
			fcc->function_handler = Z_OBJ_HT_P(fcc->object_ptr)->get_method(&fcc->object_ptr, lcname, info->func_length, NULL TSRMLS_CC);
#endif
			if (fcc->function_handler) {
				if (strict_class &&
					(!fcc->function_handler->common.scope ||
					 !instanceof_function(ce_org, fcc->function_handler->common.scope TSRMLS_CC))) {
					if ((fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0) {
						if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
						}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
					}
				} else {
					call_via_handler = (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0;
					retval = 1;
				}
			}
		}
	} else if (fcc->calling_scope) {
#if PHP_VERSION_ID >= 70000
		if (fcc->calling_scope->get_static_method) {
			fcc->function_handler = fcc->calling_scope->get_static_method(fcc->calling_scope, zs_lcname);
		} else {
			fcc->function_handler = zend_std_get_static_method(fcc->calling_scope, zs_lcname, NULL);
		}
#else
		if (fcc->calling_scope->get_static_method) {
			fcc->function_handler = fcc->calling_scope->get_static_method(fcc->calling_scope, lcname, info->func_length TSRMLS_CC);
		} else {
			fcc->function_handler = zend_std_get_static_method(fcc->calling_scope, lcname, info->func_length, NULL TSRMLS_CC);
		}
#endif
		
		if (fcc->function_handler) {
			call_via_handler = (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0;
#if PHP_VERSION_ID >= 70000
			if (call_via_handler && !fcc->object && EG(current_execute_data) && Z_OBJ(EG(current_execute_data)->This) &&
			    instanceof_function(Z_OBJCE(EG(current_execute_data)->This), fcc->calling_scope)) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
			}
#else
			if (call_via_handler && !fcc->object_ptr && EG(This) &&
				Z_OBJ_HT_P(EG(This))->get_class_entry &&
				instanceof_function(Z_OBJCE_P(EG(This)), fcc->calling_scope TSRMLS_CC)) {
				fcc->object_ptr = EG(This);
			}
#endif
			retval = 1;
		}
	}

#if PHP_VERSION_ID >= 70000
	zend_string_free(zs_lcname);
#else
	efree(lcname);
#endif

	return retval;
}

static zend_bool zephir_is_info_callable_ex(zephir_fcall_info *info, zend_fcall_info_cache *fcc TSRMLS_DC)
{
	int retval = 0;
	zend_class_entry *ce_org = fcc->calling_scope;
	zend_fcall_info_cache fcc_local;

	if (fcc == NULL) {
		fcc = &fcc_local;
	}

	fcc->initialized = 0;
	fcc->calling_scope = NULL;
	fcc->called_scope = NULL;
	fcc->function_handler = NULL;

#if PHP_VERSION_ID >= 70000
	fcc->object = NULL;
#else
	fcc->object_ptr = NULL;
#endif

	switch (info->type) {

		case ZEPHIR_FCALL_TYPE_FUNC:

#if PHP_VERSION_ID >= 70000
			if ((fcc->function_handler = zend_hash_str_find_ptr(EG(function_table), info->func_name, info->func_length)) != NULL) {
#else
			if (zend_hash_find(EG(function_table), info->func_name, info->func_length + 1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
				if (fcc == &fcc_local &&
					fcc->function_handler &&
					((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
					  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
					if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
					}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
				}
				fcc->initialized = 1;
				return 1;
			}
			break;

		case ZEPHIR_FCALL_TYPE_ZVAL_METHOD:

#if PHP_VERSION_ID >= 70000
		if (!EG(objects_store).object_buckets || !IS_OBJ_VALID(EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(info->object_ptr)])) {
			return 0;
		}
		fcc->calling_scope = Z_OBJCE_P(info->object_ptr); /* TBFixed: what if it's overloaded? */
		fcc->object = Z_OBJ_P(info->object_ptr);
#else
		if (!EG(objects_store).object_buckets || !EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(info->object_ptr)].valid) {
			return 0;
		}
		fcc->calling_scope = Z_OBJCE_P(info->object_ptr); /* TBFixed: what if it's overloaded? */
		fcc->object_ptr = info->object_ptr;
#endif

			fcc->called_scope = fcc->calling_scope;
			
			if (!ce_org) {
				ce_org = fcc->calling_scope;
			}

#if PHP_VERSION_ID >= 70000
			if ((fcc->function_handler = zend_hash_str_find_ptr(&info->ce->function_table, info->func_name, info->func_length + 1)) != NULL) {
#else
			if (zend_hash_find(&info->ce->function_table, info->func_name, info->func_length + 1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
				if (fcc == &fcc_local &&
					fcc->function_handler &&
					((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
					  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
					if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
					}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
				}
				retval = 1;
			}

			if (!retval) {
				retval = zephir_is_info_dynamic_callable(info, fcc, ce_org, 0 TSRMLS_CC);
			}
			break;

		case ZEPHIR_FCALL_TYPE_CLASS_SELF_METHOD:

			if (!EG(scope)) {
				return 0; // cannot access self:: when no class scope is active
			}

			fcc->calling_scope = EG(scope);

#if PHP_VERSION_ID >= 70000
		fcc->called_scope = EG(current_execute_data)->called_scope;
		if (!fcc->object) {
			fcc->object = Z_OBJ(EG(current_execute_data)->This);
		}
		if ((fcc->function_handler = zend_hash_str_find_ptr(&fcc->calling_scope->function_table, info->func_name, info->func_length)) != NULL) {
#else
		fcc->called_scope = EG(called_scope);
		if (!fcc->object_ptr) {
			fcc->object_ptr = EG(This);
		}
		if (zend_hash_find(&fcc->calling_scope->function_table, info->func_name, info->func_length + 1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
				if (fcc == &fcc_local &&
					fcc->function_handler &&
					((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
					  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
					if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
					}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
				}
				retval = 1;
			}

			if (!retval) {
				retval = zephir_is_info_dynamic_callable(info, fcc, ce_org, 0 TSRMLS_CC);
			}
			break;

		case ZEPHIR_FCALL_TYPE_CLASS_PARENT_METHOD:

			if (!EG(scope)) {
				return 0; // cannot access parent:: when no class scope is active
			}

			if (!EG(scope)->parent) {
				return 0; // cannot access parent:: when current class scope has no parent
			}

			fcc->calling_scope = EG(scope)->parent;
#if PHP_VERSION_ID >= 70000
			fcc->called_scope = EG(current_execute_data)->called_scope;
			if (!fcc->object) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
			}
			if ((fcc->function_handler = zend_hash_str_find_ptr(&fcc->calling_scope->function_table, info->func_name, info->func_length)) != NULL) {
#else
			fcc->called_scope = EG(called_scope);
			if (!fcc->object_ptr) {
				fcc->object_ptr = EG(This);
			}
			if (zend_hash_find(&fcc->calling_scope->function_table, info->func_name, info->func_length + 1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
			
				if (fcc == &fcc_local &&
					fcc->function_handler &&
					((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
					  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
					if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
					}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
				}
				retval = 1;
			}

			if (!retval) {
				retval = zephir_is_info_dynamic_callable(info, fcc, ce_org, 1 TSRMLS_CC);
			}
			break;

		case ZEPHIR_FCALL_TYPE_CLASS_STATIC_METHOD:

#if PHP_VERSION_ID >= 70000
			if (!EG(current_execute_data) || !EG(current_execute_data)->called_scope) {
#else
			if (!EG(called_scope)) {
#endif
				return 0; // cannot access static:: when no class scope is active
			}

#if PHP_VERSION_ID >= 70000
			fcc->called_scope = EG(current_execute_data)->called_scope;
			if (!fcc->object) {
				fcc->object = Z_OBJ(EG(current_execute_data)->This);
			}
#else
			fcc->called_scope = EG(called_scope);
			if (!fcc->object_ptr) {
				fcc->object_ptr = EG(This);
			}
#endif
			fcc->calling_scope = fcc->called_scope;

#if PHP_VERSION_ID >= 70000
			if ((fcc->function_handler = zend_hash_str_find_ptr(&fcc->calling_scope->function_table, info->func_name, info->func_length)) != NULL) {
#else
			if (zend_hash_find(&fcc->calling_scope->function_table, info->func_name, info->func_length + 1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
				if (fcc == &fcc_local &&
					fcc->function_handler &&
					((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
					  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
					 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
					if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
					}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
				}
				retval = 1;
			}

			if (!retval) {
				retval = zephir_is_info_dynamic_callable(info, fcc, ce_org, 1 TSRMLS_CC);
			}
			break;

		case ZEPHIR_FCALL_TYPE_CE_METHOD:
			{
				zend_class_entry *scope;
				fcc->calling_scope = info->ce;
#if PHP_VERSION_ID >= 70000
				scope = EG(current_execute_data) ? EG(current_execute_data)->func->common.scope : NULL;
				if (scope && !fcc->object && EG(current_execute_data) && Z_OBJ(EG(current_execute_data)->This) &&
		    		instanceof_function(Z_OBJCE(EG(current_execute_data)->This), scope) &&
				    instanceof_function(scope, fcc->calling_scope)) {
					fcc->object = Z_OBJ(EG(current_execute_data)->This);
					fcc->called_scope = Z_OBJCE(EG(current_execute_data)->This);
				} else {
					fcc->called_scope = fcc->object ? fcc->object->ce : fcc->calling_scope;
				}
				if ((fcc->function_handler = zend_hash_str_find_ptr(&fcc->calling_scope->function_table, info->func_name, info->func_length)) != NULL) {
#else
				scope = EG(active_op_array) ? EG(active_op_array)->scope : NULL;
				if (scope && !fcc->object_ptr && EG(This) &&
					instanceof_function(Z_OBJCE_P(EG(This)), scope TSRMLS_CC) &&
					instanceof_function(scope, fcc->calling_scope TSRMLS_CC)) {
					fcc->object_ptr = EG(This);
					fcc->called_scope = Z_OBJCE_P(fcc->object_ptr);
				} else {
					fcc->called_scope = fcc->object_ptr ? Z_OBJCE_P(fcc->object_ptr) : fcc->calling_scope;
				}
				if (zend_hash_find(&fcc->calling_scope->function_table, info->func_name, info->func_length + 1, (void**)&fcc->function_handler) == SUCCESS) {
#endif
					if (fcc == &fcc_local &&
						fcc->function_handler &&
						((fcc->function_handler->type == ZEND_INTERNAL_FUNCTION &&
						  (fcc->function_handler->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER)) ||
						 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY ||
						 fcc->function_handler->type == ZEND_OVERLOADED_FUNCTION)) {
						if (fcc->function_handler->type != ZEND_OVERLOADED_FUNCTION) {
#if PHP_VERSION_ID >= 70000
						zend_string_release(fcc->function_handler->common.function_name);
#else
						efree((char*)fcc->function_handler->common.function_name);
#endif
						}
#if PHP_VERSION_ID >= 70000
					zend_free_trampoline(fcc->function_handler);
#else
					efree(fcc->function_handler);
#endif
					}
					retval = 1;
				}

				if (!retval) {
					retval = zephir_is_info_dynamic_callable(info, fcc, ce_org, 1 TSRMLS_CC);
				}
			}
			break;
	}

#if PHP_VERSION_ID >= 70000
	if (fcc->object) {
		fcc->called_scope = fcc->object->ce;
	}
#else
	if (fcc->object_ptr) {
		fcc->called_scope = Z_OBJCE_P(fcc->object_ptr);
	}
#endif
	
	if (retval) {
		fcc->initialized = 1;
	}

	return retval;
}

int zephir_call_function_opt(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache, zephir_fcall_info *info TSRMLS_DC)
{
	zend_uint i;
	zend_class_entry *calling_scope = NULL;
	zend_execute_data execute_data;
	zend_fcall_info_cache fci_cache_local;
	zend_uint fn_flags;
	zend_function *func;
	
#if PHP_VERSION_ID >= 70000
	zend_class_entry *orig_scope;
	zend_execute_data *call;
	ZVAL_UNDEF(fci->retval);
#else
	zend_class_entry *current_scope;
	zend_class_entry *current_called_scope;
	HashTable *calling_symbol_table;
	zend_op **original_opline_ptr;
	zend_op_array *original_op_array;
	zval **original_return_value;
	zend_class_entry *called_scope = NULL;
	zval *current_this;
	*fci->retval_ptr_ptr = NULL;
#endif

	if (!EG(active)) {
		return FAILURE; /* executor is already inactive */
	}

	if (EG(exception)) {
		return FAILURE; /* we would result in an instable executor otherwise */
	}

	/* Initialize execute_data */
#if PHP_VERSION_ID >= 70000
	orig_scope = EG(scope);

	if (!EG(current_execute_data)) {
		/* This only happens when we're called outside any execute()'s
		 * It shouldn't be strictly necessary to NULL execute_data out,
		 * but it may make bugs easier to spot
		 */
		memset(&execute_data, 0, sizeof(zend_execute_data));
		EG(current_execute_data) = &execute_data;
	} else if (EG(current_execute_data)->func &&
	           ZEND_USER_CODE(EG(current_execute_data)->func->common.type) &&
	           EG(current_execute_data)->opline->opcode != ZEND_DO_FCALL &&
	           EG(current_execute_data)->opline->opcode != ZEND_DO_ICALL &&
	           EG(current_execute_data)->opline->opcode != ZEND_DO_UCALL &&
	           EG(current_execute_data)->opline->opcode != ZEND_DO_FCALL_BY_NAME) {
		/* Insert fake frame in case of include or magic calls */
		execute_data = *EG(current_execute_data);
		execute_data.prev_execute_data = EG(current_execute_data);
		execute_data.call = NULL;
		execute_data.opline = NULL;
		execute_data.func = NULL;
		EG(current_execute_data) = &execute_data;
	}
#else
	if (EG(current_execute_data)) {
		execute_data = *EG(current_execute_data);
		EX(op_array) = NULL;
		EX(opline) = NULL;
		EX(object) = NULL;
	} else {
		/* This only happens when we're called outside any execute()'s
		 * It shouldn't be strictly necessary to NULL execute_data out,
		 * but it may make bugs easier to spot
		 */
		memset(&execute_data, 0, sizeof(zend_execute_data));
	}
#endif

	if (!fci_cache || !fci_cache->initialized) {
		char *callable_name;
		char *error = NULL;

		if (!fci_cache) {
			fci_cache = &fci_cache_local;
		}

		if (!info) {
#if PHP_VERSION_ID >= 70000
			if (!zephir_is_callable_ex(&fci->function_name, fci->object, IS_CALLABLE_CHECK_SILENT, &callable_name, NULL, fci_cache, &error TSRMLS_CC)) {
#else
			if (!zephir_is_callable_ex(fci->function_name, fci->object_ptr, IS_CALLABLE_CHECK_SILENT, &callable_name, NULL, fci_cache, &error TSRMLS_CC)) {
#endif
				if (error) {
					zend_error(E_WARNING, "Invalid callback %s, %s", callable_name, error);
					efree(error);
				}
				if (callable_name) {
					efree(callable_name);
				}
				return FAILURE;
			} else {
				if (error) {
					zend_error(E_STRICT, "%s", error);
					efree(error);
				}
			}
			efree(callable_name);
		} else {
			if (!zephir_is_info_callable_ex(info, fci_cache TSRMLS_CC)) {
				return FAILURE;
			}
		}
	}

#ifndef ZEPHIR_RELEASE
	/*fprintf(stderr, "initialized: %d\n", fci_cache->initialized);
	if (fci_cache->function_handler) {
		if (fci_cache->function_handler->type == ZEND_INTERNAL_FUNCTION) {
			fprintf(stderr, "function handler: %s\n", fci_cache->function_handler->common.function_name);
		} else {
			fprintf(stderr, "function handler: %s\n", "unknown");
		}
	} else {
		fprintf(stderr, "function handler: NONE\n");
	}
	if (fci_cache->calling_scope) {
		fprintf(stderr, "real calling_scope: %s (%p)\n", fci_cache->calling_scope->name, fci_cache->calling_scope);
	} else {
		fprintf(stderr, "real calling_scope: NONE\n");
	}
	if (fci_cache->called_scope) {
		fprintf(stderr, "real called_scope: %s (%p)\n", fci_cache->called_scope->name, fci_cache->called_scope);
	} else {
		fprintf(stderr, "real called_scope: NONE\n");
	}*/
#endif

#if PHP_VERSION_ID >= 70000
	func = fci_cache->function_handler;

	call = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION,
		func, fci->param_count, fci_cache->called_scope, fci_cache->object);
	calling_scope = fci_cache->calling_scope;
	fci->object = fci_cache->object;
	if (fci->object &&
	    (!EG(objects_store).object_buckets ||
	     !IS_OBJ_VALID(EG(objects_store).object_buckets[fci->object->handle]))) {
		if (EG(current_execute_data) == &execute_data) {
			EG(current_execute_data) = execute_data.prev_execute_data;
		}
		return FAILURE;
	}
	
#else
	EX(function_state).function = fci_cache->function_handler;
	func = EX(function_state).function;
	calling_scope = fci_cache->calling_scope;
	called_scope = fci_cache->called_scope;
	fci->object_ptr = fci_cache->object_ptr;
	EX(object) = fci->object_ptr;

	if (fci->object_ptr && Z_TYPE_P(fci->object_ptr) == IS_OBJECT &&
		(!EG(objects_store).object_buckets || !EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(fci->object_ptr)].valid)) {
		return FAILURE;
	}
	
#endif

	fn_flags = func->common.fn_flags;
	if (fn_flags & (ZEND_ACC_ABSTRACT|ZEND_ACC_DEPRECATED)) {
		if (fn_flags & ZEND_ACC_ABSTRACT) {
			zend_error_noreturn(E_ERROR, "Cannot call abstract method %s::%s()", func->common.scope->name, func->common.function_name);
		}
		if (fn_flags & ZEND_ACC_DEPRECATED) {
#if PHP_VERSION_ID >= 70000
			zend_error(E_DEPRECATED, "Function %s%s%s() is deprecated",
				func->common.scope ? func->common.scope->name->val : "",
				func->common.scope ? "::" : "",
				func->common.function_name);
#else
			zend_error(E_DEPRECATED, "Function %s%s%s() is deprecated",
				func->common.scope ? func->common.scope->name : "",
				func->common.scope ? "::" : "",
				func->common.function_name);
#endif
		}
	}

#if PHP_VERSION_ID < 70000
	ZEND_VM_STACK_GROW_IF_NEEDED(fci->param_count + 1);

	for (i = 0; i < fci->param_count; i++) {
		zval *param;

		if (ARG_SHOULD_BE_SENT_BY_REF(func, i + 1)) {
			if (!PZVAL_IS_REF(*fci->params[i]) && Z_REFCOUNT_PP(fci->params[i]) > 1) {
				zval *new_zval;

				if (fci->no_separation &&
					!ARG_MAY_BE_SENT_BY_REF(EX(function_state).function, i + 1)) {
					if (i || UNEXPECTED(ZEND_VM_STACK_ELEMETS(EG(argument_stack)) == (EG(argument_stack)->top))) {
						/* hack to clean up the stack */
						zend_vm_stack_push((void *) (zend_uintptr_t)i TSRMLS_CC);
						zend_vm_stack_clear_multiple(0 TSRMLS_CC);
					}

					zend_error(E_WARNING, "Parameter %d to %s%s%s() expected to be a reference, value given",
						i+1,
						func->common.scope ? func->common.scope->name : "",
						func->common.scope ? "::" : "",
						func->common.function_name);
					return FAILURE;
				}

				ALLOC_ZVAL(new_zval);
				*new_zval = **fci->params[i];
				zval_copy_ctor(new_zval);
				Z_SET_REFCOUNT_P(new_zval, 1);
				Z_DELREF_PP(fci->params[i]);
				*fci->params[i] = new_zval;
			}
			Z_ADDREF_PP(fci->params[i]);
			Z_SET_ISREF_PP(fci->params[i]);
			param = *fci->params[i];
		} else if (PZVAL_IS_REF(*fci->params[i]) &&
				   /* don't separate references for __call */
				   (func->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) == 0 ) {
			ALLOC_ZVAL(param);
			*param = **(fci->params[i]);
			INIT_PZVAL(param);
			zval_copy_ctor(param);
		} else if (*fci->params[i] != &EG(uninitialized_zval)) {
			Z_ADDREF_PP(fci->params[i]);
			param = *fci->params[i];
		} else {
			ALLOC_ZVAL(param);
			*param = **(fci->params[i]);
			INIT_PZVAL(param);
		}
		zend_vm_stack_push(param TSRMLS_CC);
	}

	EX(function_state).arguments = zend_vm_stack_top(TSRMLS_C);
	zend_vm_stack_push((void*)(zend_uintptr_t)fci->param_count TSRMLS_CC);

	current_scope = EG(scope);
	EG(scope) = calling_scope;

	current_this = EG(This);

	current_called_scope = EG(called_scope);
	if (called_scope) {
		EG(called_scope) = called_scope;
	} else if (func->type != ZEND_INTERNAL_FUNCTION) {
		EG(called_scope) = NULL;
	}

	if (fci->object_ptr) {
		if ((EX(function_state).function->common.fn_flags & ZEND_ACC_STATIC)) {
			EG(This) = NULL;
		} else {
			EG(This) = fci->object_ptr;

			if (!PZVAL_IS_REF(EG(This))) {
				Z_ADDREF_P(EG(This)); /* For $this pointer */
			} else {
				zval *this_ptr;

				ALLOC_ZVAL(this_ptr);
				*this_ptr = *EG(This);
				INIT_PZVAL(this_ptr);
				zval_copy_ctor(this_ptr);
				EG(This) = this_ptr;
			}
		}
	} else {
		EG(This) = NULL;
	}

	EX(prev_execute_data) = EG(current_execute_data);
	EG(current_execute_data) = &execute_data;

	if (EX(function_state).function->type == ZEND_USER_FUNCTION) {

		calling_symbol_table = EG(active_symbol_table);
		EG(scope) = EX(function_state).function->common.scope;
		if (fci->symbol_table) {
			EG(active_symbol_table) = fci->symbol_table;
		} else {
			EG(active_symbol_table) = NULL;
		}

		original_return_value = EG(return_value_ptr_ptr);
		original_op_array = EG(active_op_array);
		EG(return_value_ptr_ptr) = fci->retval_ptr_ptr;
		EG(active_op_array) = (zend_op_array *) EX(function_state).function;
		original_opline_ptr = EG(opline_ptr);

		//if (EG(active_op_array)->fn_flags & ZEND_ACC_GENERATOR) {
		//	*fci->retval_ptr_ptr = zend_generator_create_zval(EG(active_op_array) TSRMLS_CC);
		//} else {
			zend_execute(EG(active_op_array) TSRMLS_CC);
		//}

		if (!fci->symbol_table && EG(active_symbol_table)) {
			zephir_clean_and_cache_symbol_table(EG(active_symbol_table) TSRMLS_CC);
		}
		EG(active_symbol_table) = calling_symbol_table;
		EG(active_op_array) = original_op_array;
		EG(return_value_ptr_ptr)=original_return_value;
		EG(opline_ptr) = original_opline_ptr;
	} else if (EX(function_state).function->type == ZEND_INTERNAL_FUNCTION) {
		int call_via_handler = (EX(function_state).function->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0;
		ALLOC_INIT_ZVAL(*fci->retval_ptr_ptr);
		if (EX(function_state).function->common.scope) {
			EG(scope) = EX(function_state).function->common.scope;
		}
		if (EXPECTED(zend_execute_internal == NULL)) {
			/* saves one function call if zend_execute_internal is not used */
			EX(function_state).function->internal_function.handler(fci->param_count, *fci->retval_ptr_ptr, fci->retval_ptr_ptr, fci->object_ptr, 1 TSRMLS_CC);
		} else {
			zend_execute_internal(&execute_data, fci, 1 TSRMLS_CC);
		}
		/*  We shouldn't fix bad extensions here,
			because it can break proper ones (Bug #34045)
		if (!EX(function_state).function->common.return_reference)
		{
			INIT_PZVAL(*fci->retval_ptr_ptr);
		}*/
		if (EG(exception) && fci->retval_ptr_ptr) {
			zval_ptr_dtor(fci->retval_ptr_ptr);
			*fci->retval_ptr_ptr = NULL;
		}

		if (call_via_handler) {
			/* We must re-initialize function again */
			fci_cache->initialized = 0;
		}
	} else { /* ZEND_OVERLOADED_FUNCTION */
		ALLOC_INIT_ZVAL(*fci->retval_ptr_ptr);

		/* Not sure what should be done here if it's a static method */
		if (fci->object_ptr) {
			Z_OBJ_HT_P(fci->object_ptr)->call_method(EX(function_state).function->common.function_name, fci->param_count, *fci->retval_ptr_ptr, fci->retval_ptr_ptr, fci->object_ptr, 1 TSRMLS_CC);
		} else {
			zend_error_noreturn(E_ERROR, "Cannot call overloaded function for non-object");
		}

		if (EX(function_state).function->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY) {
			efree((char*)EX(function_state).function->common.function_name);
		}
		efree(EX(function_state).function);

		if (EG(exception) && fci->retval_ptr_ptr) {
			zval_ptr_dtor(fci->retval_ptr_ptr);
			*fci->retval_ptr_ptr = NULL;
		}
	}
	zend_vm_stack_clear_multiple(0 TSRMLS_CC);

	if (EG(This)) {
		zval_ptr_dtor(&EG(This));
	}
	EG(called_scope) = current_called_scope;
	EG(scope) = current_scope;
	EG(This) = current_this;
	EG(current_execute_data) = EX(prev_execute_data);

	if (EG(exception)) {
		zephir_throw_exception_internal(NULL TSRMLS_CC);
	}
#else
	for (i=0; i<fci->param_count; i++) {
		zval *param;
		zval *arg = &fci->params[i];

		if (ARG_SHOULD_BE_SENT_BY_REF(func, i + 1)) {
			if (UNEXPECTED(!Z_ISREF_P(arg))) {
				if (fci->no_separation &&
					!ARG_MAY_BE_SENT_BY_REF(func, i + 1)) {
					if (i) {
						/* hack to clean up the stack */
						ZEND_CALL_NUM_ARGS(call) = i;
						zend_vm_stack_free_args(call);
					}
					zend_vm_stack_free_call_frame(call);

					zend_error(E_WARNING, "Parameter %d to %s%s%s() expected to be a reference, value given",
						i+1,
						func->common.scope ? func->common.scope->name->val : "",
						func->common.scope ? "::" : "",
						func->common.function_name->val);
					if (EG(current_execute_data) == &execute_data) {
						EG(current_execute_data) = execute_data.prev_execute_data;
					}
					return FAILURE;
				}

				ZVAL_NEW_REF(arg, arg);
			}
			Z_ADDREF_P(arg);
		} else {
			if (Z_ISREF_P(arg) &&
			    !(func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE)) {
				/* don't separate references for __call */
				arg = Z_REFVAL_P(arg);
			}
			if (Z_OPT_REFCOUNTED_P(arg)) {
				Z_ADDREF_P(arg);
			}
		}
		param = ZEND_CALL_ARG(call, i+1);
		ZVAL_COPY_VALUE(param, arg);
	}

	EG(scope) = calling_scope;
	if (func->common.fn_flags & ZEND_ACC_STATIC) {
		fci->object = NULL;
	}
	if (!fci->object) {
		Z_OBJ(call->This) = NULL;
	} else {
		Z_OBJ(call->This) = fci->object;
		GC_REFCOUNT(fci->object)++;
	}

	if (func->type == ZEND_USER_FUNCTION) {
		int call_via_handler = (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) != 0;
		EG(scope) = func->common.scope;
		call->symbol_table = fci->symbol_table;
		if (UNEXPECTED(func->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
			ZEND_ASSERT(GC_TYPE(func->op_array.prototype) == IS_OBJECT);
			GC_REFCOUNT(func->op_array.prototype)++;
			ZEND_ADD_CALL_FLAG(call, ZEND_CALL_CLOSURE);
		}
		if (EXPECTED((func->op_array.fn_flags & ZEND_ACC_GENERATOR) == 0)) {
			zend_init_execute_data(call, &func->op_array, fci->retval);
			zend_execute_ex(call);
		} else {
			// zend_generator_create_zval(call, &func->op_array, fci->retval);
			zend_error(E_EXCEPTION | E_ERROR, "Zephir: Generators not supported");
		}
		if (call_via_handler) {
			/* We must re-initialize function again */
			fci_cache->initialized = 0;
		}
	} else if (func->type == ZEND_INTERNAL_FUNCTION) {
		int call_via_handler = (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) != 0;
		ZVAL_NULL(fci->retval);
		if (func->common.scope) {
			EG(scope) = func->common.scope;
		}
		call->prev_execute_data = EG(current_execute_data);
		call->return_value = NULL; /* this is not a constructor call */
		EG(current_execute_data) = call;
		if (EXPECTED(zend_execute_internal == NULL)) {
			/* saves one function call if zend_execute_internal is not used */
			func->internal_function.handler(call, fci->retval);
		} else {
			zend_execute_internal(call, fci->retval);
		}
		EG(current_execute_data) = call->prev_execute_data;
		zend_vm_stack_free_args(call);
		zend_vm_stack_free_call_frame(call);

		/*  We shouldn't fix bad extensions here,
			because it can break proper ones (Bug #34045)
		if (!EX(function_state).function->common.return_reference)
		{
			INIT_PZVAL(f->retval);
		}*/
		if (EG(exception)) {
			zval_ptr_dtor(fci->retval);
			ZVAL_UNDEF(fci->retval);
		}

		if (call_via_handler) {
			/* We must re-initialize function again */
			fci_cache->initialized = 0;
		}
	} else { /* ZEND_OVERLOADED_FUNCTION */
		ZVAL_NULL(fci->retval);

		/* Not sure what should be done here if it's a static method */
		if (fci->object) {
			call->prev_execute_data = EG(current_execute_data);
			EG(current_execute_data) = call;
			fci->object->handlers->call_method(func->common.function_name, fci->object, call, fci->retval);
			EG(current_execute_data) = call->prev_execute_data;
		} else {
			zend_error(E_EXCEPTION | E_ERROR, "Cannot call overloaded function for non-object");
		}

		zend_vm_stack_free_args(call);
		zend_vm_stack_free_call_frame(call);

		if (func->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY) {
			zend_string_release(func->common.function_name);
		}
		efree(func);

		if (EG(exception)) {
			zval_ptr_dtor(fci->retval);
			ZVAL_UNDEF(fci->retval);
		}
	}

	if (fci->object) {
		OBJ_RELEASE(fci->object);
	}

	EG(scope) = orig_scope;
	if (EG(current_execute_data) == &execute_data) {
		EG(current_execute_data) = execute_data.prev_execute_data;
	}
#endif
	return SUCCESS;
}

#endif
