/* ********************************************************************** */
/**
 * @brief   Command line tool for "head -c N && shift 1 byte" loop.
 * @author  eel3
 * @date    2015/11/05
 *
 * @par Compilers
 * - TDM-GCC 4.8.1 (Windows 7 64bit SP1)
 * - GCC 4.6.3 (Ubuntu/Linaro 4.6.3-1ubuntu5) on Ubuntu 12.04.5 LTS x86
 * - Apple LLVM 6.0 (clang-600.0.57, Xcode 6.2) on Mac OS X 10.9.5
 */
/* ********************************************************************** */


#ifdef __linux__
#	ifndef _POSIX_C_SOURCE
#		define _POSIX_C_SOURCE 199309L
#	endif /* ndef _POSIX_C_SOURCE */
#endif /* def __linux__ */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
#	include <fcntl.h>
#	include <io.h>
#	ifndef STDIN_FILENO
#		define STDIN_FILENO 0
#	endif
#	ifndef STDOUT_FILENO
#		define STDOUT_FILENO 1
#	endif
#endif /* defined(_WIN32) || defined(_WIN64) */


/* ---------------------------------------------------------------------- */
/* Boolean type */
/* ---------------------------------------------------------------------- */

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#	include <stdbool.h>
#else /* (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */
	typedef int bool;
#	define _Bool bool
#	define false ((bool) 0)
#	define true  ((bool) 1)
#endif /* (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */


/* ---------------------------------------------------------------------- */
/* Variable */
/* ---------------------------------------------------------------------- */

static const char *program_name;


/* ---------------------------------------------------------------------- */
/* Function-like macro */
/* ---------------------------------------------------------------------- */

/* ====================================================================== */
/**
 * @brief  Return true if string s1 equal to s2.
 *
 * @param[in] s1  string 1.
 * @param[in] s2  string 2.
 *
 * @retval !=0  Equal.
 * @retval   0  Not equal.
 */
/* ====================================================================== */
#define  STREQ(s1, s2)  (((s1)[0] == (s2)[0]) && (strcmp((s1), (s2)) == 0))


/* ---------------------------------------------------------------------- */
/* Queue type and method */
/* ---------------------------------------------------------------------- */

/** Queue type */
typedef struct {
	char *buf;
	size_t size;
	size_t rp;
	size_t wp;
} QUEUE;

/* ====================================================================== */
/**
 * @brief  Initialize queue.
 *
 * @param[in,out] *q    Queue.
 * @param[in]     size  Maximum size of queue.
 */
/* ====================================================================== */
static bool
queue_Initialize(QUEUE * const q, const size_t size)
{
	assert((q != NULL) && (size >= 1));

	q->size = size + 1;
	q->rp = q->wp = 0;

	errno = 0;
	q->buf = (char *) malloc(q->size);
	if (q->buf == NULL) {
		q->size = 0;
		return false;
	}

	return true;
}

/* ====================================================================== */
/**
 * @brief  Finalize queue.
 *
 * @param[in,out] *q  Queue.
 */
/* ====================================================================== */
static void
queue_Finalize(QUEUE * const q)
{
	assert(q != NULL);

	free(q->buf);
	q->buf = NULL;
	q->size = 0;
	q->rp = q->wp = 0;
}

/* ====================================================================== */
/**
 * @brief  Return the next pointer value.
 *
 * @param[in] *q   Queue.
 * @param[in] ptr  The current pointer value.
 *
 * @return  The next pointer value.
 */
/* ====================================================================== */
static size_t
queue_NextPtr(const QUEUE * const q, const size_t ptr)
{
	return (ptr + 1) % q->size;
}

/* ====================================================================== */
/**
 * @brief  Return true if queue is empty.
 *
 * @param[in] *q  Queue.
 *
 * @retval true   Queue is empty.
 * @retval false  Queue is not empty.
 */
/* ====================================================================== */
static bool
queue_Empty(const QUEUE * const q)
{
	assert(q != NULL);

	return q->rp == q->wp;
}

/* ====================================================================== */
/**
 * @brief  Return true if queue is full.
 *
 * @param[in] *q  Queue.
 *
 * @retval true   Queue is full.
 * @retval false  Queue is not full.
 */
/* ====================================================================== */
static bool
queue_Full(const QUEUE * const q)
{
	assert(q != NULL);

	return queue_NextPtr(q, q->wp) == q->rp;
}

/* ====================================================================== */
/**
 * @brief  Pushes the given element value to the end of queue.
 *
 * @param[in,out] *q     Queue.
 * @param[in]     value  The value of the element to push.
 *
 * @retval true   OK (success).
 * @retval false  NG (failure).
 */
/* ====================================================================== */
static bool
queue_Push(QUEUE * const q, const char value)
{
	assert(q != NULL);

	if (queue_Full(q)) {
		return false;
	}
	q->buf[q->wp] = value;
	q->wp = queue_NextPtr(q, q->wp);

	return true;
}

/* ====================================================================== */
/**
 * @brief  Removes an element from the front of the queue.
 *
 * @param[in,out] *q      Queue.
 * @param[out]    *value  The value of the element removed from queue.
 *
 * @retval true   OK (success)
 * @retval false  NG (failure).
 */
/* ====================================================================== */
static bool
queue_Pop(QUEUE * const q, char * const value)
{
	assert(q != NULL);

	if (queue_Empty(q)) {
		return false;
	}

	if (value != NULL) {
		*value = q->buf[q->rp];
	}
	q->rp = queue_NextPtr(q, q->rp);

	return true;
}

/* ====================================================================== */
/**
 * @brief  Print all elements in the queue.
 *
 * @param[in,out] *q    Queue.
 * @param[in,out] *out  Output stream.
 */
/* ====================================================================== */
static void
queue_Print(const QUEUE * const q, FILE * const out)
{
	size_t i;

	assert(q != NULL);

	for (i = q->rp; i != q->wp; i = queue_NextPtr(q, i)) {
		(void) fputc(q->buf[i], out);
	}
	(void) fputc('\n', out);
}


/* ---------------------------------------------------------------------- */
/* Function */
/* ---------------------------------------------------------------------- */

/* ====================================================================== */
/**
 * @brief  Convert from string to long int.
 *
 * @param[in]  *s       .
 * @param[in]  base     .
 * @param[out] *retval  .
 *
 * @retval true   OK (success)
 * @retval false  NG (failure).
 */
/* ====================================================================== */
static bool
to_l(const char * const s, const int base, long * const retval)
{
	long n;
	char *endp;

	assert((s != NULL)
	       && ((base == 0) || ((base >= 2) && (base <= 36)))
	       && (retval != NULL));

	errno = 0;
	n = strtol(s, &endp, base);
	if ((n == 0) && ((const char *) endp == s)) {
		return false;
	}
	if (((n == LONG_MAX) || (n == LONG_MIN)) && (errno == ERANGE)) {
		return false;
	}
	if (*endp != '\0') {
		return false;
	}

	*retval = n;

	return true;
}

/* ====================================================================== */
/**
 * @brief  .
 *
 * @param[in] *s      .
 * @param[in] *chars  .
 *
 * @return  .
 */
/* ====================================================================== */
static size_t
strrcspn(const char * const s, const char * const chars)
{
	const char *begin, *p;

	assert((s != NULL) && (chars != NULL));

	begin = NULL;
	for (p = s; *p != '\0'; p++) {
		const char *q;

		for (q = chars; *q != '\0'; q++) {
			if (*q == *p) {
				break;
			}
		}
		if (begin == NULL) {
			if (*q != '\0') {
				begin = p;
			}
		} else {
			if (*q == '\0') {
				begin = NULL;
			}
		}
	}

	return (begin == NULL) ? (p - s) : (begin - s);
}

/* ====================================================================== */
/**
 * @brief  Remove all trailing characters contained in chars from string s.
 *
 * @param[in] *s      .
 * @param[in] *chars  .
 *
 * @return  .
 */
/* ====================================================================== */
static void
trim_right(char * const s, const char * const chars)
{
	size_t rpos;

	assert((s != NULL) && (chars != NULL));

	rpos = strrcspn(s, chars);
	s[rpos] = '\0';
}

/* ====================================================================== */
/**
 * @brief  Tiny copy of basename(3).
 *
 * @param[in] *s  (See man 3 basename).
 *
 * @return  (See man 3 basename).
 */
/* ====================================================================== */
static const char *
my_basename(char * const name)
{
#define DOT "."
#define SEP_S "/"
#define SEP_C '/'
#if defined(_WIN32) || defined(_WIN64)
#	define WSEP_S "\\"
#	define WSEP_C '\\'
#	define SEPS SEP_S WSEP_S
#else /* defined(_WIN32) || defined(_WIN64) */
#	define SEPS SEP_S
#endif /* defined(_WIN32) || defined(_WIN64) */

	const char *bn;

	if ((name == NULL) || (name[0] == '\0')) {
		return DOT;
	}

	if (STREQ(name, SEP_S)) {
		return name;
	}
#if defined(_WIN32) || defined(_WIN64)
	if (STREQ(name, WSEP_S)) {
		return name;
	}
#endif /* defined(_WIN32) || defined(_WIN64) */

	trim_right(name, SEPS);

	bn = strrchr(name, SEP_C);
#if defined(_WIN32) || defined(_WIN64)
	if (bn == NULL) {
		bn = strrchr(name, WSEP_C);
	}
#endif /* defined(_WIN32) || defined(_WIN64) */
	return (bn == NULL) ? name : bn+1;

#undef SEPS
#if defined(_WIN32) || defined(_WIN64)
#	undef WSEP_C
#	undef WSEP_S
#endif /* defined(_WIN32) || defined(_WIN64) */
#undef SEP_C
#undef SEP_S
#undef DOT
}

/* ====================================================================== */
/**
 * @brief  Print the usage.
 *
 * @param[in,out] *out  Output stream.
 */
/* ====================================================================== */
static void
usage(FILE * const out)
{
	assert(out != NULL);

	(void) fprintf(out,
	               "usage: %s [options] [file...]\n"
	               "    -n N\n"
	               "     print the N bytes per line (N >= 1)\n"
	               "    -o FILE\n"
	               "     place output in file FILE\n",
	               program_name);
}

/* ====================================================================== */
/**
 * @brief  Print the version.
 */
/* ====================================================================== */
static void
version(void)
{
	(void) fprintf(stdout, "%s 1.0.0\n", program_name);
}

/* ====================================================================== */
/**
 * @brief  "head -c && shift 1 byte" loop.
 *
 * @param[in,out] *in   Input stream.
 * @param[in,out] *out  Output stream.
 * @param[in,out] *buf  Working buffer.
 */
/* ====================================================================== */
static void
hcasl(FILE * const in, FILE * const out, QUEUE * const buf)
{
	int c;

	assert((in != NULL) && (out != NULL) && (buf != NULL));

	while ((c = fgetc(in)) != EOF) {
		queue_Push(buf, (char) (unsigned char) c);
		if (queue_Full(buf)) {
			queue_Print(buf, out);
			queue_Pop(buf, NULL);
		}
	}
}

/* ********************************************************************** */
/**
 * @brief  Main routine.
 *
 * @retval EXIT_SUCCESS  OK (success).
 * @retval EXIT_FAILURE  NG (failure).
 */
/* ********************************************************************** */
int
main(int argc, char *argv[])
{
	size_t bytes;
	const char *output;
	int c;
	FILE *out;
	int retval;
	QUEUE buf;

	program_name = my_basename(argv[0]);

#if defined(_WIN32) || defined(_WIN64)
	errno = 0;
	if (_setmode(STDIN_FILENO, O_BINARY) == -1) {
		perror("_setmode");
		return EXIT_FAILURE;
	}
	errno = 0;
	if (_setmode(STDOUT_FILENO, O_BINARY) == -1) {
		perror("_setmode");
		return EXIT_FAILURE;
	}
#endif /* defined(_WIN32) || defined(_WIN64) */

	bytes = 8;
	output = "-";

	while ((c = getopt(argc, argv, "hn:o:v")) != -1) {
		switch (c) {
		case 'h':
			usage(stdout);
			return EXIT_SUCCESS;
		case 'n':
			{
				long n;
				if (!to_l(optarg, 0, &n) || (n <= 0)) {
					usage(stderr);
					return EXIT_FAILURE;
				}
				bytes = (size_t) n;
			}
			break;
		case 'o':
			output = optarg;
			break;
		case 'v':
			version();
			return EXIT_SUCCESS;
		default:
			usage(stderr);
			return EXIT_FAILURE;
		}
	}

	if (STREQ(output, "-")) {
		out = stdout;
	} else if (errno = 0, (out = fopen(output, "wb")) == NULL) {
		perror(output);
		return EXIT_FAILURE;
	} else {
		/*EMPTY*/
	}

	retval = EXIT_SUCCESS;

	if (!queue_Initialize(&buf, bytes)) {
		perror(program_name);
		retval = EXIT_FAILURE;
		goto DONE;
	}

	if (optind >= argc) {
		hcasl(stdin, out, &buf);
	} else {
		int i;
		FILE *in;

		for (i = optind; i < argc; i++) {
			if (STREQ(argv[i], "-")) {
				in = stdin;
			} else if (errno = 0, (in = fopen(argv[i], "rb")) == NULL) {
				perror(argv[i]);
				retval = EXIT_FAILURE;
				continue;
			} else {
				/*EMPTY*/
			}

			hcasl(in, out, &buf);

			if (in != stdin) {
				(void) fclose(in);
			}
		}
	}

	queue_Finalize(&buf);

DONE:
	if (out != stdout) {
		(void) fclose(out);
	}

	return retval;
}
