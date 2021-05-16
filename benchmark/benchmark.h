
#include <string.h>
#include <stdlib.h>
#ifndef __APPLE__
#  include <malloc.h>
#endif

#include "allocator.h"

int
benchmark_initialize() {
	return 0;
}

int
benchmark_finalize(void) {
	return 0;
}

int
benchmark_thread_initialize(void) {
	return 0;
}

int
benchmark_thread_finalize(void) {
	return 0;
}

void*
benchmark_malloc(size_t alignment, size_t size) {
    return myMalloc(size);
}

extern void
benchmark_free(void* ptr) {
    myFree(ptr);
}

const char*
benchmark_name(void) {
	return "crt";
}

void
benchmark_thread_collect(void) {
}
