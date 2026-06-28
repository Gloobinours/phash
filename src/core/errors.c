#ifndef RELEASE_MODE

#include "../../include/core/errors.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static log_level_t static_current_level = LOG_LEVEL_DEBUG;

static const char* level_strings[] = { "DEBUG", "INFO", "WARN", "ERROR" };

void
log_set_level(log_level_t level)
{
  static_current_level = level;
}

void
log_internal(log_level_t level,
             const char* file,
             int line,
             const char* fmt,
             ...)
{
  if (level < static_current_level) {
    return;
  }

  time_t rawtime;
  struct tm timeinfo;
  char time_buffer[26]; // Safe buffer length for timestamp format
  va_list args;

  time(&rawtime);
  localtime_r(&rawtime, &timeinfo);
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  fprintf( // NOLINT
    stderr,
    "%s [%s] (%s:%d): ",
    time_buffer,
    level_strings[level],
    file,
    line);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n"); // NOLINT
}

#endif // RELEASE_MODE
