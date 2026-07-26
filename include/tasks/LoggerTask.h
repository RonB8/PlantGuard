#pragma once

// ============================================================================
// LoggerTask.h — Sole owner of Serial output.
//
// Every other task sends LogMessage structs through Resources::logQueue
// (via the Logger::log helper in logging.h) instead of calling Serial
// directly. This is the only task allowed to touch Serial, which sidesteps
// the need for a Serial-access mutex entirely: a single-consumer queue is
// simpler and cheaper than shared-resource locking.
// ============================================================================

void LoggerTask(void* pvParameters);
