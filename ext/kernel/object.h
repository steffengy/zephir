
#ifndef ZEPHIR_KERNEL_OBJECT_H
#define ZEPHIR_KERNEL_OBJECT_H

void zephir_get_called_class(zval *return_value);
int zephir_instance_of_ev(const zval *object, const zend_class_entry *ce);
int zephir_create_closure_ex(zval *return_value, zval *this_ptr, zend_class_entry *ce, const char *method_name, uint32_t method_length);

/* _str receives the old char * wrapped in SL */
#define zephir_fetch_class_str(class_name, fetch_type) zephir_fetch_class_str_ex(class_name, fetch_type);
#define zephir_fetch_class(class_name, fetch_type) zend_fetch_class(class_name, fetch_type)
inline zend_class_entry *zephir_fetch_class_str_ex(char *class_name, size_t length, int fetch_type);
int zephir_class_exists(const zval *class_name, int autoload);
int zephir_interface_exists(const zval *class_name, int autoload);
int zephir_method_exists(const zval *object, const zval *method_name);

//zephir_fetch_static_property_ce(zend_class_entry *ce, const char *property, int len)
#define zephir_read_static_property_ce(target_zval, ce, property_str) ZVAL_COPY_VALUE(target_zval, zend_read_static_property(ce, property_str, 0))
//zephir_read_property(zval *target, zval *src, char *name, size_t length)
zval *zephir_read_property(zval *target, zval *src, const char *name, size_t length, zend_bool silent);
#define zephir_return_property(object, name_str) zephir_read_property(return_value, this_ptr, name_str, 0)
int zephir_read_property_zval(zval *result, zval *object, zval *property, int flags);

/* Update properties */
#define zephir_update_property_zval(this_ptr, name_str, value) zend_update_property(Z_OBJCE_P(this_ptr), this_ptr, name_str, value)
#define zephir_update_static_property_ce(ce, name_str, value) zend_update_static_property(ce, name_str, value)
#define zephir_update_static_property_null(ce, name_str) zend_update_static_property_null(ce, name_str)
#define zephir_update_static_property_bool(ce, name_str, value) zend_update_static_property_bool(ce, name_str, value)

/* Unset properties */
int zephir_unset_property(zval* object, const char* name);

/* Isset properties */
int zephir_isset_property_zval(zval *object, const zval *property);
int zephir_isset_property(zval *object, const char *property_name, unsigned int property_length);

/* Update properties (array) */
int zephir_update_property_array(zval *object, const char *property, uint32_t property_length, const zval *index, zval *value);
int zephir_update_property_array_append(zval *object, char *property, unsigned int property_length, zval *value);
int zephir_update_property_array_multi(zval *object, const char *property, uint32_t property_length, zval *value, const char *types, int types_length, int types_count, ...);
#endif
