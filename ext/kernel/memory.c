#include "php.h"
#include "php_ext.h"
#include "kernel/memory.h"
#include <Zend/zend_alloc.h>

#include "kernel/fcall.h"

static zephir_memory_entry* zephir_memory_grow_stack_common(zend_zephir_globals_def *g)
{
	assert(g->start_memory != NULL);
	if (!g->active_memory) {
		g->active_memory = g->start_memory;
	}
	else if (!g->active_memory->next) {
#ifndef PHP_WIN32
		assert(g->active_memory >= g->end_memory - 1 || g->active_memory < g->start_memory);
#endif
		zephir_memory_entry *entry = (zephir_memory_entry *) ecalloc(1, sizeof(zephir_memory_entry));
		entry->prev       = g->active_memory;
		entry->prev->next = entry;
		g->active_memory  = entry;
	}
	else {
		assert(g->active_memory < g->end_memory && g->active_memory >= g->start_memory);
		g->active_memory = g->active_memory->next;
	}

	assert(g->active_memory->pointer == 0);
	assert(g->active_memory->hash_pointer == 0);

	return g->active_memory;
}

/**
 * Pre-allocates memory for further use in execution
 */
void zephir_initialize_memory(zend_zephir_globals_def *zephir_globals_ptr)
{
	zephir_memory_entry *start;
	size_t i;

	start = (zephir_memory_entry *) pecalloc(ZEPHIR_NUM_PREALLOCATED_FRAMES, sizeof(zephir_memory_entry), 1);
/* pecalloc() will take care of these members for every frame
	start->pointer      = 0;
	start->hash_pointer = 0;
	start->prev = NULL;
	start->next = NULL;
*/
	for (i = 0; i < ZEPHIR_NUM_PREALLOCATED_FRAMES; ++i) {
		start[i].addresses       = pecalloc(24, sizeof(zval*), 1);
		start[i].capacity        = 24;
		start[i].hash_addresses  = pecalloc(8, sizeof(zval*), 1);
		start[i].hash_capacity   = 8;

#ifndef ZEPHIR_RELEASE
		start[i].permanent = 1;
#endif
	}

	start[0].next = &start[1];
	start[ZEPHIR_NUM_PREALLOCATED_FRAMES - 1].prev = &start[ZEPHIR_NUM_PREALLOCATED_FRAMES - 2];

	for (i = 1; i < ZEPHIR_NUM_PREALLOCATED_FRAMES - 1; ++i) {
		start[i].next = &start[i + 1];
		start[i].prev = &start[i - 1];
	}

	zephir_globals_ptr->start_memory = start;
	zephir_globals_ptr->end_memory   = start + ZEPHIR_NUM_PREALLOCATED_FRAMES;

	zephir_globals_ptr->fcache = pemalloc(sizeof(HashTable), 1);
	zend_hash_init(zephir_globals_ptr->fcache, 128, NULL, NULL, 1); // zephir_fcall_cache_dtor

	zephir_globals_ptr->initialized = 1;
}

/**
 * Restore a memory stack applying GC to all observed variables
 */
static void zephir_memory_restore_stack_common(zend_zephir_globals_def *g)
{
	size_t i;
	zephir_memory_entry *prev, *active_memory;
	zephir_symbol_table *active_symbol_table;
	zval *ptr;

	active_memory = g->active_memory;
	assert(active_memory != NULL);

	if (EXPECTED(!CG(unclean_shutdown))) {
		/* Clean active symbol table */
		if (g->active_symbol_table) {
			active_symbol_table = g->active_symbol_table;
			if (active_symbol_table->scope == active_memory) {
				zend_hash_destroy(EG(current_execute_data)->symbol_table);
				FREE_HASHTABLE(EG(current_execute_data)->symbol_table);
				EG(current_execute_data)->symbol_table = active_symbol_table->symbol_table;
				g->active_symbol_table = active_symbol_table->prev;
				efree(active_symbol_table);
			}
		}

		/* Check for non freed hash key zvals, mark as null to avoid string freeing */
		for (i = 0; i < active_memory->hash_pointer; ++i) {
			assert(active_memory->hash_addresses[i] != NULL);
			/* Skip non-reference tracked variables */
			if (!Z_REFCOUNTED_P(active_memory->hash_addresses[i])) {
				continue;
			}
			if (Z_REFCOUNT_P(active_memory->hash_addresses[i]) <= 1) {
				ZVAL_NULL(active_memory->hash_addresses[i]);
			} else {
				zval_copy_ctor(active_memory->hash_addresses[i]);
			}
		}

#ifndef ZEPHIR_RELEASE
		for (i = 0; i < active_memory->pointer; ++i) {
			if (active_memory->addresses[i] != NULL) {
				zval *var = active_memory->addresses[i];
				if (Z_TYPE_P(var) > IS_CALLABLE && Z_TYPE_P(var) != IS_INDIRECT) {
					fprintf(stderr, "%s: observed variable #%d (%p) has invalid type %u [%s]\n", __func__, (int)i, var, Z_TYPE_P(var), active_memory->func);
					assert(0);
				}

				if (Z_REFCOUNTED_P(var) && Z_REFCOUNT_P(var) == 0) {
					fprintf(stderr, "%s: observed variable #%d (%p) has 0 references, type=%d [%s]\n", __func__, (int)i, var, Z_TYPE_P(var), active_memory->func);
					assert(0);
				}
				else if (Z_REFCOUNTED_P(var) && Z_REFCOUNT_P(var) >= 1000000) {
					fprintf(stderr, "%s: observed variable #%d (%p) has too many references (%u), type=%d  [%s]\n", __func__, (int)i, var, Z_REFCOUNT_P(var), Z_TYPE_P(var), active_memory->func);
					assert(0);
				}
#if 0
				/* Skip this check, PDO does return variables with is_ref = 1 and refcount = 1*/
				else if (Z_REFCOUNTED(var) && Z_REFCOUNT_P(var) == 1 && Z_ISREF_P(var)) {
					fprintf(stderr, "%s: observed variable #%d (%p) is a reference with reference count = 1, type=%d  [%s]\n", __func__, (int)i, var, Z_TYPE_P(var), active_memory->func);
				}
#endif
			}
		}
#endif

		/* Traverse all zvals allocated, reduce the reference counting or free them */
		for (i = 0; i < active_memory->pointer; ++i) {
			ptr = active_memory->addresses[i];
			if (EXPECTED(ptr != NULL)) {
				if (!Z_REFCOUNTED_P(ptr)) {
					zval_ptr_dtor(ptr);
					continue;
				}
				if (Z_REFCOUNT_P(ptr) == 1) {
					//TODO: Is it really a good idea to just free a zval?
					if (!Z_ISREF_P(ptr)) {
						zval_ptr_dtor(ptr);
					} else {
						zval_ptr_dtor(ptr);
						//ZVAL_UNREF(ptr);
						//efree(ptr);
					}
				} else {
					Z_DELREF_P(ptr);
				}
			}
		}
	}

#ifndef ZEPHIR_RELEASE
	active_memory->func = NULL;
#endif

	prev = active_memory->prev;

	if (active_memory >= g->end_memory || active_memory < g->start_memory) {
#ifndef ZEPHIR_RELEASE
		assert(g->active_memory->permanent == 0);
#endif
		assert(prev != NULL);

		if (active_memory->hash_addresses != NULL) {
			efree(active_memory->hash_addresses);
		}

		if (active_memory->addresses != NULL) {
			efree(active_memory->addresses);
		}

		efree(g->active_memory);
		g->active_memory = prev;
		prev->next = NULL;
	} else {
#ifndef ZEPHIR_RELEASE
		assert(g->active_memory->permanent == 1);
#endif
		active_memory->pointer      = 0;
		active_memory->hash_pointer = 0;
		g->active_memory = prev;
	}

#ifndef ZEPHIR_RELEASE
	if (g->active_memory) {
		zephir_memory_entry *f = g->active_memory;
		if (f >= g->start_memory && f < g->end_memory - 1) {
			assert(f->permanent == 1);
			assert(f->next != NULL);

			if (f > g->start_memory) {
				assert(f->prev != NULL);
			}
		}
	}
#endif
}

/**
 * Adds a memory frame in the current executed method
 */
void ZEND_FASTCALL zephir_memory_grow_stack()
{
	zend_zephir_globals_def *g = ZEPHIR_VGLOBAL;
	if (g->start_memory == NULL) {
		zephir_initialize_memory(g);
	}
	zephir_memory_grow_stack_common(g);
}

int ZEND_FASTCALL zephir_memory_restore_stack()
{
	zephir_memory_restore_stack_common(ZEPHIR_VGLOBAL);
	return SUCCESS;
}

/**
 * Cleans the zephir memory stack recursivery
 */
int ZEND_FASTCALL zephir_clean_restore_stack() {

	zend_zephir_globals_def *zephir_globals_ptr = ZEPHIR_VGLOBAL;

	while (zephir_globals_ptr->active_memory != NULL) {
		zephir_memory_restore_stack_common(zephir_globals_ptr);
	}

	return SUCCESS;
}

/**
 * Deinitializes all the memory allocated by Zephir
 */
void zephir_deinitialize_memory()
{
	size_t i;
	zend_zephir_globals_def *zephir_globals_ptr = ZEPHIR_VGLOBAL;

	if (zephir_globals_ptr->initialized != 1) {
		zephir_globals_ptr->initialized = 0;
		return;
	}

	if (zephir_globals_ptr->start_memory != NULL) {
		zephir_clean_restore_stack();
	}

	//TODO zend_hash_apply_with_arguments(zephir_globals_ptr->fcache, zephir_cleanup_fcache, 0);

#ifndef ZEPHIR_RELEASE
	assert(zephir_globals_ptr->start_memory != NULL);
#endif

	for (i = 0; i < ZEPHIR_NUM_PREALLOCATED_FRAMES; ++i) {
		pefree(zephir_globals_ptr->start_memory[i].hash_addresses, 1);
		pefree(zephir_globals_ptr->start_memory[i].addresses, 1);
	}

	pefree(zephir_globals_ptr->start_memory, 1);
	zephir_globals_ptr->start_memory = NULL;

	zend_hash_destroy(zephir_globals_ptr->fcache);
	pefree(zephir_globals_ptr->fcache, 1);
	zephir_globals_ptr->fcache = NULL;

	zephir_globals_ptr->initialized = 0;
}

ZEPHIR_ATTR_NONNULL static void zephir_reallocate_memory(const zend_zephir_globals_def *g)
{
	zephir_memory_entry *frame = g->active_memory;
	int persistent = (frame >= g->start_memory && frame < g->end_memory);
	void *buf = perealloc(frame->addresses, sizeof(zval **) * (frame->capacity + 16), persistent);
	if (EXPECTED(buf != NULL)) {
		frame->capacity += 16;
		frame->addresses = buf;
	}
	else {
		zend_error(E_CORE_ERROR, "Memory allocation failed");
	}

#ifndef ZEPHIR_RELEASE
	assert(frame->permanent == persistent);
#endif
}

ZEPHIR_ATTR_NONNULL1(2) static inline void zephir_do_memory_observe(zval *var, const zend_zephir_globals_def *g)
{
	zephir_memory_entry *frame = g->active_memory;
#ifndef ZEPHIR_RELEASE
	if (UNEXPECTED(frame == NULL)) {
		fprintf(stderr, "ZEPHIR_MM_GROW() must be called before using any of MM functions or macros!");
		zephir_print_backtrace();
		abort();
	}
#endif

	if (UNEXPECTED(frame->pointer == frame->capacity)) {
		zephir_reallocate_memory(g);
	}

#ifndef ZEPHIR_RELEASE
	{
		size_t i;
		for (i = 0; i < frame->pointer; ++i) {
			if (frame->addresses[i] == var) {
				fprintf(stderr, "Variable %p is already observed", var);
				zephir_print_backtrace();
				abort();
			}
		}
	}
#endif

	frame->addresses[frame->pointer] = var;
	++frame->pointer;
}

/**
 * Observes a variable and allocates memory for it
 */
void ZEND_FASTCALL zephir_memory_alloc(zval *var)
{
	zend_zephir_globals_def *g = ZEPHIR_VGLOBAL;
	zephir_do_memory_observe(var, g);
	ZVAL_NULL(var);
}

/**
 * Observes a memory pointer to release its memory at the end of the request
 */
void ZEND_FASTCALL zephir_memory_observe(zval *var)
{
	zend_zephir_globals_def *g = ZEPHIR_VGLOBAL;
	zephir_do_memory_observe(var, g);
	ZVAL_NULL(var); /* In case an exception or error happens BEFORE the observed variable gets initialized */
}

void ZEND_FASTCALL zephir_memory_only_observe(zval *var)
{
	zend_zephir_globals_def *g = ZEPHIR_VGLOBAL;
	zephir_do_memory_observe(var, g);
	ZVAL_NULL(var); /* In case an exception or error happens BEFORE the observed variable gets initialized */
}


/**
 * Initialize and duplicate a new zval, containing a zend_object (memory-safe)
 */
void zephir_obj_cpy_wrt_ctor(zval *d, zend_object *obj)
{
	zval tmp;

	if (d && Z_REFCOUNTED_P(d)) {
		zval_ptr_dtor(d);
	} else {
		zephir_memory_observe(d);
	}
	ZVAL_OBJ(&tmp, obj);
	ZVAL_DUP(d, &tmp);
}

#ifndef ZEPHIR_RELEASE

/**
 * Dumps a memory frame for debug purposes
 */
void zephir_dump_memory_frame(zephir_memory_entry *active_memory TSRMLS_DC)
{
	size_t i;

	assert(active_memory != NULL);

	fprintf(stderr, "Dump of the memory frame %p (%s)\n", active_memory, active_memory->func);

	if (active_memory->hash_pointer) {
		for (i = 0; i < active_memory->hash_pointer; ++i) {
			assert(active_memory->hash_addresses[i] != NULL);
			fprintf(stderr, "Hash ptr %lu (%p), type=%u, refcnt=%u\n", (ulong)i, active_memory->hash_addresses[i], Z_TYPE_P(active_memory->hash_addresses[i]), Z_REFCOUNT_P(active_memory->hash_addresses[i]));
		}
	}

	for (i = 0; i < active_memory->pointer; ++i) {
		if (EXPECTED(active_memory->addresses[i] != NULL)) {
			zval *var = active_memory->addresses[i];
			fprintf(stderr, "Obs var %lu (%p => %p), type=%u, refcnt=%u; ", (ulong)i, var, *var, Z_TYPE_P(var), Z_REFCOUNT_P(var));
			/*switch (Z_TYPE_PP(var)) {
				case IS_NULL:     fprintf(stderr, "value=NULL\n"); break;
				case IS_LONG:     fprintf(stderr, "value=%ld\n", Z_LVAL_PP(var)); break;
				case IS_DOUBLE:   fprintf(stderr, "value=%E\n", Z_DVAL_PP(var)); break;
				case IS_BOOL:     fprintf(stderr, "value=(bool)%d\n", Z_BVAL_PP(var)); break;
				case IS_ARRAY:    fprintf(stderr, "value=array(%p), %d elements\n", Z_ARRVAL_PP(var), zend_hash_num_elements(Z_ARRVAL_PP(var))); break;
				case IS_OBJECT:   fprintf(stderr, "value=object(%u), %s\n", Z_OBJ_HANDLE_PP(var), Z_OBJCE_PP(var)->name); break;
				case IS_STRING:   fprintf(stderr, "value=%*s (%p)\n", Z_STRLEN_PP(var), Z_STRVAL_PP(var), Z_STRVAL_PP(var)); break;
				case IS_RESOURCE: fprintf(stderr, "value=(resource)%ld\n", Z_LVAL_PP(var)); break;
				default:          fprintf(stderr, "\n"); break;
			}*/
		}
	}

	fprintf(stderr, "End of the dump of the memory frame %p\n", active_memory);
}

void zephir_dump_current_frame(TSRMLS_D)
{
	zend_zephir_globals_def *zephir_globals_ptr = ZEPHIR_VGLOBAL;

	if (UNEXPECTED(zephir_globals_ptr->active_memory == NULL)) {
		fprintf(stderr, "WARNING: calling %s() without an active memory frame!\n", __func__);
		zephir_print_backtrace();
		return;
	}

	zephir_dump_memory_frame(zephir_globals_ptr->active_memory TSRMLS_CC);
}

void zephir_dump_all_frames(TSRMLS_D)
{
	zend_zephir_globals_def *zephir_globals_ptr = ZEPHIR_VGLOBAL;
	zephir_memory_entry *active_memory       = zephir_globals_ptr->active_memory;

	fprintf(stderr, "*** DUMP START ***\n");
	while (active_memory != NULL) {
		zephir_dump_memory_frame(active_memory TSRMLS_CC);
		active_memory = active_memory->prev;
	}

	fprintf(stderr, "*** DUMP END ***\n");
}
#endif
