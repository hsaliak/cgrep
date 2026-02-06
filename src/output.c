#include "raii.h"
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>

static pthread_mutex_t g_output_mutex = PTHREAD_MUTEX_INITIALIZER;

void output_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pthread_mutex_lock(&g_output_mutex);
    vprintf(fmt, args);
    pthread_mutex_unlock(&g_output_mutex);
    va_end(args);
}
