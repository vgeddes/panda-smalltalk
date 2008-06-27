#include <stdio.h>
#include <stdlib.h>
#include "optparse.h"

int verbose = 1, group = 0;
struct opt_str fn = {NULL, 0};
const char *mode[] = {
    "intermediate", "novice", "expert", NULL
};

struct opt_spec options[] = {
    {opt_help, "h", "--help", OPT_NO_METAVAR, OPT_NO_HELP, OPT_NO_DATA},
    {opt_store_1, "v", "--verbose", OPT_NO_METAVAR,
     "make lots of noise [default]", &verbose},
    {opt_store_0, "q", "--quiet", OPT_NO_METAVAR,
     "be vewwy quiet (I'm hunting wabbits)", &verbose},
    {opt_store_str, "f", "--file", "FILE", "write output to FILE", &fn},
    {opt_store_choice, "m", "--mode", "MODE", "interaction mode: one of "
     "'novice', 'intermediate' [default], 'expert'", mode},
    {opt_text, OPT_NO_SF, OPT_NO_LF, OPT_NO_METAVAR,
     "  Dangerous Options:", OPT_NO_DATA},
    {opt_text, OPT_NO_SF, OPT_NO_LF, OPT_NO_METAVAR,
     "    Caution: use of these options is at your own risk.  "
     "It is believed that some of them bite.", OPT_NO_DATA},
    {opt_store_1, "g", OPT_NO_LF, OPT_NO_METAVAR, "  Group option.", &group},
    {OPT_NO_ACTION}
};

int main(int argc, char **argv)
{
   int i;

    opt_basename(argv[0], '/');
    if (opt_parse("usage: %s [options] arg1 arg2", options, argv) != 2) {
        fprintf(stderr, "%s: 2 arguments required\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("noise level: %s\n", verbose ? "verbose" : "quiet");
    if (fn.s) {
        fn.s[0] = fn.s0;
        printf("output file: %s\n", fn.s);
    }
    else
        puts("output file: none");
    printf("interaction mode: %s\n", mode[0]);
    printf("group option: %d\n", group);
    puts("positional arguments:");
    for (i = 1; i < argc; ++i) {
        if (*argv[i] && argv[i] != fn.s)
            printf("  %s\n", argv[i]);
    }
    return 0;
}
