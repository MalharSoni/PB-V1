#include "slog.hpp"
#include "pros/misc.h"
#include <cstdarg>
#include <cstring>
#include <ctime>

namespace slog {

// ============================================================================
// INTERNAL STATE
// ============================================================================

static char ring_buffer[QUEUE_SLOTS][LINE_BYTES];
static int write_idx = 0;      // Producer writes here
static int read_idx = 0;       // Consumer reads from here
static pros::Mutex queue_mutex;

static FILE* logfile = nullptr;
static char filename[64];
static bool is_running = false;

static Stats stats = {0, QUEUE_SLOTS, 0, 0, 0};

// Writer task handle
static pros::Task* writer_task = nullptr;

// ============================================================================
// RING BUFFER HELPERS
// ============================================================================

static inline int queue_size() {
    return (write_idx - read_idx + QUEUE_SLOTS) % QUEUE_SLOTS;
}

static inline bool queue_full() {
    return queue_size() >= (QUEUE_SLOTS - 1);
}

static inline bool queue_empty() {
    return read_idx == write_idx;
}

// ============================================================================
// WRITER TASK (Consumer)
// ============================================================================

static void writer_task_fn(void*) {
    int lines_since_flush = 0;

    while (is_running) {
        // Check if there's data to write
        queue_mutex.take(TIMEOUT_MAX);
        bool has_data = !queue_empty();
        int depth = queue_size();
        queue_mutex.give();

        if (has_data && logfile != nullptr) {
            // Dequeue one line
            queue_mutex.take(TIMEOUT_MAX);
            const char* line = ring_buffer[read_idx];
            read_idx = (read_idx + 1) % QUEUE_SLOTS;
            queue_mutex.give();

            // Write to file (blocking, but at low priority)
            fprintf(logfile, "%s\n", line);
            stats.lines++;
            lines_since_flush++;

            // Periodic flush
            if (lines_since_flush >= FLUSH_EVERY) {
                fflush(logfile);
                lines_since_flush = 0;
            }

            // Update high water mark
            if (depth > stats.high_water) {
                stats.high_water = depth;
            }
        } else {
            // No data, yield CPU
            pros::delay(1000 / WRITER_HZ);
        }
    }

    // Final flush before exit
    if (logfile != nullptr) {
        fflush(logfile);
    }
}

// ============================================================================
// API IMPLEMENTATION
// ============================================================================

bool init(const char* hint, const char* header_csv) {
    if (is_running) {
        return false;  // Already initialized
    }

    // Generate timestamped filename
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    snprintf(filename, sizeof(filename), "/usd/%s_%02d%02d_%02d%02d%02d.csv",
             hint,
             timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    // Open file
    logfile = fopen(filename, "w");
    if (logfile == nullptr) {
        printf("[slog] ERROR: Failed to open %s\n", filename);
        return false;
    }

    // Write header
    if (header_csv != nullptr) {
        fprintf(logfile, "%s\n", header_csv);
        printf("[slog] Wrote header: %s\n", header_csv);
    } else {
        fprintf(logfile, "v=1,t_ms,data\n");  // Default header
        printf("[slog] Wrote default header\n");
    }
    fflush(logfile);
    printf("[slog] Flushed header\n");

    // Initialize state
    write_idx = 0;
    read_idx = 0;
    stats = {0, QUEUE_SLOTS, 0, 0, 0};
    is_running = true;

    // Start writer task at low priority
    writer_task = new pros::Task(writer_task_fn, nullptr, TASK_PRIORITY_DEFAULT - 1, TASK_STACK_DEPTH_DEFAULT, "SD Writer");

    printf("[slog] Logging to %s\n", filename);
    return true;
}

void close() {
    if (!is_running) {
        return;
    }

    printf("[slog] Closing... Queue has %d lines to flush\n", queue_size());

    // Wait for queue to drain (up to 10 seconds)
    int wait_count = 0;
    while (!queue_empty() && wait_count < 100) {
        pros::delay(100);  // Check every 100ms
        wait_count++;
        if (wait_count % 10 == 0) {
            printf("[slog] Still flushing... %d lines remaining\n", queue_size());
        }
    }

    if (!queue_empty()) {
        printf("[slog] WARNING: Queue not empty after 10s! Forcing close. %d lines lost.\n", queue_size());
    }

    is_running = false;

    // Wait for writer task to finish
    if (writer_task != nullptr) {
        pros::delay(200);  // Give it time to finish final write
        delete writer_task;
        writer_task = nullptr;
    }

    // Close file
    if (logfile != nullptr) {
        fflush(logfile);
        fclose(logfile);
        printf("[slog] File closed: %s\n", filename);
        logfile = nullptr;
    }

    printf("[slog] Closed. Lines=%u Drops=%u\n", stats.lines, stats.drops);
}

bool ready() {
    return is_running && logfile != nullptr;
}

bool enqueue_line(const char* line) {
    if (!ready()) {
        return false;
    }

    queue_mutex.take(TIMEOUT_MAX);

    if (queue_full()) {
        stats.drops++;
        queue_mutex.give();
        return false;  // Queue full, drop line
    }

    // Copy line to ring buffer
    strncpy(ring_buffer[write_idx], line, LINE_BYTES - 1);
    ring_buffer[write_idx][LINE_BYTES - 1] = '\0';  // Ensure null termination

    write_idx = (write_idx + 1) % QUEUE_SLOTS;

    // Update stats
    stats.queue_depth = queue_size();

    queue_mutex.give();
    return true;
}

void logf(const char* fmt, ...) {
    if (!ready()) {
        return;
    }

    char line[LINE_BYTES];
    va_list args;
    va_start(args, fmt);
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    enqueue_line(line);
}

void throttled_logf(int n, const char* fmt, ...) {
    static int counter = 0;
    static int logged_count = 0;

    if (!ready()) {
        return;
    }

    counter++;
    if (counter % n != 0) {
        return;  // Skip this call
    }

    char line[LINE_BYTES];
    va_list args;
    va_start(args, fmt);
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    bool success = enqueue_line(line);
    if (success) {
        logged_count++;
        if (logged_count <= 5 || logged_count % 100 == 0) {
            printf("[slog] Logged line %d\n", logged_count);
        }
    }
}

void get_stats(Stats& out) {
    queue_mutex.take(TIMEOUT_MAX);
    out = stats;
    out.queue_depth = queue_size();  // Real-time depth
    queue_mutex.give();
}

} // namespace slog
