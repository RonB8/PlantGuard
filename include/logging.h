#pragma once

// ============================================================================
// logging.h — Non-blocking, thread-safe logging front-end.
//
// Every task calls Logger::log(...) instead of touching Serial directly.
// Serial is not safe to share across tasks without serialization, so this
// module just formats a message and pushes it onto Resources::logQueue;
// LoggerTask is the sole owner of Serial and drains that queue. If the
// queue is momentarily full the call drops the message rather than
// blocking the caller — logging must never stall a sensor/network task.
// ============================================================================

#include "types.h"

namespace Logger {

// printf-style formatting. Truncates to fit LogMessage::text.
void log(LogLevel level, const char* fmt, ...);

inline void debug(const char* fmt, ...);
inline void info(const char* fmt, ...);
inline void warn(const char* fmt, ...);
inline void error(const char* fmt, ...);

} // namespace Logger

// Convenience macros so call sites read `LOG_INFO("moisture %d%%", pct);`
// without repeating Logger::log(LogLevel::LOG_INFO, ...) everywhere.
#define LOG_DEBUG(...) Logger::log(LogLevel::LOG_DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  Logger::log(LogLevel::LOG_INFO, __VA_ARGS__)
#define LOG_WARN(...)  Logger::log(LogLevel::LOG_WARN, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(LogLevel::LOG_ERROR, __VA_ARGS__)
