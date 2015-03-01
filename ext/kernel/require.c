
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
  |          Eduar Carvajal <eduar@zephir-lang.com>                        |
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"
#include "kernel/require.h"
//#include "kernel/backtrace.h"

#include <main/php_main.h>
#include <Zend/zend_exceptions.h>

#ifndef ENFORCE_SAFE_MODE
#define ENFORCE_SAFE_MODE    0
#endif

/**
 * Do an internal require to a plain php file taking care of the value returned by the file
 */
int zephir_require_ret(zval *return_value_ptr, const char *require_path)
{
	zend_bool ret;
	zend_file_handle file_handle;
	zend_op_array *retval;
	char *opened_path = NULL;

	file_handle.filename = require_path;
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	retval = zend_compile_file(&file_handle, ZEND_REQUIRE);
	if (retval && file_handle.handle.stream.handle) {
		if (!file_handle.opened_path) {
			file_handle.opened_path = opened_path = estrdup(require_path);
		}

		zend_hash_str_add_empty_element(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path));

		if (opened_path) {
			efree(opened_path);
		}
		zend_execute(retval, return_value_ptr);
		zend_exception_restore();
		destroy_op_array(retval);
		efree(retval);
		if (EG(exception)) {
			assert(!return_value_ptr);
			ret = FAILURE;
		} else {
			ret = SUCCESS;
		}
	}
	zend_destroy_file_handle(&file_handle);

	return ret;
}

