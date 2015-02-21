#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <ctype.h>

#include <php.h>
#include "php_ext.h"
#include "php_main.h"

#include <ext/standard/php_smart_string.h>
#include <ext/standard/php_string.h>
#include <ext/standard/php_rand.h>
#include <ext/standard/php_lcg.h>
#include <ext/standard/php_http.h>
#include "ext/standard/base64.h"
#include "ext/standard/md5.h"
#include "ext/standard/url.h"
#include "ext/standard/html.h"
#include "ext/date/php_date.h"

#ifdef ZEPHIR_USE_PHP_PCRE
#include <ext/pcre/php_pcre.h>
#endif

#include <ext/json/php_json.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"

#define PH_RANDOM_ALNUM 0
#define PH_RANDOM_ALPHA 1
#define PH_RANDOM_HEXDEC 2
#define PH_RANDOM_NUMERIC 3
#define PH_RANDOM_NOZERO 4

int zephir_json_encode(zval *return_value, zval *v, int opts)
{
	smart_str buf = { 0 };

	php_json_encode(&buf, v, opts);
	smart_str_0(&buf);
	ZVAL_STR(return_value, buf.s);
	//TODO: missing free of buf? check memleak

	return SUCCESS;
}

int zephir_json_decode(zval *return_value, zval *v, zend_bool assoc)
{
	zval copy;
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(v) != IS_STRING)) {
		use_copy = zend_make_printable_zval(v, &copy);
		if (use_copy) {
			v = &copy;
		}
	}
	php_json_decode(return_value, Z_STRVAL_P(v), Z_STRLEN_P(v), assoc, 512 /* JSON_PARSER_DEFAULT_DEPTH */);
	if (unlikely(use_copy)) {
		zval_dtor(&copy);
	}

	return SUCCESS;
}
