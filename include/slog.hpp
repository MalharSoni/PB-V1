#pragma once

#include "pros/rtos.hpp"
#include <cstdint>
#include <cstdio>

/**
 * @file slog.hpp
 * @brief Non-blocking SD card logger with ring buffer and writer task
 *
 * Producer-consumer design:
 * - Producer (telemetry): enqueue_line() → ring buffer (high priority)
 * - Consumer (writer): background task flushes to SD (low priority)
 *
 * This prevents blocking motion loops during SD writes.
 */

namespace slog {

// ============================================================================
// CONFIGURATION
// ============================================================================

constexpr int QUEUE_SLOTS = 512;        // Ring buffer capacity
constexpr int LINE_BYTES = 200;         // Max bytes per CSV line
constexpr int WRITER_HZ = 100;          // Writer task frequency
constexpr int FLUSH_EVERY = 50;         // Flush after N lines

// ============================================================================
// STATISTICS
// ============================================================================

struct Stats {
    int queue_depth;        // Current items in queue
    int queue_capacity;     // Max queue size
    int high_water;         // Highest queue depth seen
    uint32_t drops;         // Lines dropped (queue full)
    uint32_t lines;         // Total lines written
};

// ============================================================================
// API
// ============================================================================

/**
 * @brief Initialize logger and start writer task
 * @param hint Filename hint (creates /usd/<hint>_MMDD_HHMMSS.csv)
 * @param header_csv Optional CSV header line (will prepend "v=1,")
 * @return true if initialization succeeded
 */
bool init(const char* hint = "run", const char* header_csv = nullptr);

/**
 * @brief Close logger and flush all remaining data
 * Blocks until writer task finishes
 */
void close();

/**
 * @brief Check if logger is ready to accept data
 * @return true if initialized and file is open
 */
bool ready();

/**
 * @brief Enqueue a pre-formatted CSV line
 * @param line Null-terminated string (max LINE_BYTES-1 chars)
 * @return true if enqueued, false if queue full (line dropped)
 *
 * Non-blocking. Safe to call from high-priority tasks.
 */
bool enqueue_line(const char* line);

/**
 * @brief Format and enqueue a line (printf-style)
 * @param fmt Printf format string
 * @param ... Format arguments
 *
 * Convenience wrapper around enqueue_line().
 */
void logf(const char* fmt, ...);

/**
 * @brief Throttled logging - only logs every Nth call
 * @param n Throttle factor (1=every call, 2=every other, etc.)
 * @param fmt Printf format string
 * @param ... Format arguments
 *
 * Uses static counter. Useful for rate limiting.
 */
void throttled_logf(int n, const char* fmt, ...);

/**
 * @brief Get current statistics
 * @param out Stats struct to fill
 */
void get_stats(Stats& out);

} // namespace slog
