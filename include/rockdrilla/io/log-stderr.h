/* log-stderr: print message to stderr
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_IO_LOG_STDERR
#define HEADER_INCLUDED_IO_LOG_STDERR 1

#include "../misc/ext-c-begin.h"

#ifndef _GNU_SOURCE
// better to define in source before any #include
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "const.h"

static
void vlog_stderr(const char * prefix, const char * suffix, const char * fmt, va_list args)
__attribute__((format (printf, 3, 0)));

static
void vlog_stderr_error(const char * prefix, int error_num, const char * fmt, va_list args)
__attribute__((format (printf, 3, 0)));

static
void vlog_stderr_path_error(const char * prefix, const char * path_name, int error_num, const char * fmt, va_list args)
__attribute__((format (printf, 4, 0)));

static
void log_stderr(const char * fmt, ...)
__attribute__((format (printf, 1, 2)));

static
void log_stderr_ex(const char * prefix, const char * suffix, const char * fmt, ...)
__attribute__((format (printf, 3, 4)));

static
void log_stderr_error(int error_num, const char * fmt, ...)
__attribute__((format (printf, 2, 3)));

static
void log_stderr_error_ex(const char * prefix, int error_num, const char * fmt, ...)
__attribute__((format (printf, 3, 4)));

static
void log_stderr_path_error(const char * path_name, int error_num, const char * fmt, ...)
__attribute__((format (printf, 3, 4)));

static
void log_stderr_path_error_ex(const char * prefix, const char * path_name, int error_num, const char * fmt, ...)
__attribute__((format (printf, 4, 5)));

static
size_t _vlog_timestamp(const struct timespec * ts, char * buffer)
{
	struct tm _time_local;
	struct tm * t = localtime_r(&(ts->tv_sec), &_time_local);

	char b[64];
	size_t x = 0;

	x += strftime(b + x, sizeof(b) - x, "%Y-%m-%d %H:%M:%S", t);
	x += snprintf(b + x, sizeof(b) - x, ".%06d", (int) (ts->tv_nsec / 1000));
	x += strftime(b + x, sizeof(b) - x, "%z", t);
	b[x] = 0;

	if (buffer)
		strncpy(buffer, b, x);

	return x;
}

static
size_t _vlog_string(char * buffer, size_t length, const char * fmt, va_list args)
{
	if (!buffer) {
		va_list args2;
		va_copy(args2, args);
		size_t x = vsnprintf(NULL, 0, fmt, args2);
		va_end(args2);

		return x;
	}

	return vsnprintf(buffer, length, fmt, args);
}

static
void vlog_stderr(const char * prefix, const char * suffix, const char * fmt, va_list args)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);

	size_t len_ts   = _vlog_timestamp(&ts, NULL);
	size_t len_pre  = (prefix) ? strlen(prefix) : 0;
	size_t len_args = _vlog_string(NULL, 0, fmt, args);
	size_t len_suf  = (suffix) ? strlen(suffix) : 0;
	size_t buf_len  = len_ts + 1 /* space */
	                + len_pre + ((len_pre) ? 1 : 0) /* space */
	                + len_args + 1 /* LF */
	                + len_suf + ((len_suf) ? 1 : 0) /* space */
	                + 8 /* padding */;
	char b[buf_len];
	b[buf_len - 1] = 0;

	size_t x = 0;

	x += _vlog_timestamp(&ts, b);
	b[x++] = ' ';

	if (len_pre) {
		memcpy(b + x, prefix, len_pre);
		x += len_pre;
		b[x++] = ' ';
	}

	x += _vlog_string(b + x, buf_len - x, fmt, args);

	if (len_suf) {
		b[x++] = ' ';
		memcpy(b + x, suffix, len_suf);
		x += len_suf;
	}

	b[x++] = '\n';

	(void) write(STDERR_FILENO, b, x);
}

static
void vlog_stderr_error(const char * prefix, int error_num, const char * fmt, va_list args)
{
	char b[1024];
	b[sizeof(b) - 1] = 0;

	size_t k = snprintf(b, sizeof(b), "error %d: ", error_num);
	char * e_tmp = b + k;
	char * e_str = strerror_r(error_num, e_tmp, sizeof(b) - k - 1);
	if (e_str != e_tmp) strcpy(e_tmp, e_str);
	b[k + strlen(e_str)] = 0;

	vlog_stderr(prefix, b, fmt, args);
}

// should be reusing vlog_stderr_error() but...
static
void vlog_stderr_path_error(const char * prefix, const char * path_name, int error_num, const char * fmt, va_list args)
{
	char b[1024 + PATH_MAX];
	b[sizeof(b) - 1] = 0;

	size_t k = snprintf(b, sizeof(b), "path '%s' error %d: ", path_name, error_num);
	char * e_tmp = b + k;
	char * e_str = strerror_r(error_num, e_tmp, sizeof(b) - k - 1);
	if (e_str != e_tmp) strcpy(e_tmp, e_str);
	b[k + strlen(e_str)] = 0;

	vlog_stderr(prefix, b, fmt, args);
}

static
void log_stderr(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog_stderr(NULL, NULL, fmt, args);
	va_end(args);
}

static
void log_stderr_ex(const char * prefix, const char * suffix, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog_stderr(prefix, suffix, fmt, args);
	va_end(args);
}

static
void log_stderr_error(int error_num, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog_stderr_error(NULL, error_num, fmt, args);
	va_end(args);
}

static
void log_stderr_error_ex(const char * prefix, int error_num, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog_stderr_error(prefix, error_num, fmt, args);
	va_end(args);
}

static
void log_stderr_path_error(const char * path_name, int error_num, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog_stderr_path_error(NULL, path_name, error_num, fmt, args);
	va_end(args);
}

static
void log_stderr_path_error_ex(const char * prefix, const char * path_name, int error_num, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vlog_stderr_path_error(prefix, path_name, error_num, fmt, args);
	va_end(args);
}

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_IO_LOG_STDERR */
