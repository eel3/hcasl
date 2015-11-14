/* ********************************************************************** */
/**
 * @brief   Command line tool for "head -c N && shift 1 byte" loop.
 * @author  eel3
 * @date    2015/11/13
 *
 * @par Compilers
 * - DMD 2.069.0 Windows/32bit code
 */
/* ********************************************************************** */

import std.stdio;

void hcasl(File istream, File ostream, const int bytes, ref ubyte[] buf)
{
    import std.array;

    foreach (ubyte[] c; istream.byChunk(1))
    {
        buf.insertInPlace(buf.length, c[0]);
        if (buf.length >= bytes)
        {
            ostream.rawWrite(buf);
            ostream.write('\n');
            buf.popFront();
        }
    }
}

int main(string[] args)
{
    import core.stdc.stdlib;
    import std.getopt;
    import std.path;

    auto progname = stripExtension(baseName(args[0]));
    auto usage = "usage: " ~ progname ~ " [options] [file...]";

    auto bytes = 8;
    auto output = "-";
    auto versionWanted = false;

    try
    {
        auto help = getopt(
            args, config.bundling,
            "bytes|n",   "print the N bytes per line (N >= 1)",    &bytes,
            "output|o",  "place output in file, or stdout (`-')",  &output,
            "version|v", "show program's version number and exit", &versionWanted
        );
        if (help.helpWanted)
        {
            defaultGetoptPrinter(usage, help.options);
            return EXIT_SUCCESS;
        }

        if (versionWanted)
        {
            writeln(progname ~ " 1.0.0");
            return EXIT_SUCCESS;
        }

        if (bytes <= 0)
        {
            defaultGetoptFormatter(stderr.lockingTextWriter(), usage,
                help.options);
            return EXIT_FAILURE;
        }

        auto ostream = (output == "-") ? stdout : File(output, "wb");
        auto files = (args.length == 1) ? [ "-" ] : args[1..$];
        ubyte[] buf = [];
        auto retval = EXIT_SUCCESS;

        foreach (input; files)
        {
            try
            {
                auto istream = (input == "-") ? stdin : File(input);
                hcasl(istream, ostream, bytes, buf);
            }
            catch (Exception e)
            {
                stderr.writeln(e.msg);
                retval = EXIT_FAILURE;
            }
        }

        return retval;
    }
    catch (Exception e)
    {
        stderr.writeln(e.msg);
        return EXIT_FAILURE;
    }
}
