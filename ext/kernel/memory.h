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

#define ZEPHIR_OBS_VAR(z)  zephir_memory_observe(&z)
#define ZEPHIR_INIT_VAR(z) zephir_memory_alloc(&z)

/* Initialize a var without tracking */
#define ZEPHIR_INIT_ZVAL_NREF(z) ZVAL_NULL(&z);

#define ZEPHIR_INIT_NVAR(z)	\
    if (&z) { \
		if (Z_REFCOUNTED(z)) { \
			zval_ptr_dtor(&z); \
			if (Z_ISREF(z) && Z_REFCOUNT(z) > 1) { \
				ZVAL_UNREF(&z); \
			} \
		} \
		ZVAL_NULL(&z); \
	} else { \
		zephir_memory_alloc(&z); \
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

void zephir_initialize_memory(zend_zephir_globals_def *zephir_globals_ptr);
void zephir_deinitialize_memory();
void ZEPHIR_FASTCALL zephir_memory_grow_stack();
int ZEPHIR_FASTCALL zephir_memory_restore_stack();
int ZEPHIR_FASTCALL zephir_clean_restore_stack();
int zephir_cleanup_fcache(void *pDest, int num_args, va_list args, zend_hash_key *hash_key);

void ZEPHIR_FASTCALL zephir_memory_observe(zval *var);
void ZEPHIR_FASTCALL zephir_memory_alloc(zval *var);

#endif
