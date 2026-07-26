#pragma once

// ============================================================================
// CameraTask.h — Blocks on Resources::captureSemaphore, then captures one
// JPEG frame and forwards it to AITask via Resources::imageQueue.
// ============================================================================

void CameraTask(void* pvParameters);
