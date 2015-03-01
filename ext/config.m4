PHP_ARG_ENABLE(test, whether to enable test, [ --enable-test   Enable Test])

if test "$PHP_TEST" = "yes"; then

	

	if ! test "x" = "x"; then
		PHP_EVAL_LIBLINE(, TEST_SHARED_LIBADD)
	fi

	AC_DEFINE(HAVE_TEST, 1, [Whether you have Test])
	test_sources="test.c kernel/main.c kernel/memory.c kernel/exception.c kernel/hash.c kernel/debug.c kernel/backtrace.c kernel/object.c kernel/array.c kernel/extended/array.c kernel/string.c kernel/fcall.c kernel/require.c kernel/file.c kernel/operators.c kernel/concat.c kernel/variables.c kernel/filter.c kernel/iterator.c kernel/exit.c test\arithmetic.zep.c
	test\arrayobject.zep.c
	test\assign.zep.c
	test\bitwise.zep.c
	test\branchprediction.zep.c
	test\cast.zep.c
	test\cblock.zep.c
	test\closures.zep.c
	test\compare.zep.c
	test\concat.zep.c
	test\constants.zep.c
	test\constantsparent.zep.c
	test\declaretest.zep.c
	test\diinterface.zep.c
	test\echoes.zep.c
	test\emptytest.zep.c
	test\evaltest.zep.c
	test\exception.zep.c
	test\exceptions.zep.c
	test\exists.zep.c
	test\exitdie.zep.c
	test\extendedinterface.zep.c
	test\factorial.zep.c
	test\fannkuch.zep.c
	test\fcall.zep.c
	test\fibonnaci.zep.c
	test\fortytwo.zep.c
	test\instanceoff.zep.c
	test\internalclasses.zep.c
	test\internalinterfaces.zep.c
	test\issettest.zep.c
	test\issues.zep.c
	test\json.zep.c
	test\logical.zep.c
	test\methodabstract.zep.c
	test\methodinterface.zep.c
	test\nativearray.zep.c
	test\ooimpl\abeginning.zep.c
	test\ooimpl\zbeginning.zep.c
	test\operator.zep.c
	test\optimizers\arraymerge.zep.c
	test\properties\extendspublicproperties.zep.c
	test\properties\privateproperties.zep.c
	test\properties\propertyarray.zep.c
	test\properties\protectedproperties.zep.c
	test\properties\publicproperties.zep.c
	test\properties\staticprotectedproperties.zep.c
	test\properties\staticpublicproperties.zep.c
	test\range.zep.c
	test\references.zep.c
	test\regexdna.zep.c
	test\requires.zep.c
	test\resourcetest.zep.c
	test\returns.zep.c
	test\router\exception.zep.c
	test\scallparent.zep.c
	test\spectralnorm.zep.c
	test\strings.zep.c
	test\ternary.zep.c
	test\testinterface.zep.c
	test\trie.zep.c
	test\trytest.zep.c
	test\typeoff.zep.c
	test\unknownclass.zep.c
	test\unsettest.zep.c
	test\vars.zep.c
	test\0__closure.zep.c
	test\1__closure.zep.c
	test\2__closure.zep.c
	test\3__closure.zep.c
	test\4__closure.zep.c
	test\5__closure.zep.c
	test\6__closure.zep.c "
	PHP_NEW_EXTENSION(test, $test_sources, $ext_shared,, )
	PHP_SUBST(TEST_SHARED_LIBADD)

	old_CPPFLAGS=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $INCLUDES"

	AC_CHECK_DECL(
		[HAVE_BUNDLED_PCRE],
		[
			AC_CHECK_HEADERS(
				[ext/pcre/php_pcre.h],
				[
					PHP_ADD_EXTENSION_DEP([test], [pcre])
					AC_DEFINE([ZEPHIR_USE_PHP_PCRE], [1], [Whether PHP pcre extension is present at compile time])
				],
				,
				[[#include "main/php.h"]]
			)
		],
		,
		[[#include "php_config.h"]]
	)

	AC_CHECK_DECL(
		[HAVE_JSON],
		[
			AC_CHECK_HEADERS(
				[ext/json/php_json.h],
				[
					PHP_ADD_EXTENSION_DEP([test], [json])
					AC_DEFINE([ZEPHIR_USE_PHP_JSON], [1], [Whether PHP json extension is present at compile time])
				],
				,
				[[#include "main/php.h"]]
			)
		],
		,
		[[#include "php_config.h"]]
	)

	CPPFLAGS=$old_CPPFLAGS

	PHP_INSTALL_HEADERS([ext/test], [php_TEST.h])

fi
