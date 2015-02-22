#ifndef ZEPHIR_KERNEL_FILE_H
#define ZEPHIR_KERNEL_FILE_H

int zephir_file_exists(zval *filename);
void zephir_file_get_contents(zval *return_value, zval *filename);
void zephir_file_put_contents(zval *return_value, zval *filename, zval *data);

#endif
