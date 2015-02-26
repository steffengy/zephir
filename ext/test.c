
/* This file was generated automatically by Zephir do not modify it! */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#if PHP_VERSION_ID < 50500
#include <locale.h>
#endif

#include "php_ext.h"
#include "test.h"

#include <ext/standard/info.h>

#include <Zend/zend_operators.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/memory.h"



zend_class_entry *test_methodinterface_ce;
zend_class_entry *test_ooimpl_zbeginning_ce;
zend_class_entry *test_diinterface_ce;
zend_class_entry *test_extendedinterface_ce;
zend_class_entry *test_ooimpl_abeginning_ce;
zend_class_entry *test_testinterface_ce;
zend_class_entry *test_constantsparent_ce;
zend_class_entry *test_properties_publicproperties_ce;
zend_class_entry *test_0__closure_ce;
zend_class_entry *test_1__closure_ce;
zend_class_entry *test_2__closure_ce;
zend_class_entry *test_3__closure_ce;
zend_class_entry *test_4__closure_ce;
zend_class_entry *test_5__closure_ce;
zend_class_entry *test_6__closure_ce;
zend_class_entry *test_arithmetic_ce;
zend_class_entry *test_arrayobject_ce;
zend_class_entry *test_bitwise_ce;
zend_class_entry *test_branchprediction_ce;
zend_class_entry *test_cast_ce;
zend_class_entry *test_cblock_ce;
zend_class_entry *test_closures_ce;
zend_class_entry *test_concat_ce;
zend_class_entry *test_constants_ce;
zend_class_entry *test_declaretest_ce;
zend_class_entry *test_echoes_ce;
zend_class_entry *test_evaltest_ce;
zend_class_entry *test_exception_ce;
zend_class_entry *test_exceptions_ce;
zend_class_entry *test_exists_ce;
zend_class_entry *test_exitdie_ce;
zend_class_entry *test_factorial_ce;
zend_class_entry *test_fannkuch_ce;
zend_class_entry *test_fcall_ce;
zend_class_entry *test_fibonnaci_ce;
zend_class_entry *test_fortytwo_ce;
zend_class_entry *test_instanceoff_ce;
zend_class_entry *test_internalclasses_ce;
zend_class_entry *test_internalinterfaces_ce;
zend_class_entry *test_issettest_ce;
zend_class_entry *test_issues_ce;
zend_class_entry *test_json_ce;
zend_class_entry *test_logical_ce;
zend_class_entry *test_methodabstract_ce;
zend_class_entry *test_optimizers_arraymerge_ce;
zend_class_entry *test_properties_extendspublicproperties_ce;
zend_class_entry *test_properties_privateproperties_ce;
zend_class_entry *test_properties_propertyarray_ce;
zend_class_entry *test_properties_protectedproperties_ce;
zend_class_entry *test_properties_staticprotectedproperties_ce;
zend_class_entry *test_properties_staticpublicproperties_ce;
zend_class_entry *test_range_ce;
zend_class_entry *test_references_ce;
zend_class_entry *test_regexdna_ce;
zend_class_entry *test_requires_ce;
zend_class_entry *test_resourcetest_ce;
zend_class_entry *test_returns_ce;
zend_class_entry *test_router_exception_ce;
zend_class_entry *test_scallparent_ce;
zend_class_entry *test_spectralnorm_ce;
zend_class_entry *test_strings_ce;
zend_class_entry *test_ternary_ce;
zend_class_entry *test_trie_ce;
zend_class_entry *test_trytest_ce;
zend_class_entry *test_typeoff_ce;
zend_class_entry *test_unknownclass_ce;
zend_class_entry *test_vars_ce;

ZEND_DECLARE_MODULE_GLOBALS(test)

static PHP_MINIT_FUNCTION(test)
{
#if PHP_VERSION_ID < 50500
	char* old_lc_all = setlocale(LC_ALL, NULL);
	if (old_lc_all) {
		size_t len = strlen(old_lc_all);
		char *tmp  = calloc(len+1, 1);
		if (UNEXPECTED(!tmp)) {
			return FAILURE;
		}

		memcpy(tmp, old_lc_all, len);
		old_lc_all = tmp;
	}

	setlocale(LC_ALL, "C");
#endif

	ZEPHIR_INIT(Test_MethodInterface);
	ZEPHIR_INIT(Test_OoImpl_ZBeginning);
	ZEPHIR_INIT(Test_DiInterface);
	ZEPHIR_INIT(Test_ExtendedInterface);
	ZEPHIR_INIT(Test_OoImpl_ABeginning);
	ZEPHIR_INIT(Test_TestInterface);
	ZEPHIR_INIT(Test_ConstantsParent);
	ZEPHIR_INIT(Test_Properties_PublicProperties);
	ZEPHIR_INIT(Test_Arithmetic);
	ZEPHIR_INIT(Test_ArrayObject);
	ZEPHIR_INIT(Test_Bitwise);
	ZEPHIR_INIT(Test_BranchPrediction);
	ZEPHIR_INIT(Test_Cast);
	ZEPHIR_INIT(Test_Cblock);
	ZEPHIR_INIT(Test_Closures);
	ZEPHIR_INIT(Test_Concat);
	ZEPHIR_INIT(Test_Constants);
	ZEPHIR_INIT(Test_DeclareTest);
	ZEPHIR_INIT(Test_Echoes);
	ZEPHIR_INIT(Test_EvalTest);
	ZEPHIR_INIT(Test_Exception);
	ZEPHIR_INIT(Test_Exceptions);
	ZEPHIR_INIT(Test_Exists);
	ZEPHIR_INIT(Test_ExitDie);
	ZEPHIR_INIT(Test_Factorial);
	ZEPHIR_INIT(Test_Fannkuch);
	ZEPHIR_INIT(Test_Fcall);
	ZEPHIR_INIT(Test_Fibonnaci);
	ZEPHIR_INIT(Test_FortyTwo);
	ZEPHIR_INIT(Test_Instanceoff);
	ZEPHIR_INIT(Test_InternalClasses);
	ZEPHIR_INIT(Test_InternalInterfaces);
	ZEPHIR_INIT(Test_IssetTest);
	ZEPHIR_INIT(Test_Issues);
	ZEPHIR_INIT(Test_Json);
	ZEPHIR_INIT(Test_Logical);
	ZEPHIR_INIT(Test_MethodAbstract);
	ZEPHIR_INIT(Test_Optimizers_ArrayMerge);
	ZEPHIR_INIT(Test_Properties_ExtendsPublicProperties);
	ZEPHIR_INIT(Test_Properties_PrivateProperties);
	ZEPHIR_INIT(Test_Properties_PropertyArray);
	ZEPHIR_INIT(Test_Properties_ProtectedProperties);
	ZEPHIR_INIT(Test_Properties_StaticProtectedProperties);
	ZEPHIR_INIT(Test_Properties_StaticPublicProperties);
	ZEPHIR_INIT(Test_Range);
	ZEPHIR_INIT(Test_References);
	ZEPHIR_INIT(Test_RegexDNA);
	ZEPHIR_INIT(Test_Requires);
	ZEPHIR_INIT(Test_ResourceTest);
	ZEPHIR_INIT(Test_Returns);
	ZEPHIR_INIT(Test_Router_Exception);
	ZEPHIR_INIT(Test_ScallParent);
	ZEPHIR_INIT(Test_SpectralNorm);
	ZEPHIR_INIT(Test_Strings);
	ZEPHIR_INIT(Test_Ternary);
	ZEPHIR_INIT(Test_Trie);
	ZEPHIR_INIT(Test_TryTest);
	ZEPHIR_INIT(Test_Typeoff);
	ZEPHIR_INIT(Test_UnknownClass);
	ZEPHIR_INIT(Test_Vars);
	ZEPHIR_INIT(test_0__closure);
	ZEPHIR_INIT(test_1__closure);
	ZEPHIR_INIT(test_2__closure);
	ZEPHIR_INIT(test_3__closure);
	ZEPHIR_INIT(test_4__closure);
	ZEPHIR_INIT(test_5__closure);
	ZEPHIR_INIT(test_6__closure);

#if PHP_VERSION_ID < 50500
	setlocale(LC_ALL, old_lc_all);
	free(old_lc_all);
#endif
	return SUCCESS;
}

#ifndef ZEPHIR_RELEASE
static PHP_MSHUTDOWN_FUNCTION(test)
{

	zephir_deinitialize_memory();
	return SUCCESS;
}
#endif

/**
 * Initialize globals on each request or each thread started
 */
static void php_zephir_init_globals(zend_test_globals *zephir_globals TSRMLS_DC)
{
	zephir_globals->initialized = 0;

	/* Memory options */
	zephir_globals->active_memory = NULL;

	/* Virtual Symbol Tables */
	zephir_globals->active_symbol_table = NULL;

	/* Cache Enabled */
	zephir_globals->cache_enabled = 1;

	/* Recursive Lock */
	zephir_globals->recursive_lock = 0;

	zephir_globals->test.my_setting_1 = 1;
	zephir_globals->test.my_setting_2 = 100;
	zephir_globals->test.my_setting_3 = 7.5;
	zephir_globals->my_setting_1 = 1;
	zephir_globals->my_setting_2 = 10;
	zephir_globals->my_setting_3 = 15.2;

}

static PHP_RINIT_FUNCTION(test)
{

	zend_test_globals *zephir_globals_ptr = ZEPHIR_VGLOBAL;

	php_zephir_init_globals(zephir_globals_ptr TSRMLS_CC);
	//zephir_init_interned_strings(TSRMLS_C);

	zephir_initialize_memory(zephir_globals_ptr TSRMLS_CC);

	return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(test)
{

	
	zephir_deinitialize_memory(TSRMLS_C);
	return SUCCESS;
}

static PHP_MINFO_FUNCTION(test)
{
	php_info_print_box_start(0);
	php_printf("%s", PHP_TEST_DESCRIPTION);
	php_info_print_box_end();

	php_info_print_table_start();
	php_info_print_table_header(2, PHP_TEST_NAME, "enabled");
	php_info_print_table_row(2, "Author", PHP_TEST_AUTHOR);
	php_info_print_table_row(2, "Version", PHP_TEST_VERSION);
	php_info_print_table_row(2, "Powered by Zephir", "Version " PHP_TEST_ZEPVERSION);
	php_info_print_table_end();

	php_info_print_table_start();
	php_info_print_table_header(2, "Directive", "Value");
	php_info_print_table_row(2, "setting1", "value1");
	php_info_print_table_row(2, "setting2", "value2");
	php_info_print_table_end();
	php_info_print_table_start();
	php_info_print_table_header(2, "Directive", "Value");
	php_info_print_table_row(2, "setting3", "value3");
	php_info_print_table_row(2, "setting4", "value4");
	php_info_print_table_end();

}

static PHP_GINIT_FUNCTION(test)
{
	php_zephir_init_globals(test_globals TSRMLS_CC);
}

static PHP_GSHUTDOWN_FUNCTION(test)
{

}

zend_module_entry test_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	NULL,
	PHP_TEST_EXTNAME,
	NULL,
	PHP_MINIT(test),
#ifndef ZEPHIR_RELEASE
	PHP_MSHUTDOWN(test),
#else
	NULL,
#endif
	PHP_RINIT(test),
	PHP_RSHUTDOWN(test),
	PHP_MINFO(test),
	PHP_TEST_VERSION,
	ZEND_MODULE_GLOBALS(test),
	PHP_GINIT(test),
	PHP_GSHUTDOWN(test),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_TEST
ZEND_GET_MODULE(test)
#endif
