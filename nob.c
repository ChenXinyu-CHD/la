#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "thirdparty/nob.h"
#define FLAG_IMPLEMENTATION
#include "thirdparty/flag.h"

#define BUILD_FOLDER      "build/"
#define SRC_FOLDER        "src/"
#define EXAMPLES_FOLDER   "examples/"
#define THIRDPARTY_FOLDER "thirdparty/"

static Cmd cmd = {0};
static Procs procs = {0};

static void compile(void)
{
    cmd_append(&cmd, "clang");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-Wswitch-enum");
    cmd_append(&cmd, "-Wno-unused-function");
    cmd_append(&cmd, "-fsanitize=memory,undefined");
    cmd_append(&cmd, "-I.");
    cmd_append(&cmd, "-I"THIRDPARTY_FOLDER);
}

void print_usage(void)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", flag_program_name());
    fprintf(stderr, "OPTIONS:\n");
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool trace = false;
    bool help = false;

    flag_bool_var(&trace, "trace", false, "Enabling tracing of the location where each single line was generated");
    flag_bool_var(&help, "help", false, "Print this help message");

    if (!flag_parse(argc, argv)) {
        print_usage();
        flag_print_options(stderr);
        return 1;
    }

    if (help) {
        print_usage();
        return 0;
    }

    if (!mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    compile();
    if (trace) cmd_append(&cmd, "-DGEN_TRACE");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", BUILD_FOLDER"lag");
    cmd_append(&cmd, SRC_FOLDER"lag.c");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, BUILD_FOLDER"lag");
    if (!cmd_run(&cmd, .stdout_path = "./la.h")) return 1;

    {
        compile();
        cmd_append(&cmd, "-pedantic");
        cmd_append(&cmd, "-std=c11");
        cmd_append(&cmd, "-ggdb");
        cmd_append(&cmd, "-o", BUILD_FOLDER"ball");
        cmd_append(&cmd, EXAMPLES_FOLDER"ball.c");
        cmd_append(&cmd, "-lm");
        if (!cmd_run(&cmd, .async = &procs)) return 1;

        compile();
        cmd_append(&cmd, "-DLA_IMPLEMENTATION");
        cmd_append(&cmd, "-x", "c++");
        cmd_append(&cmd, "-o", BUILD_FOLDER"la.cxx.o");
        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "la.h");
        if (!cmd_run(&cmd, .async = &procs)) return 1;

        compile();
        cmd_append(&cmd, "-pedantic");
        cmd_append(&cmd, "-std=c11");
        cmd_append(&cmd, "-DLA_IMPLEMENTATION");
        cmd_append(&cmd, "-x", "c");
        cmd_append(&cmd, "-o", BUILD_FOLDER"la.c.o");
        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "la.h");
        if (!cmd_run(&cmd, .async = &procs)) return 1;

        if (!procs_flush(&procs)) return 1;
    }

    return 0;
}
