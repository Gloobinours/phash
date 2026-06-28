#ifndef ERRORS_H
#define ERRORS_H

typedef enum
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} log_level_t;

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL 0
#endif // !CURRENT_LOG_LEVEL

#ifndef RELEASE_MODE
void
log_internal(log_level_t level,
             const char* file,
             int line,
             const char* fmt,
             ...);
void
log_set_level(log_level_t level);
#endif // !RELEASE_MODE

#ifdef RELEASE_MODE
// clang-format off
  #define LOG_DEBUG(fmt, ...) do {} while (0)
  #define LOG_INFO(fmt, ...) do {} while (0)
  #define LOG_WARN(fmt, ...) do {} while (0)
  #define LOG_ERROR(fmt, ...) do {} while (0)
  #define log_set_level(level) do {} while (0)
#else
  #define LOG_DEBUG(fmt, ...) log_internal(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_INFO(fmt, ...) log_internal(LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_WARN(fmt, ...) log_internal(LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_ERROR(fmt, ...) log_internal(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif // RELEASE_MODE
// clang-format on

#endif // !ERRORS_H
