#ifndef ZEPHIR_KERNEL_EXIT_H
#define ZEPHIR_KERNEL_EXIT_H

#include <Zend/zend.h>

void zephir_exit_empty();
void zephir_exit(zval *ptr);

#endif