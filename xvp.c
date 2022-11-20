/* xvp: simple (or sophisticated?) launcher
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022, Konstantin Demin
 *
 * Example usage in shell scripts:
 *   xvp -u program /tmp/list
 * is roughly equal to:
 *   xargs -0 -a /tmp/list program &
 *   wait ; rm -f /tmp/list
 * where /tmp/list is file with NUL-separated arguments
 * except:
 * - `xvp' is NOT replacement for `xargs' or `xe'
 * - return code is EXACT program return code
 *   or appropriate error code
 * - /tmp/list is deleted by `xvp' as early as possible
 */

#define _GNU_SOURCE
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#define __STDC_WANT_LIB_EXT1__ 1

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "include/io/const.h"
#include "include/io/log-stderr.h"
#include "include/uvector/uvector.h"

#define XVP_OPTS "fhiu"

static void usage(int retcode)
{
	fputs(
	"xvp 0.2.1\n"
	"Usage: xvp [-" XVP_OPTS "] <program> [..<common args>] <arg file>\n"
	" -h  - help (show this message)\n"
	" -i  - info (print limits and do nothing)\n"
	" -f  - force (force _single_ <program> execution or return error)\n"
	" -u  - unlink (delete <arg file> if it's regular file)\n"
	"\n"
	" <arg file> - file with NUL-separated arguments\n"
	, stderr);

	exit(retcode);
}

static struct {
	uint8_t
	  Force_once,
	  Info_only,
	  Unlink_argfile
	;
} opt;

static char * callee = NULL;
static char * script = NULL;

static void parse_opts(int argc, char * argv[]);
static void prepare(int argc, char * argv[]);
static void run(void);

int main(int argc, char * argv[])
{
	if (argc < 2) usage(0);

	parse_opts(argc, argv);
	prepare(argc, argv);
	run();

	return 0;
}

static int handle_file_type(uint32_t type, const char * arg);
static void dump_error(int error_num, const char * where);
static void dump_path_error(int error_num, const char * where, const char * name);

static void parse_opts(int argc, char * argv[])
{
	memset(&opt, 0, sizeof(opt));

	int o;
	while ((o = getopt(argc, (char * const *) argv, "+" XVP_OPTS)) != -1) {
		switch (o) {
		case 'h':
			usage(0);
			break;
		case 'f':
			if (opt.Force_once) break;
			opt.Force_once = 1;
			continue;
		case 'i':
			if (opt.Info_only) break;
			opt.Info_only = 1;
			continue;
		case 'u':
			if (opt.Unlink_argfile) break;
			opt.Unlink_argfile = 1;
			continue;
		}

		usage(EINVAL);
	}

	if (((argc - optind) < 2) && !opt.Info_only)
		usage(EINVAL);
}

static size_t get_env_size(void)
{
	static size_t x = 0;
	if (x) return x;

	for (char ** p = environ; *p; ++p)
		x += strlen(*p) + 1;

	return x;
}

static size_t get_arg_max(void)
{
	static size_t x = 0;
	if (x) return x;

#ifdef _SC_ARG_MAX
	long len = sysconf(_SC_ARG_MAX);
	if (len > 0) return x = len;
#endif

#ifdef ARG_MAX
	if (ARG_MAX > 0) return x = ARG_MAX;
#endif

#ifdef RLIMIT_STACK
	struct rlimit stack_limit;
	if (getrlimit(RLIMIT_STACK, &stack_limit) == 0)
		return x = (stack_limit.rlim_cur / 4);
#endif

	// differs from "findutils" variant
	return x = (LONG_MAX >> 1);
}

static size_t   size_env, size_args, argc_max;
static string_v argv_init, argv_curr;

static struct stat f_stat;

static void prepare(int argc, char * argv[])
{
	callee = argv[optind];
	script  = argv[argc - 1];

	size_env = get_env_size();
	{
		size_t x = roundbyl(size_env, memfun_page_default);

		const uint32_t POSIX_ENV_HEADROOM = memfun_page_default / 2;
		if ((x - size_env) <= POSIX_ENV_HEADROOM)
			x += memfun_page_default;

		size_env = x;
	}
	size_args = get_arg_max() - size_env;
	// differs from "findutils" variant
	argc_max = (size_args / sizeof(size_t)) - 4;

	UVECTOR_CALL(string_v, init, &argv_init);
	UVECTOR_CALL(string_v, append, &argv_init, callee);
	for (int i = (optind + 1); i < (argc - 1); i++) {
		UVECTOR_CALL(string_v, append, &argv_init, argv[i]);
	}

	if ((argv_init.used >= size_args) || (argv_init.offsets.used >= argc_max)) {
		dump_error(E2BIG, "prepare()");
		exit(E2BIG);
	}
}

static void do_exec(void)
{
	if (argv_curr.offsets.used == argv_init.offsets.used)
		return;

	UVECTOR_CALL(string_v, free, &argv_init);

	int err = 0;
	const char * const * argv = UVECTOR_CALL(string_v, to_ptrlist, &argv_curr);
	execvp(argv[0], (char * const *) argv);
	// execution follows here in case of errors
	err = errno;
	dump_error(err, "execvp(3)");
	exit(err);
}

static void delete_script(void)
{
	if (!opt.Unlink_argfile) return;
	opt.Unlink_argfile = 0;

	struct stat l_stat;
	memset(&l_stat, 0, sizeof(l_stat));
	if (lstat(script, &l_stat) < 0) {
		dump_path_error(errno, "lstat(2)", script);
		return;
	}

	if (f_stat.st_dev != l_stat.st_dev) return;
	if (f_stat.st_ino != l_stat.st_ino) return;

	l_stat.st_mode &= S_IFMT;
	if (f_stat.st_mode != l_stat.st_mode) return;

	if (IFTODT(l_stat.st_mode) != DT_REG) return;

	unlink(script);
}

static void run(void)
{
	size_t s_buf_arg  = 32 * memfun_page_size();

	if (opt.Info_only) {
		fprintf(stderr, "System page size: %lu\n", memfun_page_size());
		fprintf(stderr, "Maximum (single) argument length: %lu\n", s_buf_arg);
		fprintf(stderr, "Environment size, as is: %lu\n", get_env_size());
		fprintf(stderr, "Environment size, round: %lu\n", size_env);
		fprintf(stderr, "Maximum arguments length, system:  %lu\n", get_arg_max());
		fprintf(stderr, "Maximum arguments length, current: %lu\n", size_args);
		fprintf(stderr, "Initial arguments length:          %lu\n", argv_init.used);
		fprintf(stderr, "Maximum argument count: %lu\n", argc_max);
		fprintf(stderr, "Initial argument count: %u\n", argv_init.offsets.used);
		return;
	}

	int err = 0;
	int fd = -1;

	size_t s_buf_read = s_buf_arg + memfun_page_size(); // s_buf_arg + one extra page
	char * buf_arg  = memfun_alloc(s_buf_arg);
	char * buf_read = memfun_alloc(s_buf_read);
	if ((!buf_arg) || (!buf_read)) {
		err = errno;
		if (!err) err = ENOMEM;
		goto _run_err;
	}

	if (!UVECTOR_CALL(string_v, dup, &argv_curr, &argv_init)) {
		err = errno;
		if (!err) err = ENOMEM;
		goto _run_err;
	}

	fd = open(script, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		err = errno;
		dump_path_error(err, "open(2)", script);
		exit(err);
	}

	memset(&f_stat, 0, sizeof(f_stat));
	if (fstat(fd, &f_stat) < 0) {
		err = errno;
		dump_path_error(err, "fstat(2)", script);
		exit(err);
	}
	f_stat.st_mode &= S_IFMT;

	if (!handle_file_type(IFTODT(f_stat.st_mode), script)) {
		err = EINVAL;
		exit(err);
	}

	size_t n_buf = 0, total = 0, block;
	ssize_t n_read = 0;
	char * tbuf;
	uint32_t arg_idx;
	int arg_pend = 0, exec_ready = 0;
	pid_t child;
	siginfo_t child_info;

	(void) memset(buf_arg, 0, s_buf_arg);

	for (;;) {
		if (arg_pend) {
			arg_idx = UVECTOR_CALL(string_v, append_fixed, &argv_curr, buf_arg, total);
			if (UVECTOR_CALL(ptroff_v, is_inv, arg_idx)) {
				err = errno;
				if (!err) err = ENOMEM;
				goto _run_out;
			}

			total = 0;
			arg_pend = 0;
			(void) memset(buf_arg, 0, s_buf_arg);
		}

		if (!n_buf) {
			(void) memset(buf_read, 0, s_buf_read);
			n_read = read(fd, buf_read, s_buf_read);
			if (n_read > 0) n_buf = (size_t) n_read;
			tbuf = buf_read;
		}

		while (n_buf > 0) {
			block = strnlen(tbuf, n_buf);
			total += block;

			if ((total + 1) >= s_buf_arg) {
				if (block == n_buf) {
					n_buf = 0;
					break;
				}

				block++; n_buf -= block; tbuf += block;

				total = 0;
				(void) memset(buf_arg, 0, s_buf_arg);

				continue;
			}

			(void) memcpy(buf_arg + total - block, tbuf, block);

			if (block == n_buf) {
				n_buf = 0;
				break;
			}

			block++; n_buf -= block; tbuf += block;

			if ((argv_curr.used + total + 1) >= size_args) {
				exec_ready = 1;
				arg_pend = 1;
				break;
			}

			arg_idx = UVECTOR_CALL(string_v, append_fixed, &argv_curr, buf_arg, total);
			if (UVECTOR_CALL(ptroff_v, is_inv, arg_idx)) {
				err = errno;
				if (!err) err = ENOMEM;
				goto _run_out;
			}

			total = 0;
			(void) memset(buf_arg, 0, s_buf_arg);

			if (argv_curr.offsets.used == argc_max) {
				exec_ready = 1;
				break;
			}
		}

		if (n_read <= 0) break;

		if (!exec_ready) continue;

		if (opt.Force_once) {
			err = E2BIG;
			goto _run_out;
		}

		child = fork();
		if (child == 0) do_exec();
		if (child == -1) {
			err = errno;
			if (!err) err = ENOMEM;
			goto _run_out;
		}

		// wait for child
		waitid(P_PID, child, &child_info, WEXITED);

		// do rest of work
		exec_ready = 0;

		// refine current argv
		UVECTOR_CALL(string_v, free, &argv_curr);
		if (!UVECTOR_CALL(string_v, dup, &argv_curr, &argv_init)) {
			err = errno;
			if (!err) err = ENOMEM;
			goto _run_out;
		}
	}

	close(fd); fd = -1;

	delete_script();

	do_exec();
	return;

_run_out:
	if (fd >= 0) close(fd);

	delete_script();

_run_err:
	dump_error(err, "run()");
	exit(err);
}

static int handle_file_type(uint32_t type, const char * arg)
{
	const char * e_type = NULL;
	switch (type) {
	case DT_BLK:  break;
	case DT_CHR:  break;
	case DT_FIFO: break;
	case DT_REG:  break;
	case DT_SOCK: break;
	case DT_DIR:  e_type = "directory";          break;
	case DT_LNK:  e_type = "symbolic link";      break;
	default:      e_type = "unknown entry type"; break;
	}

	if (!e_type) return 1;

	fprintf(stderr, "xvp: <arg file> %s is type of %s\n", arg, e_type);
	return 0;
}

static void dump_error(int error_num, const char * where)
{
	log_stderr_error_ex("xvp:", error_num, "%s", where);
}

static void dump_path_error(int error_num, const char * where, const char * name)
{
	log_stderr_path_error_ex("xvp:", name, error_num, "%s", where);
}
