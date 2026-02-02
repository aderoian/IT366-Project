#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "common/logger.h"

#include "common/thread/thread.h"

// Forward declarations
void *logger_thread_fn(void *arg);

static LogQueue g_log_queue;
static FILE *g_log_file = NULL;
static int g_logger_running = 1;
static thread_t g_logger_thread;
static LogLevel g_log_min_print = LOG_INFO;
static LogLevel g_log_min_file = LOG_DEBUG;

void logger_init(const char *path, LogLevel minPrint, LogLevel minFile) {
    memset(&g_log_queue, 0, sizeof(g_log_queue));
    mutex_init(&g_log_queue.mutex);
    condvar_init(&g_log_queue.cond);

    g_log_min_print = minPrint;
    g_log_min_file = minFile;

    g_log_file = fopen(path, "w");

    thread_create(&g_logger_thread, logger_thread_fn, NULL);
}

void logger_shutdown(void) {
    g_logger_running = 0;

    // Wake up logger thread if waiting
    mutex_lock(&g_log_queue.mutex);
    condvar_signal(&g_log_queue.cond);
    mutex_unlock(&g_log_queue.mutex);

    thread_join(&g_logger_thread);

    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }

    mutex_destroy(&g_log_queue.mutex);
    condvar_destroy(&g_log_queue.cond);
}

void logger_queue_push(LogQueue *q, const LogMessage *msg) {
    mutex_lock(&q->mutex);

    if (q->count == LOG_QUEUE_CAP) {
        q->head = (q->head + 1) % LOG_QUEUE_CAP; // Drop oldest log
        q->count--;
    }

    q->buffer[q->tail] = *msg;
    q->tail = (q->tail + 1) % LOG_QUEUE_CAP;
    q->count++;

    condvar_signal(&q->cond);
    mutex_unlock(&q->mutex);
}

int logger_queue_pop(LogQueue *q, LogMessage *out) {
    pthread_mutex_lock(&q->mutex);

    while (q->count == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    *out = q->buffer[q->head];
    q->head = (q->head + 1) % LOG_QUEUE_CAP;
    q->count--;

    pthread_mutex_unlock(&q->mutex);
    return 1;
}

const char *log_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default:        return "UNKNOWN";
    }
}

void log_internal(const LogLevel level, const char* file, const int line, const char *fmt, ...) {
    LogMessage msg;
    msg.level = level;
    strncpy(msg.file, file, strlen(file));
    msg.file[strlen(file)] = '\0';
    msg.line = line;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.text, LOG_MSG_MAX, fmt, args);
    va_end(args);

    logger_queue_push(&g_log_queue, &msg);
}

void *logger_thread_fn(void *arg) {
    LogMessage msg;

    while (1) {
        logger_queue_pop(&g_log_queue, &msg);

        if (!g_logger_running) {
            break;
        }

        if (msg.level >= g_log_min_print) fprintf(stdout,
            "[%s] %s:%d: %s\n",
            log_level_to_string(msg.level),
            msg.file,
            msg.line,
            msg.text
        );

        if (msg.level >= g_log_min_file) fprintf(g_log_file,
            "[%s] %s:%d: %s\n",
            log_level_to_string(msg.level),
            msg.file,
            msg.line,
            msg.text
        );

        fflush(g_log_file);
    }

    return NULL;
}