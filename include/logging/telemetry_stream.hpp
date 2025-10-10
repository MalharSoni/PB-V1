#pragma once

/**
 * @file telemetry_stream.hpp
 * @brief CSV formatting for telemetry data
 *
 * Formats telemetry data into CSV rows < 200 bytes.
 * No heap allocation - uses stack buffers only.
 */

#include <cstdio>

namespace telem {

/**
 * @brief CSV header string (version 1)
 *
 * Schema:
 * - v: Schema version (always 1)
 * - t_ms: Timestamp in milliseconds
 * - x(in): X position in inches
 * - y(in): Y position in inches
 * - theta(deg): Heading in degrees
 * - v_l(ips): Left wheel velocity in inches per second
 * - v_r(ips): Right wheel velocity in inches per second
 * - batt_V: Battery voltage in volts
 * - mark: Marker/waypoint tag (empty or "MARK:tag=value")
 */
static const char* kHeader = "v=1,t_ms,x(in),y(in),theta(deg),v_l(ips),v_r(ips),batt_V,mark";

/**
 * @brief Format a telemetry row as CSV
 * @param out Output buffer
 * @param n Buffer size (must be >= 200)
 * @param t_ms Timestamp in milliseconds
 * @param x X position (inches)
 * @param y Y position (inches)
 * @param th Heading (degrees)
 * @param vl Left wheel velocity (ips)
 * @param vr Right wheel velocity (ips)
 * @param batt Battery voltage (volts)
 * @param mark Marker string (or "" for none)
 *
 * Output format:
 *   "t_ms,x,y,theta,vl,vr,batt,mark"
 *
 * Example:
 *   "1250,12.345,24.678,45.123,36.500,36.450,12.45,"
 *   "2500,15.234,28.912,47.250,0.000,0.000,12.42,MARK:wp=1"
 */
inline void format_row(char* out, size_t n,
                      int t_ms,
                      double x, double y, double th,
                      double vl, double vr,
                      double batt,
                      const char* mark) {
    std::snprintf(out, n, "%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%s",
                  t_ms, x, y, th, vl, vr, batt, (mark ? mark : ""));
}

} // namespace telem
