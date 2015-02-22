#ifndef ZEPHIR_KERNEL_MEMORY_H
#define ZEPHIR_KERNEL_MEMORY_H

#define ZEPHIR_NUM_PREALLOCATED_FRAMES 25

#define ZEPHIR_MM_GROW() zephir_memory_grow_stack()
#define ZEPHIR_MM_RESTORE() zephir_memory_restore_stack()

#define ZEPHIR_OBSERVE_OR_NULLIFY_ZVAL(z) \
	do { \
		zval *tmp = z; \
		if (z) { \
			zval_ptr_dtor(tmp); \
			ZVAL_UNDEF(tmp); \
		} \
		else if (tmp != NULL) { \
			zephir_memory_observe(z); \
		} \
	} while (0)

#define ZEPHIR_OBS_VAR(z)  zephir_memory_observe(z)

#define ZEPHIR_OBS_NVAR(z)\
	if (z) { \
		if (Z_REFCOUNTED_P(z) && Z_REFCOUNT_P(z) > 1) { \
			Z_DELREF_P(z); \
		} else {\
			zval_ptr_dtor(z); \
			ZVAL_NULL(z); \
		} \
	} else { \
		zephir_memory_observe(z); \
	}

/* ZVAL_UNDEF to prevent errors in zend engine (dtor calls method for a specific type since junk data in memory...) */
#define ZEPHIR_INIT_VAR(z) ZVAL_UNDEF(z); zephir_memory_alloc(z)

/* Even a not referenced variable can hold values on the stack! */
#define ZEPHIR_INIT_ZVAL_NREF(z) ZEPHIR_INIT_VAR(z)

#define ZEPHIR_INIT_NVAR(z)	\
    if (z) { \
		if (Z_REFCOUNTED_P(z)) { \
			zval_ptr_dtor(z); \
			if (Z_ISREF_P(z) && Z_REFCOUNT_P(z) > 1) { \
				ZVAL_UNREF(z); \
			} \
		} \
		ZVAL_NULL(z); \
	} else { \
		zephir_memory_alloc(z); \
	}

#define ZEPHIR_CPY_WRT(d, v) \
    if (Z_REFCOUNTED_P(v)) Z_ADDREF_P(v); \
	if (d) { \
		if (Z_REFCOUNTED_P(d) && Z_REFCOUNT_P(d) > 0) { \
			zval_ptr_dtor(d); \
		} \
	} else { \
		zephir_memory_observe(d); \
	} \
	ZVAL_COPY(d, v);

#define ZEPHIR_CPY_WRT_CTOR(d, v) \
	if (d) { \
		if (Z_REFCOUNTED_P(d) && Z_REFCOUNT_P(d) > 0) { \
			zval_ptr_dtor(d); \
		} \
	} else { \
		zephir_memory_observe(d); \
	} \
	ZVAL_NULL(d); \
	ZVAL_DUP(d, v); \

#define ZEPHIR_SEPARATE_PARAM(z) \
	do { \
		zval *orig_ptr = z; \
		zephir_memory_observe(z TSRMLS_CC); \
		ZVAL_NULL(z); \
		zval_copy_ctor(z); \
		ZVAL_UNREF(z); \
	} while (0)

void zephir_initialize_memory(zend_zephir_globals_def *zephir_globals_ptr);
void zephir_deinitialize_memory();
void ZEPHIR_FASTCALL zephir_memory_grow_stack();
int ZEPHIR_FASTCALL zephir_memory_restore_stack();
int ZEPHIR_FASTCALL zephir_clean_restore_stack();
int zephir_cleanup_fcache(void *pDest, int num_args, va_list args, zend_hash_key *hash_key);

void ZEPHIR_FASTCALL zephir_memory_observe(zval *var);
void ZEPHIR_FASTCALL zephir_memory_alloc(zval *var);

#endif
