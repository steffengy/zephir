<?php

/*
 +--------------------------------------------------------------------------+
 | Zephir Language                                                          |
 +--------------------------------------------------------------------------+
 | Copyright (c) 2013-2015 Zephir Team and contributors                     |
 +--------------------------------------------------------------------------+
 | This source file is subject the MIT license, that is bundled with        |
 | this package in the file LICENSE, and is available through the           |
 | world-wide-web at the following url:                                     |
 | http://zephir-lang.com/license.html                                      |
 |                                                                          |
 | If you did not receive a copy of the MIT license and are unable          |
 | to obtain it through the world-wide-web, please send a note to           |
 | license@zephir-lang.com so we can mail you a copy immediately.           |
 +--------------------------------------------------------------------------+
*/

namespace Zephir;

/**
 * Class StringsManager
 *
 * Manages the concatenation keys for the extension and the interned strings
 * TODO: Rewrite thís...
 */
class StringsManager
{
    /**
     * List of headers
     * @var array
     */
    protected $_concatKeys = array(
        'vv' => true,
        'vs' => true,
        'sv' => true
    );

    /**
    * @param string $path
    */
    public function addConcatKey($key)
    {
        $this->_concatKeys[$key] = true;
    }

    /**
     * @return array
     */
    public function genConcatCode()
    {

        $code = '
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ext.h"
#include "ext/standard/php_string.h"
#include "ext.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/concat.h"' . PHP_EOL . PHP_EOL;

        $codeh = '';

        $macros = array();
        ksort($this->_concatKeys, SORT_STRING);
        foreach ($this->_concatKeys as $key => $one) {

            $len = strlen($key);
            $params = array();
            $zvalCopy = array();
            $useCopy = array();
            $avars = array();
            $zvars = array();
            $svars = array();
            $lengths = array();
            $sparams = array();
            $lparams = array();
            for ($i = 0; $i < $len; $i++) {
                $n = $i + 1;
                $t = substr($key, $i, 1);
                $sparams[] = 'op' . $n;
                if ($t == 's') {
                    $params[] = 'const char *op' . $n . ', uint32_t op' . $n . '_len';
                    $lparams[] = 'op' . $n . ', sizeof(op' . $n . ')-1';
                    $lengths[] = 'op' . $n . '_len';
                    $svars[] = $n;
                    $avars[$n] = 's';
                } else {
                    $params[] = 'zval *op' . $n;
                    $lparams[] = 'op' . $n;
                    $zvalCopy[] = 'op' . $n . '_copy';
                    $useCopy[] = 'use_copy' . $n . ' = 0';
                    $lengths[] = 'Z_STRLEN_P(op' . $n . ')';
                    $zvars[] = $n;
                    $avars[$n] = 'v';
                }
            }

            $macros[] = '#define ZEPHIR_CONCAT_' . strtoupper($key) . '(result, ' . join(', ', $sparams) . ') \\' . PHP_EOL . "\t" . ' zephir_concat_' . $key . '(result, ' . join(', ', $lparams) . ', 0);';
            $macros[] = '#define ZEPHIR_SCONCAT_' . strtoupper($key) . '(result, ' . join(', ', $sparams) . ') \\' . PHP_EOL . "\t" . ' zephir_concat_' . $key . '(result, ' . join(', ', $lparams) . ', 1);';
            $macros[] = '';

            $proto = 'void zephir_concat_' . $key . '(zval *result, ' . join(', ', $params) . ', int self_var)';
            $proto = 'void zephir_concat_' . $key . '(zval *result, ' . join(', ', $params) . ', int self_var)';

            $codeh.= '' . $proto . ';' . PHP_EOL;

            $code .= $proto . '{' . PHP_EOL . PHP_EOL;
            
            if (count($zvalCopy)) {
                $code .= "\t" . 'zval result_copy, ' . join(', ', $zvalCopy) . ';' . PHP_EOL;
                $code .= "\t" . 'int use_copy = 0, ' . join(', ', $useCopy) . ';' . PHP_EOL;
            } else {
                $code .= "\t" . 'zval result_copy;' . PHP_EOL;
                $code .= "\t" . 'int use_copy = 0;' . PHP_EOL;
            }
            $code .= "\t" . 'zend_string *tmp_str;' . PHP_EOL;
            $code .= "\t" . 'uint offset = 0, length;' . PHP_EOL . PHP_EOL;

            foreach ($zvars as $zvar) {
                $code .= "\t" . 'if (Z_TYPE_P(op' . $zvar . ') != IS_STRING) {' . PHP_EOL;
                $code .= "\t" . '   use_copy' . $zvar . ' = zend_make_printable_zval(op' . $zvar . ', &op' . $zvar . '_copy);' . PHP_EOL;
                $code .= "\t" . '   if (use_copy' . $zvar . ') {' . PHP_EOL;
                $code .= "\t" . '       op' . $zvar . ' = &op' . $zvar . '_copy;' . PHP_EOL;
                $code .= "\t" . '   }' . PHP_EOL;
                $code .= "\t" . '}' . PHP_EOL . PHP_EOL;
            }

            $code .= "\t" . 'length = ' . join(' + ', $lengths) . ';' . PHP_EOL;
            
            $code .= "\t" . 'if (self_var) {' . PHP_EOL;
            $code .= "\t\t" . 'if (Z_TYPE_P(result) != IS_STRING) {' . PHP_EOL;
            $code .= "\t\t\t" . 'use_copy = zend_make_printable_zval(result, &result_copy);' . PHP_EOL;
            $code .= "\t\t\t" . 'if (use_copy) {' . PHP_EOL;
            $code .= "\t\t\t\t" .'ZEPHIR_CPY_WRT_CTOR(result, &result_copy);' . PHP_EOL;
            $code .= "\t\t\t" .'}'. PHP_EOL;
            $code .= "\t\t" . '} '. PHP_EOL;
            $code .= "\t" . '}' . PHP_EOL . PHP_EOL;
            
            $varTypes = array();
            $varOps = array();
            foreach ($avars as $n => $type) {
                if ($type == 's') {
                    $varTypes[] = '%s';
                    $varOps[] = 'op' . $n;
                } else {
                    $varTypes[] = '%s';
                    $varOps[] = 'Z_STRVAL_P(op' . $n . ')';
                }
            }
            
            $code .= "\t" . 'if (self_var) {' . PHP_EOL;
            $code .= "\t\t" . 'tmp_str = strpprintf(length + Z_STRLEN_P(result), "%s' . implode('', $varTypes) . '", Z_STRVAL_P(result), ' . implode(', ', $varOps) . ');' . PHP_EOL;
            $code .= "\t" . '} else {' . PHP_EOL;
            $code .= "\t\t" . 'tmp_str = strpprintf(length, "' . implode('', $varTypes) . '", ' . implode(', ', $varOps) . ');' . PHP_EOL;
            $code .= "\t" . '}' . PHP_EOL;
            $code .= "\t" . 'ZVAL_NEW_STR(result, tmp_str);' . PHP_EOL;
            $code .= "\t" . 'zend_string_release(tmp_str);' . PHP_EOL;

            foreach ($zvars as $zvar) {
                $code .= "\t" . 'if (use_copy' . $zvar . ') {' . PHP_EOL;
                $code .= "\t" . '   zval_dtor(op' . $zvar . ');' . PHP_EOL;
                $code .= "\t" . '}' . PHP_EOL . PHP_EOL;
            }

            $code .= "\t" . 'if (use_copy) {' . PHP_EOL;
            $code .= "\t" . '   zval_dtor(&result_copy);' . PHP_EOL;
            $code .= "\t" . '}' . PHP_EOL . PHP_EOL;

            $code .= "}" . PHP_EOL . PHP_EOL;
        }

        $code .= <<<EOF
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
EOF;

        $codeh .= "void zephir_concat_function(zval *result, zval *op1, zval *op2);";
        Utils::checkAndWriteIfNeeded(join(PHP_EOL, $macros) . PHP_EOL . PHP_EOL . $codeh, 'ext/kernel/concat.h');
        Utils::checkAndWriteIfNeeded($code, 'ext/kernel/concat.c');
    }

    public function getConcatKeys()
    {
        return $this->_concatKeys;
    }
}
