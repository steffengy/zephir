
#ifndef ZEPHIR_KERNEL_OBJECT_H
#define ZEPHIR_KERNEL_OBJECT_H

void zephir_get_called_class(zval *return_value);

//zephir_read_property(zval *target, zval *src, char *name, size_t length)
#define zephir_read_property(target_zval, src_zval, name_str) zend_read_property(Z_OBJCE_P(src_zval), src_zval, name_str, 0, target_zval)

/* Update properties */
#define zephir_update_property_zval(this_ptr, name_str, value) zend_update_property(Z_OBJCE_P(this_ptr), this_ptr, name_str, &value)
#define zephir_update_static_property_ce(ce, name_str, value) zend_update_static_property(ce, name_str, value)
#endif
