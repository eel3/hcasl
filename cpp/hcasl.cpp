/* ********************************************************************** */
/**
 * @brief   Command line tool for "head -c N && shift 1 byte" loop.
 * @author  eel3
 * @date    2015/11/05
 *
 * @par Compilers
 * - TDM-GCC 4.8.1 (Windows 7 64bit SP1)
 * - Apple LLVM 6.0 (clang-600.0.57, Xcode 6.2) on Mac OS X 10.9.5
 */
/* ********************************************************************** */


#ifdef __linux__
#	ifndef _POSIX_C_SOURCE
#		define _POSIX_C_SOURCE 199309L
#	endif /* ndef _POSIX_C_SOURCE */
#endif /* def __linux__ */

#include <cassert>
#include <cstdlib>

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
#	include <cerrno>
#	include <fcntl.h>
#	include <io.h>
#	ifndef STDIN_FILENO
#		define STDIN_FILENO 0
#	endif
#	ifndef STDOUT_FILENO
#		define STDOUT_FILENO 1
#	endif
#endif /* defined(_WIN32) || defined(_WIN64) */


namespace {

/* ---------------------------------------------------------------------- */
/* Variable */
/* ---------------------------------------------------------------------- */

std::string program_name;


/* ---------------------------------------------------------------------- */
/* Function */
/* ---------------------------------------------------------------------- */

/* ====================================================================== */
/**
 * @brief  Returns a slice of the string s,
 *         with all trailing characters contained in chars removed.
 *
 * @param[in] s      .
 * @param[in] chars  .
 *
 * @return  .
 */
/* ====================================================================== */
std::string
trim_right(const std::string &s, const std::string &chars = "\t\n\v\f\r ")
{
	auto rpos = s.find_last_not_of(chars);

	return (rpos == std::string::npos) ? std::string() : s.substr(0, rpos + 1);
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
std::string
my_basename(const char * const s)
{
	using std::string;

	static const string DOT = ".";
	static const string SEP = "/";
#if defined(_WIN32) || defined(_WIN64)
	static const string WSEP = "\\";
	static const string SEPS = SEP + WSEP;
#else /* defined(_WIN32) || defined(_WIN64) */
	static const string SEPS = SEP;
#endif /* defined(_WIN32) || defined(_WIN64) */

	if (s == NULL) {
		return DOT;
	}

	string path = s;

	if (path.empty()) {
		return DOT;
	}
	if (path == SEP) {
		return SEP;
	}
#if defined(_WIN32) || defined(_WIN64)
	if (path == WSEP) {
		return WSEP;
	}
#endif /* defined(_WIN32) || defined(_WIN64) */

	path = trim_right(path, SEPS);

	auto pos = path.find_last_of(SEPS);
	return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

/* ====================================================================== */
/**
 * @brief  Print the usage.
 *
 * @param[in,out] out  Output stream.
 */
/* ====================================================================== */
void
usage(std::ostream &out)
{
	out << "usage: " << program_name << " [options] [file...]\n"
	    << "    -n N\n"
	    << "     print the N bytes per line (N >= 1)\n"
	    << "    -o FILE\n"
	    << "     place output in file FILE" << std::endl;
}

/* ====================================================================== */
/**
 * @brief  Print the version.
 */
/* ====================================================================== */
void
version(void)
{
	std::cout << program_name << " 1.0.0" << std::endl;
}

/* ====================================================================== */
/**
 * @brief  "head -c && shift 1 byte" loop.
 *
 * @param[in,out] in     Input stream.
 * @param[in,out] out    Output stream.
 * @param[in]     bytes  `head -c <bytes>`.
 * @param[in,out] buf    Working buffer.
 */
/* ====================================================================== */
void
hcasl(std::istream &in, std::ostream &out, const unsigned long bytes, std::deque<char> &buf)
{
	int c;

	while ((c = in.get()) != std::char_traits<char>::eof()) {
		buf.push_back(static_cast<char>(static_cast<unsigned char>(c)));
		if (buf.size() >= bytes) {
			std::for_each(buf.cbegin(), buf.cend(), [&out] (const char c) {
				out << c;
			});
			out << std::endl;
			buf.pop_front();
		}
	}
}

} // namespace

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
	using std::cerr;
	using std::cin;
	using std::cout;
	using std::endl;
	using std::ios;
	using std::string;

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

	unsigned long bytes = 8;
	string output  = "-";

	int c;
	while ((c = getopt(argc, argv, "hn:o:v")) != -1) {
		switch (c) {
		case 'h':
			usage(cout);
			return EXIT_SUCCESS;
		case 'n':
			{
				std::istringstream nbuf(optarg);
				long n;
				nbuf >> n;
				if (!nbuf || (n <= 0)) {
					usage(cerr);
					return EXIT_FAILURE;
				}
				bytes = static_cast<unsigned long>(n);
			}
			break;
		case 'o':
			output = optarg;
			break;
		case 'v':
			version();
			return EXIT_SUCCESS;
		default:
			usage(cerr);
			return EXIT_FAILURE;
		}
	}

	bool use_stdout = output == "-";
	std::ofstream fout;
	if (!use_stdout) {
		fout.open(output, ios::binary);
		if (!fout) {
			cerr << program_name << ": " << output << ": cannot open" << endl;
			return EXIT_FAILURE;
		}
	}
	std::ostream &out = use_stdout ? cout : fout;

	std::deque<char> buf;
	int retval = EXIT_SUCCESS;

	if (optind >= argc) {
		hcasl(cin, out, bytes, buf);
	} else {
		std::for_each(&argv[optind], &argv[argc], [&out, &retval, &bytes, &buf] (const char * const s) {
			assert(s != NULL);
			string arg = s;

			if (arg == "-") {
				hcasl(cin, out, bytes, buf);
			} else {
				std::ifstream fin(arg, ios::binary);
				if (!fin) {
					cerr << program_name << ": " << arg << ": cannot open" << endl;
					retval = EXIT_FAILURE;
					return;
				}
				hcasl(fin, out, bytes, buf);
			}
		});
	}

	return retval;
}
