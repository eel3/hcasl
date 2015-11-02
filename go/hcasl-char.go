/* ********************************************************************** */
/**
 * @brief   Command line tool for "head -c N && shift 1 char" loop.
 * @author  eel3
 * @date    2015/11/02
 *
 * @par Compilers
 * - Go 1.4.2 windows/amd64
 */
/* ********************************************************************** */

package main

import (
	"container/list"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

const (
	exit_success = iota // EXIT_SUCCESS
	exit_failure        // EXIT_FAILURE
)

func dumpList(out *os.File, lst *list.List) {
	for v := lst.Front(); v != nil; v = v.Next() {
		fmt.Fprintf(out, "%c", v.Value)
	}
	fmt.Fprintln(out)
}

// "head -c && shift 1 char" loop.
func hcasl(in *os.File, out *os.File, chars int, buf *list.List) {
	var c int
	for {
		n, err := fmt.Fscanf(in, "%c", &c)
		if n == 0 && err == io.EOF {
			break
		}
		buf.PushBack(c)
		if buf.Len() >= chars {
			dumpList(out, buf)
			buf.Remove(buf.Front())
		}
	}
}

// Read from File and do hcasl.
func hcaslFromFile(infile string, out *os.File, chars int, buf *list.List) error {
	var in *os.File

	if infile == "-" {
		in = os.Stdin
	} else {
		var err error
		in, err = os.Open(infile)
		if err != nil {
			return err
		}
		defer in.Close()
	}

	hcasl(in, out, chars, buf)
	return nil
}

// Main routine.
func main() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "usage: %s [options] [file ...]\n", filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
	chars := flag.Int("n", 8, "print the N characters per line (N >= 1)")
	output := flag.String("o", "-", "place output in file, or stdout (`-')")
	version := flag.Bool("v", false, "show program's version number and exit")
	flag.Parse()

	if *version {
		fmt.Fprintf(os.Stderr, "%s 1.0.0\n", filepath.Base(os.Args[0]))
		os.Exit(exit_success)
	}

	if *chars <= 0 {
		flag.Usage()
		os.Exit(exit_failure)
	}

	var out *os.File
	if *output == "-" {
		out = os.Stdout
	} else {
		var err error
		out, err = os.OpenFile(*output, os.O_CREATE|os.O_WRONLY, 0666)
		if err != nil {
			fmt.Println(err)
			os.Exit(exit_failure)
		}
		defer out.Close()
	}

	buf := list.New()
	retval := exit_success

	if flag.NArg() <= 0 {
		hcasl(os.Stdin, out, *chars, buf)
	} else {
		for i := 0; i < flag.NArg(); i++ {
			err := hcaslFromFile(flag.Arg(i), out, *chars, buf)
			if err != nil {
				fmt.Println(err)
				retval = exit_failure
			}
		}
	}

	os.Exit(retval)
}
