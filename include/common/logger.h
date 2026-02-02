#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

#include "common/thread/condvar.h"

typedef enum LogLevel_E {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

#define LOG_MSG_MAX 512
#define LOG_QUEUE_CAP 1024

typedef struct LogMessage_S {
    LogLevel level;
    char file[128];
    int line;
    char text[LOG_MSG_MAX];
} LogMessage;

typedef struct {
    LogMessage buffer[LOG_QUEUE_CAP];
    int head;
    int tail;
    int count;

    mutex_t mutex;
    cond_t cond;
} LogQueue;

void logger_init(const char *path, LogLevel minPrint, LogLevel minFile);
void logger_shutdown(void);

void logger_queue_push(LogQueue *q, const LogMessage *msg);
void log_internal(const LogLevel level, const char* file, int line, const char *fmt, ...);

#define log_trace(fmt, ...) log_internal(LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_internal(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) log_internal(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) log_internal(LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_internal(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...) log_internal(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

const char * log_level_to_string(LogLevel level);

#endif /* LOGGER_H */