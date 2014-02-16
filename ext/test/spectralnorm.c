
#ifdef HAVE_CONFIG_H
#include "../ext_config.h"
#endif

#include <php.h>
#include "../php_ext.h"
#include "../ext.h"

#include <Zend/zend_operators.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"


/**
 * SpectralNorm
 *
 * @see http://mathworld.wolfram.com/SpectralNorm.html
 */
ZEPHIR_INIT_CLASS(Test_SpectralNorm) {

	ZEPHIR_REGISTER_CLASS(Test, SpectralNorm, test, spectralnorm, test_spectralnorm_method_entry, 0);


	return SUCCESS;

}

PHP_METHOD(Test_SpectralNorm, Ax) {

	zval *i, *j, *_0, _1;

	ZEPHIR_MM_GROW();
	zephir_fetch_params(1, 2, 0, &i, &j);



	ZEPHIR_INIT_VAR(_0);
	zephir_add_function(_0, i, j TSRMLS_CC);
	ZEPHIR_SINIT_VAR(_1);
	zephir_add_function(&_1, i, j TSRMLS_CC);
	RETURN_MM_LONG((1 / ((((zephir_get_numberval(_0) * (zephir_get_numberval(&_1) + 1)) / 2) + zephir_get_numberval(i)) + 1)));

}

PHP_METHOD(Test_SpectralNorm, Au) {

	zend_function *_9 = NULL;
	zend_bool _0, _3;
	zval *n_param = NULL, *u, *v, *_6 = NULL, *_7 = NULL, *_8 = NULL, *_10 = NULL;
	int n, t, i, j, _1, _2, _4, _5;

	ZEPHIR_MM_GROW();
	zephir_fetch_params(1, 3, 0, &n_param, &u, &v);

	n = zephir_get_intval(n_param);


	_2 = (n - 1);
	_1 = 0;
	_0 = 0;
	if ((_1 <= _2)) {
		while (1) {
			if (_0) {
				_1++;
				if (!((_1 <= _2))) {
					break;
				}
			} else {
				_0 = 1;
			}
			i = _1;
			t = 0;
			_5 = (n - 1);
			_4 = 0;
			_3 = 0;
			if ((_4 <= _5)) {
				while (1) {
					if (_3) {
						_4++;
						if (!((_4 <= _5))) {
							break;
						}
					} else {
						_3 = 1;
					}
					j = _4;
					ZEPHIR_INIT_NVAR(_6);
					ZEPHIR_INIT_NVAR(_7);
					ZVAL_LONG(_7, i);
					ZEPHIR_INIT_NVAR(_8);
					ZVAL_LONG(_8, j);
					zephir_call_method_p2_cache(_6, this_ptr, "ax", &_9, _7, _8);
					ZEPHIR_INIT_NVAR(_7);
					ZEPHIR_INIT_NVAR(_8);
					ZVAL_LONG(_8, j);
					zephir_call_method_p1(_7, u, "offsetge", _8);
					ZEPHIR_INIT_LNVAR(_10);
					mul_function(_10, _6, _7 TSRMLS_CC);
					t += zephir_get_numberval(_10);
				}
			}
			ZEPHIR_INIT_NVAR(_6);
			ZVAL_LONG(_6, i);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, t);
			zephir_call_method_p2_noret(v, "offsetset", _6, _7);
		}
	}
	ZEPHIR_MM_RESTORE();

}

PHP_METHOD(Test_SpectralNorm, Atu) {

	zend_function *_9 = NULL;
	zend_bool _0, _3;
	zval *n_param = NULL, *u, *v, *_6 = NULL, *_7 = NULL, *_8 = NULL, *_10 = NULL;
	int n, t, i, j, _1, _2, _4, _5;

	ZEPHIR_MM_GROW();
	zephir_fetch_params(1, 3, 0, &n_param, &u, &v);

	n = zephir_get_intval(n_param);


	_2 = (n - 1);
	_1 = 0;
	_0 = 0;
	if ((_1 <= _2)) {
		while (1) {
			if (_0) {
				_1++;
				if (!((_1 <= _2))) {
					break;
				}
			} else {
				_0 = 1;
			}
			i = _1;
			t = 0;
			_5 = (n - 1);
			_4 = 0;
			_3 = 0;
			if ((_4 <= _5)) {
				while (1) {
					if (_3) {
						_4++;
						if (!((_4 <= _5))) {
							break;
						}
					} else {
						_3 = 1;
					}
					j = _4;
					ZEPHIR_INIT_NVAR(_6);
					ZEPHIR_INIT_NVAR(_7);
					ZVAL_LONG(_7, j);
					ZEPHIR_INIT_NVAR(_8);
					ZVAL_LONG(_8, i);
					zephir_call_method_p2_cache(_6, this_ptr, "ax", &_9, _7, _8);
					ZEPHIR_INIT_NVAR(_7);
					ZEPHIR_INIT_NVAR(_8);
					ZVAL_LONG(_8, j);
					zephir_call_method_p1(_7, u, "offsetget", _8);
					ZEPHIR_INIT_LNVAR(_10);
					mul_function(_10, _6, _7 TSRMLS_CC);
					t += zephir_get_numberval(_10);
				}
			}
			ZEPHIR_INIT_NVAR(_6);
			ZVAL_LONG(_6, i);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, t);
			zephir_call_method_p2_noret(v, "offsetset", _6, _7);
		}
	}
	ZEPHIR_MM_RESTORE();

}

PHP_METHOD(Test_SpectralNorm, AtAu) {

	zval *n, *u, *v, *w;

	ZEPHIR_MM_GROW();
	zephir_fetch_params(1, 4, 0, &n, &u, &v, &w);



	zephir_call_method_p3_noret(this_ptr, "au", n, u, w);
	zephir_call_method_p3_noret(this_ptr, "atu", n, w, v);
	ZEPHIR_MM_RESTORE();

}

PHP_METHOD(Test_SpectralNorm, process) {

	zend_function *_11 = NULL, *_12 = NULL;
	zend_bool _4, _8, _13;
	zend_class_entry *_0, *_2, *_3;
	zval *n_param = NULL, *u, *v, *w, *_1 = NULL, *_7 = NULL, *_16 = NULL, *_17 = NULL, *_18 = NULL, _19;
	int n, i, vv = 0, vBv = 0, _5, _6, _9, _10, _14, _15;

	ZEPHIR_MM_GROW();
	zephir_fetch_params(1, 1, 0, &n_param);

	n = zephir_get_intval(n_param);


	ZEPHIR_INIT_VAR(u);
	_0 = zend_fetch_class(SL("SplFixedArray"), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
	object_init_ex(u, _0);
	ZEPHIR_INIT_VAR(_1);
	ZVAL_LONG(_1, n);
	zephir_call_method_p1_noret(u, "__construct", _1);
	ZEPHIR_INIT_VAR(v);
	_2 = zend_fetch_class(SL("SplFixedArray"), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
	object_init_ex(v, _2);
	ZEPHIR_INIT_BNVAR(_1);
	ZVAL_LONG(_1, n);
	zephir_call_method_p1_noret(v, "__construct", _1);
	ZEPHIR_INIT_VAR(w);
	_3 = zend_fetch_class(SL("SplFixedArray"), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
	object_init_ex(w, _3);
	ZEPHIR_INIT_BNVAR(_1);
	ZVAL_LONG(_1, n);
	zephir_call_method_p1_noret(w, "__construct", _1);
	_6 = (n - 1);
	_5 = 0;
	_4 = 0;
	if ((_5 <= _6)) {
		while (1) {
			if (_4) {
				_5++;
				if (!((_5 <= _6))) {
					break;
				}
			} else {
				_4 = 1;
			}
			i = _5;
			ZEPHIR_INIT_NVAR(_1);
			ZVAL_LONG(_1, i);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, 1);
			zephir_call_method_p2_noret(u, "offsetset", _1, _7);
			ZEPHIR_INIT_NVAR(_1);
			ZVAL_LONG(_1, i);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, 1);
			zephir_call_method_p2_noret(v, "offsetset", _1, _7);
			ZEPHIR_INIT_NVAR(_1);
			ZVAL_LONG(_1, i);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, 1);
			zephir_call_method_p2_noret(w, "offsetset", _1, _7);
		}
	}
	_10 = 9;
	_9 = 0;
	_8 = 0;
	if ((_9 <= _10)) {
		while (1) {
			if (_8) {
				_9++;
				if (!((_9 <= _10))) {
					break;
				}
			} else {
				_8 = 1;
			}
			i = _9;
			ZEPHIR_INIT_NVAR(_1);
			ZVAL_LONG(_1, n);
			zephir_call_method_p4_cache_noret(this_ptr, "atau", &_11, _1, u, v, w);
			ZEPHIR_INIT_NVAR(_1);
			ZVAL_LONG(_1, n);
			zephir_call_method_p4_cache_noret(this_ptr, "atau", &_12, _1, v, u, w);
		}
	}
	_15 = (n - 1);
	_14 = 0;
	_13 = 0;
	if ((_14 <= _15)) {
		while (1) {
			if (_13) {
				_14++;
				if (!((_14 <= _15))) {
					break;
				}
			} else {
				_13 = 1;
			}
			i = _14;
			ZEPHIR_INIT_NVAR(_1);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, i);
			zephir_call_method_p1(_1, u, "offsetget", _7);
			ZEPHIR_INIT_NVAR(_7);
			ZEPHIR_INIT_NVAR(_16);
			ZVAL_LONG(_16, i);
			zephir_call_method_p1(_7, v, "offsetget", _16);
			ZEPHIR_INIT_LNVAR(_17);
			mul_function(_17, _1, _7 TSRMLS_CC);
			vBv += zephir_get_numberval(_17);
			ZEPHIR_INIT_NVAR(_1);
			ZEPHIR_INIT_NVAR(_7);
			ZVAL_LONG(_7, i);
			zephir_call_method_p1(_1, v, "offsetget", _7);
			ZEPHIR_INIT_NVAR(_7);
			ZEPHIR_INIT_NVAR(_16);
			ZVAL_LONG(_16, i);
			zephir_call_method_p1(_7, v, "offsetget", _16);
			ZEPHIR_INIT_LNVAR(_18);
			mul_function(_18, _1, _7 TSRMLS_CC);
			vv += zephir_get_numberval(_18);
		}
	}
	ZEPHIR_SINIT_VAR(_19);
	ZVAL_LONG(&_19, (vBv / vv));
	zephir_call_func_p1(return_value, "sqrt", &_19);
	RETURN_MM();

}

