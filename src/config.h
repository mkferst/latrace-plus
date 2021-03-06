/*
  Copyright (C) 2008, 2009, 2010 Jiri Olsa <olsajiri@gmail.com>

  This file is part of the latrace.

  The latrace is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  The latrace is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the latrace (file COPYING).  If not, see 
  <http://www.gnu.org/licenses/>.
*/


#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <search.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "audit.h"
#include "list.h"


#define TRANSFORMER_CRASH_PROTECTION 1
#define TRANSFORMER_CRASH_PROTECTION_ENHANCED 1

#ifdef TRANSFORMER_CRASH_PROTECTION
#include <signal.h>
#include <setjmp.h>
#include <ucontext.h>
#endif

#define STATIC static
//#define STATIC 

#define CODE_LOC_LA_TRANSFORMER	1
#define CODE_LOC_LA_INTERCEPT	2
#define CODE_LOC_LA_VERSION	10
#define CODE_LOC_LA_OBJOPEN	11
#define CODE_LOC_LA_SYMBIND	12
#define CODE_LOC_LA_ACTIVITY	13
#define CODE_LOC_LA_OBJSEARCH	14
#define CODE_LOC_LA_PREINIT	15
#define CODE_LOC_LA_OBJCLOSE	16
#define CODE_LOC_LA_SYMBIND_NATIVE	17
#define CODE_LOC_LA_PLTENTER	18
#define CODE_LOC_LA_PLTEXIT	19

typedef struct function_call {
	char *fn_name;
	La_regs registers;
	void **args;
	size_t argcnt;
} fn_call_t;

#define SUPPRESS_FN_LEN		128
typedef struct lt_tsd {
	char suppress_while[SUPPRESS_FN_LEN];
	int suppress_collapsed;
	int suppress_nested;
	size_t nsuppressed;

#ifdef TRANSFORMER_CRASH_PROTECTION
	jmp_buf crash_insurance;
	int jmp_set;
	const char *last_symbol;
#endif
	int last_operation;
	char *fault_reason;

	int ass_integer;
	int ass_sse;
	int ass_memory;

	fn_call_t *xfm_call_stack;
	size_t xfm_call_stack_max;
	size_t xfm_call_stack_sz;

	int *errno_loc;

	int pipe_fd;
	int indent_depth;
	int flow_below_stack;

	char *excised;

	void *stack_start;
	void *stack_end;

	int is_new;
} lt_tsd_t;

#define TSD_SET(memb,val)	do { if (tsd) tsd->memb = val; } while (0)
#define TSD_GET(memb,defval)	(tsd ? tsd->memb : defval)
extern lt_tsd_t *thread_get_tsd(int create);


#ifdef CONFIG_ARCH_HAVE_ARGS
#include "args.h"
#endif

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#define LT_NAMES_MAX  50
#define LT_NAMES_SEP  ','

#define LT_SYM_HMAX    1000

#define LT_ARGS_DEF_STRUCT_NUM  1000
#define LT_ARGS_DEF_TYPEDEF_NUM 1000
#define LT_ARGS_DEF_ENUM_NUM    1000
#define LT_ARGS_STRUCT_XFM_NUM  1000


enum { 
	LT_CSORT_TIME = 0,
	LT_CSORT_PERCENT, 
	LT_CSORT_CALL, 
	LT_CSORT_UCALL, 
	LT_CSORT_LIB,
	LT_CSORT_SYM
};

struct lt_config_tv {
	int type;
	char *name;
};

enum {
	LT_OPT_HEADERS = 1,
	LT_OPT_PIPE,
	LT_OPT_INDENT_SYM,
	LT_OPT_TIMESTAMP,
	LT_OPT_FRAMESIZE,
	LT_OPT_FRAMESIZE_CHECK,
	LT_OPT_HIDE_TID,
	LT_OPT_FOLLOW_FORK,
	LT_OPT_FOLLOW_EXEC,
	LT_OPT_DEMANGLE,
	LT_OPT_FORMATTING,
	LT_OPT_BRACES,
	LT_OPT_ENABLE_ARGS,
	LT_OPT_DETAIL_ARGS,
	LT_OPT_OUTPUT_TTY,
	LT_OPT_LIBS,
	LT_OPT_LIBS_TO,
	LT_OPT_LIBS_FROM,
	LT_OPT_SYM,
	LT_OPT_SYM_OMIT,
	LT_OPT_SYM_BELOW,
	LT_OPT_SYM_NOEXIT,
	LT_OPT_ARGS_STRING_POINTER_LENGTH,
	LT_OPT_RUN_IN_GDB,
};

struct lt_config_opt {
	int idx;
	char *sval;
	unsigned long nval;
	struct lt_list_head list;
};

struct lt_config_shared {
#define LT_CONFIG_VERSION	1
#define LT_CONFIG_MAGIC		((LT_CONFIG_VERSION << 16) + 0xdead)
	unsigned int magic;

#define LT_LIBS_MAXSIZE     4096
	char libs_to[LT_LIBS_MAXSIZE];
	char libs_both[LT_LIBS_MAXSIZE];
	char libs_from[LT_LIBS_MAXSIZE];

	char libs_subst[LT_LIBS_MAXSIZE];

#define LT_SYMBOLS_MAXSIZE  4096
	char symbols[LT_SYMBOLS_MAXSIZE];
	char symbols_omit[LT_SYMBOLS_MAXSIZE];
	char symbols_noexit[LT_SYMBOLS_MAXSIZE];

	char flow_below[LT_SYMBOLS_MAXSIZE];

#define LT_MAXFILE 4096
	char output[LT_MAXFILE];
	FILE *fout;

	char args_def[LT_MAXFILE];
	char args_enabled;
	char args_detailed;
	char args_string_pointer_length;
#define LR_ARGS_MAXLEN 1000
	int  args_maxlen;
#define LR_ARGS_DETAIL_MAXLEN 1000
	int  args_detail_maxlen;
#define LT_ARGS_TAB 10000
	struct hsearch_data args_tab;

	int disabled;
	int ctl_config;
	int verbose;
	int timestamp;
	int debug;
	int run_in_gdb;
	int indent_sym;
	int indent_size;
	int braces;
	int demangle;
	int lib_short;
	int src_lib_pfx;
	int fmt_colors;
	int reset_on_jmp;
	int resolve_syms;
	int counts;
	int pipe;
	int hide_tid;
	int not_follow_exec;
	int not_follow_fork;
	int framesize_check;
	unsigned int framesize;
	int global_symbols;

	/* for 'not_follow_fork' */
	pid_t pid;

	/* XXX feel like an idiot.. find another way!!! */
	struct lt_config_shared *sh;
};

struct lt_config_app {
	/*
	 * This is to copy the lt_config_audit, so we can use
	 * one PRINT_VERBOSE only.
	 */
	struct lt_config_shared *sh;
	struct lt_config_shared sh_storage;

	char *prog;
#define LT_NUM_ARG 500
	char *arg[LT_NUM_ARG];
	int arg_num;

	int csort;

	int  output_tty;
	int  output_tty_fd;
	char output_tty_file[LT_MAXFILE];

	struct lt_thread *threads;
	struct lt_thread *iter;
};

struct lt_config_ctl {
	struct lt_config_shared *sh;
	char *config;

	int set_disabled;
	int disabled;
};

enum {
	LT_OS_PATH = 1,  /* '=' */
	LT_OS_PTN,       /* '%' */
	LT_OS_PTN2PATH,  /* '~' */
};

struct lt_objsearch {
	int   type;
	char *src;
	char *dst;
};

struct lt_config_audit {

	/*
	 * Normally sh points to the sh_storage. When using
	 * ctl-config feature, the shared config is stored
	 * in mmaped area.
        */
	struct lt_config_shared *sh;
	struct lt_config_shared sh_storage;

	char *libs_to[LT_NAMES_MAX];
	int libs_to_cnt;

	char *libs_from[LT_NAMES_MAX];
	int libs_from_cnt;

	char *libs_both[LT_NAMES_MAX];
	int libs_both_cnt;

	char *symbols[LT_NAMES_MAX];
	int symbols_cnt;

	char *symbols_omit[LT_NAMES_MAX];
	int symbols_omit_cnt;

	char *symbols_noexit[LT_NAMES_MAX];
	int symbols_noexit_cnt;

	char *flow_below[LT_NAMES_MAX];
	int flow_below_cnt;

	struct lt_objsearch subst[LT_NAMES_MAX];
	int subst_cnt;

	char *dir;
	int init_ok;
};

/* config - list name support */
struct lt_config_ln {
	char *name;
	struct lt_list_head list;
};

#define lt_sh(cfg, field) ((cfg)->sh->field)

#define FIFO_MSG_MAXLEN       2000

/* common message data */
struct lt_fifo_mbase {
#define FIFO_MSG_TYPE_ENTRY   1
#define FIFO_MSG_TYPE_EXIT    2
	uint32_t type;
	struct timeval tv;
	pid_t tid;
	int len; /* the rest of the message size */
};

#define COLLAPSED_NONE        0
#define COLLAPSED_BASIC	      1
#define COLLAPSED_TERSE       2
#define COLLAPSED_BARE        3
#define COLLAPSED_NESTED      16

/* symbol message */
struct lt_fifo_msym {
	struct lt_fifo_mbase h;

	int sym;
	int lib_to;
	int lib_from;
	int arg;
	int argd;
	int collapsed;
	char data[0];
};

struct lt_stats_sym {
        char *name;
        char *sym;
        char *lib;

        struct timeval tv_cur;
        struct timeval tv_all;

        unsigned int call;

        /* post mortem statistics */
        float percent;
        unsigned int usec_call;
};

struct lt_thread {
	/* global */
	int fifo_fd;
        pid_t tid;

	int indent_depth;
	size_t nsuppressed;

	/* start/stop time */
	struct timeval tv_start;
	struct timeval tv_stop;

	/* symbol statistics */
        struct lt_stats_sym **sym_array;
        struct hsearch_data sym_htab;
        unsigned int sym_cnt;
        unsigned int sym_max;

	struct lt_thread *next;
};

struct lt_symbol {
	struct lt_args_sym *args;

	/* symbol name */
	const char *name;
	/* symbol address */
	void *ptr;
	int collapsed;
};

/* ctl */
int main_ctl(int argc, char **argv);

/* global */
const char *lt_config(struct lt_config_app *cfg, int argc, char **argv);
int lt_run(struct lt_config_app *cfg);

/* fifo */
int lt_fifo_create(struct lt_config_audit *cfg, char *dir);
int lt_fifo_open(struct lt_config_app *cfg, char *dir, char *name);
int lt_fifo_notify_fd(struct lt_config_app *cfg, char *dir);
int lt_fifo_send(struct lt_config_audit *cfg, int fd, char *buf, int len);
int lt_fifo_recv(struct lt_config_app *cfg, struct lt_thread *t, 
		void *buf, int bufsize);
int lt_fifo_msym_get(struct lt_config_audit *cfg, char *buf, int type,
			struct timeval *tv, char *symname, char *libto,
			char *libfrom, char *arg, char *argd, int collapsed);

/* counts */
int lt_stats_init(struct lt_config_app *cfg);
int lt_stats_sym(struct lt_config_app *cfg, struct lt_thread *t, 
		struct lt_fifo_msym* m);
int lt_stats_alloc(struct lt_config_app *cfg, struct lt_thread *t);
int lt_stats_show(struct lt_config_app *cfg);

/* thread */
struct lt_thread *lt_thread_add(struct lt_config_app *cfg, int fd, pid_t pid);
struct lt_thread *lt_thread_first(struct lt_config_app *cfg);
struct lt_thread *lt_thread_next(struct lt_config_app *cfg);

/* output */
int lt_out_entry(struct lt_config_shared *cfg, struct timeval *tv,
		pid_t tid, int indent_depth, int collapsed,
		const char *symname, char *lib_to, char *lib_from,
		char *argbuf, char *argdbuf, size_t *nsuppressed);
int lt_out_exit(struct lt_config_shared *cfg, struct timeval *tv,
		pid_t tid, int indent_depth, int collapsed,
		const char *symname, char *lib_to, char *lib_from,
		char *argbuf, char *argdbuf, size_t *nsuppressed);

/* la_objsearch */
int lt_objsearch_init(struct lt_config_audit *cfg, char **ptr, int cnt);
char* lt_objsearch(struct lt_config_audit *cfg, const char *name, 
		uintptr_t *cookie, unsigned int flag);

/* stack */
int lt_stack_framesize(struct lt_config_audit *cfg, La_regs *regs, lt_tsd_t *tsd);

/* symbol */
struct lt_symbol* lt_symbol_bind(struct lt_config_shared *cfg,
				void *ptr, const char *name);
struct lt_symbol* lt_symbol_get(struct lt_config_shared *cfg,
				void *ptr, const char *name);

/* config options */
struct lt_config_opt *lt_config_opt_new(struct lt_config_app *cfg,
					int idx, char *sval, long nval);
int lt_config_opt_process(struct lt_config_app *cfg, struct lt_list_head *list);
int lt_config_ln_add(struct lt_list_head *head, char *name);
int lt_config_ln_fill(struct lt_list_head *head, char *buf, int size);

/* tty */
int tty_master(struct lt_config_app *cfg);
int tty_init(struct lt_config_app *cfg, int master);
int tty_restore(struct lt_config_app *cfg);
int tty_process(struct lt_config_app *cfg, int master);
void tty_close(struct lt_config_app *cfg);

#define PRINT(fmt, args...) \
do { \
	char lpbuf[1024]; \
	sprintf(lpbuf, "[%d %s:%05d] %s", \
		(pid_t) syscall(SYS_gettid), \
		__FUNCTION__, \
		__LINE__, \
		fmt); \
	printf(lpbuf, ## args); \
	fflush(NULL); \
} while(0)

#define PRINT_VERBOSE(cfg, cond, fmt, args...) \
do { \
	if (cond > (cfg)->sh->verbose) \
		break; \
	PRINT(fmt, ## args); \
} while(0)

#define RESET   "\033[0m"
#define BOLD	"\033[1m"
#define BOLDOFF	"\033[22m"
#define INVERT	"\033[7m"
#define INVOFF	"\033[27m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define PRINT_COLOR(color, fmt, ...)	fprintf(stderr, color fmt RESET, __VA_ARGS__)
//#define PRINT_ERROR(fmt, ...)		PRINT_COLOR(BOLDRED, fmt, __VA_ARGS__)
#define PRINT_ERROR	PRINT_ERROR_SAFE
#define PRINT_ERROR_SAFE(fmt, ...)	do { char buf[1024]; memset(buf, 0, sizeof(buf));	\
					snprintf(buf, sizeof(buf), BOLDRED fmt RESET, __VA_ARGS__);	\
					if (write(STDERR_FILENO, buf, strlen(buf))) { }  \
					fsync(STDERR_FILENO); } while (0)
#define PERROR(func)	do {	\
				char errbuf[256];	\
				memset(errbuf, 0, sizeof(errbuf));	\
				if (strerror_r(errno, errbuf, sizeof(errbuf))) { }	\
				PRINT_ERROR("%s: %s", func, errbuf);	\
			} while (0)
#define PERROR_PRINTF(fmt,...)	do {	\
						char msgbuf[256];	\
						ssize_t max = sizeof(msgbuf);	\
						memset(msgbuf, 0, sizeof(msgbuf));	\
						max -= snprintf(msgbuf, sizeof(msgbuf), fmt, __VA_ARGS__);	\
						if (max > 3) {	\
							char errbuf[256];	\
							memset(errbuf, 0, sizeof(errbuf));	\
							if (strerror_r(errno, errbuf, sizeof(errbuf))) { }	\
							snprintf(&msgbuf[strlen(msgbuf)], max, ": %s\n", errbuf);	\
							if (write(STDERR_FILENO, msgbuf, strlen(msgbuf))) { }	\
							fsync(STDERR_FILENO);	\
						}	\
					} while (0)


//#define USE_GLIBC_FEATURES	1
#define USE_LIBUNWIND	1

#ifdef TRANSFORMER_CRASH_PROTECTION
#ifdef USE_LIBUNWIND
extern void backtrace_unwind(ucontext_t *start_context);
#endif
#endif

extern int glibc_unsafe;

extern void *xxmalloc(size_t size);
extern void *xxrealloc(void *ptr, size_t size);
extern char *xxstrdup(const char *s);
extern void _print_backtrace(void);

extern void *safe_malloc(size_t size);
extern void *safe_realloc(void *ptr, size_t size);
extern void safe_free(void *ptr);
extern char *safe_strdup(const char *s);

#define SANITY_CHECK(ret)		if (!ret) { PRINT_ERROR("%s", "Fatal internal memory error; allocation returned NULL.\n"); }

#ifdef USE_GLIBC_FEATURES
	#define XMALLOC_ASSIGN(val,parm)	do { val = xxmalloc(parm); SANITY_CHECK(val); } while (0)
	#define XREALLOC_ASSIGN(val,p1,p2)	do { val = xxrealloc(p1,p2); SANITY_CHECK(val); } while (0)
	#define XSTRDUP_ASSIGN(val,parm)	do { val = xxstrdup(parm); SANITY_CHECK(val); } while (0)
	#define XFREE(parm)			free(parm)
#else
	#define SAFETY_WARNING(func)		if (glibc_unsafe) {	\
							PRINT_ERROR("Warning: potentially unsafe call to %s() from line %d, file %s\n", func, __LINE__, __FILE__); 	\
							_print_backtrace(); \
						}
	#define XMALLOC_ASSIGN(val,parm)	do {	\
							SAFETY_WARNING("malloc");	\
							val = xxmalloc(parm); \
							SANITY_CHECK(val); } while (0)
	#define XREALLOC_ASSIGN(val,p1,p2)	do {	\
							SAFETY_WARNING("realloc");	\
							val = xxrealloc(p1,p2); \
							SANITY_CHECK(val); } while (0)
	#define XSTRDUP_ASSIGN(val,parm)	do {	\
							SAFETY_WARNING("strdup");	\
							val = xxstrdup(parm); \
							SANITY_CHECK(val); } while (0)
	#define XFREE(parm)			do {	\
							SAFETY_WARNING("free");	\
							free(parm); } while (0)
#endif


/* libiberty external */
extern char* cplus_demangle(const char *mangled, int options);

#ifdef CONFIG_LIBERTY
#define DEMANGLE(sym, d) \
do { \
	char *dem; \
	dem = cplus_demangle(sym, 0); \
	if (dem) { \
		sym = dem; \
		d = 1; \
	} \
} while(0)
#else
#define DEMANGLE(sym, d)
#endif

extern int _safe_demangle(const char *symname, char *buf, size_t bufsize);


#define ANON_PREFIX	"_anon_"


#if __WORDSIZE == 64
#define ELF_DYN		Elf64_Dyn
#define ELF_SYM		Elf64_Sym
#define ELF_ST_TYPE	ELF64_ST_TYPE
#else
#define ELF_DYN		Elf32_Dyn
#define ELF_SYM		Elf32_Sym
#define ELF_ST_TYPE	ELF32_ST_TYPE
#endif



#if defined(__x86_64)
#include "sysdeps/x86_64/args.h"
#endif

#define IGN_RET(x)	{ if (x) {} }

#endif // !CONFIG_H
