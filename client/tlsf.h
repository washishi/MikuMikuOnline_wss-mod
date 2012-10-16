#ifndef INCLUDED_tlsf
#define INCLUDED_tlsf

/*
** Two Level Segregated Fit memory allocator, version 1.9.
** Written by Matthew Conte, and placed in the Public Domain.
**	http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**	http://rtportal.upv.es/rtmalloc/allocators/tlsf/index.shtml
**
** Please see the accompanying Readme.txt for implementation
** notes and caveats.
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
*/
#include <stddef.h>
#include <Windows.h>
#include <new>

#if defined(__cplusplus)
extern "C" {
#endif

/* Create/destroy a memory pool. */
typedef void* tlsf_pool;
tlsf_pool tlsf_create(void* mem, size_t bytes);
void tlsf_destroy(tlsf_pool pool);

/* malloc/memalign/realloc/free replacements. */
void* tlsf_malloc(tlsf_pool pool, size_t bytes);
void* tlsf_memalign(tlsf_pool pool, size_t align, size_t bytes);
void* tlsf_realloc(tlsf_pool pool, void* ptr, size_t size);
void tlsf_free(tlsf_pool pool, void* ptr);

/* Debugging. */
typedef void (*tlsf_walker)(void* ptr, size_t size, int used, void* user);
void tlsf_walk_heap(tlsf_pool pool, tlsf_walker walker, void* user);
/* Returns nonzero if heap check fails. */
int tlsf_check_heap(tlsf_pool pool);

/* Returns internal block size, not original request size */
size_t tlsf_block_size(void* ptr);

/* Overhead of per-pool internal structures. */
size_t tlsf_overhead();

#if defined(__cplusplus)
};

void *tlsf_new(tlsf_pool mempool,size_t size);
void tlsf_delete(tlsf_pool mempool,void *p);

template<typename T>
T *tlsf_new(tlsf_pool mempool,size_t size){
	T *v= (T*)tlsf_malloc(mempool,size * sizeof(T));
	if(!v){
		std::bad_alloc b;
		throw b;
	}
	for(size_t t = 0;t < size;++t)::new(&v[t]) T;
	return v;
}

template<typename T>
T *tlsf_new(tlsf_pool mempool){
	T *v= (T*)tlsf_malloc(mempool,sizeof(T));
	if(!v){
		std::bad_alloc b;
		throw b;
	}
	return ::new(v) T;
}

template<typename T,class X>
T *tlsf_new(tlsf_pool mempool,X initialize){
	T *v= (T*)tlsf_malloc(mempool,sizeof(T));
	if(!v){
		std::bad_alloc b;
		throw b;
	}
	return ::new(v) T(initialize);
}

template<typename T,class X,class Y>
T *tlsf_new(tlsf_pool mempool,X initialize1,Y initialize2){
	T *v= (T*)tlsf_malloc(mempool,sizeof(T));
	if(!v){
		std::bad_alloc b;
		throw b;
	}
	return ::new(v) T(initialize1,initialize2);
}

template<typename T>
void tlsf_delete(tlsf_pool mempool,T *p)
{
	p->~T();
	if(p)tlsf_free(mempool,p);
}

template<typename T>
void tlsf_delete(tlsf_pool mempool,T *p,size_t size)
{
	int i = 0;
	for(i;i < size;++i)p->~T();
	if(p)tlsf_free(mempool,p);
}
#endif

#endif
