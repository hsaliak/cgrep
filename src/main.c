#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include "raii.h"
#include "discovery.h"
#include "worker.h"
#include "matcher.h"


static void print_usage(const char *progname) {
    fprintf(stderr, "Usage: %s [OPTIONS] PATTERN [PATH...]\n", progname);
    fprintf(stderr, "If no PATH is provided, or if PATH is '-', read from standard input.\n");
    fprintf(stderr, "If -r is specified and no PATH is provided, search recursively from '.'.\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i, --ignore-case      Ignore case distinctions\n");
    fprintf(stderr, "  -F, --fixed-strings    Interpret PATTERN as a fixed string, not a regular expression\n");
    fprintf(stderr, "  -n, --line-number      Print line number with output lines\n");
    fprintf(stderr, "  -v, --invert-match     Invert match: select non-matching lines\n");
    fprintf(stderr, "  -r, --recursive        Read all files under each directory, recursively\n");
    fprintf(stderr, "  -w, --workers=NUM      Number of worker threads (default: auto)\n");
    fprintf(stderr, "  -I                     Process a binary file as if it did not contain matching data (default)\n");
    fprintf(stderr, "  --include=GLOB         Search only files whose base name matches GLOB\n");
    fprintf(stderr, "  --exclude=GLOB         Skip files whose base name matches GLOB\n");
}

int main(int argc, char *argv[]) {
    grep_config_t grep_cfg = { .code = NULL, .case_insensitive = false, .line_numbering = false, .fixed_strings = false, .invert_match = false };
    auto_str_array char **include_patterns = NULL;
    auto_str_array char **exclude_patterns = NULL;
    discovery_config_t disc_cfg = { 
        .include_patterns = NULL, .include_count = 0, 
        .exclude_patterns = NULL, .exclude_count = 0, 
        .ignore_binary = true, .recursive = false 
    };

    static struct option long_options[] = {
        {"ignore-case", no_argument, 0, 'i'},
        {"fixed-strings", no_argument, 0, 'F'},
        {"line-number", no_argument, 0, 'n'},
        {"invert-match", no_argument, 0, 'v'},
        {"recursive",   no_argument, 0, 'r'},
        {"workers",     required_argument, 0, 'w'},
        {"include",     required_argument, 0, 1},
        {"exclude",     required_argument, 0, 2},
        {"help",        no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int num_workers = 3;
    int opt;
    while ((opt = getopt_long(argc, argv, "iFnv r w:Ih", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i': grep_cfg.case_insensitive = true; break;
            case 'F': grep_cfg.fixed_strings = true; break;
            case 'n': grep_cfg.line_numbering = true; break;
            case 'v': grep_cfg.invert_match = true; break;
            case 'r': disc_cfg.recursive = true; break;
            case 'w':
                num_workers = atoi(optarg);
                if (num_workers <= 0) {
                    fprintf(stderr, "Error: Number of workers must be at least 1.\n");
                    return 1;
                }
                break;
            case 'I': disc_cfg.ignore_binary = true; break;
            case 1: // --include
                include_patterns = realloc(include_patterns, sizeof(char*) * (disc_cfg.include_count + 2));
                include_patterns[disc_cfg.include_count++] = strdup(optarg);
                include_patterns[disc_cfg.include_count] = NULL;
                break;
            case 2: // --exclude
                exclude_patterns = realloc(exclude_patterns, sizeof(char*) * (disc_cfg.exclude_count + 2));
                exclude_patterns[disc_cfg.exclude_count++] = strdup(optarg);
                exclude_patterns[disc_cfg.exclude_count] = NULL;
                break;
            case 'h': print_usage(argv[0]); return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: Pattern is required.\n");
        print_usage(argv[0]);
        return 1;
    }

    const char *pattern = argv[optind++];
    auto_pcre2_code pcre2_code *code = matcher_compile(pattern, grep_cfg.case_insensitive, grep_cfg.fixed_strings);
    if (code == NULL) return 1;
    grep_cfg.code = code;
    disc_cfg.include_patterns = include_patterns;
    disc_cfg.exclude_patterns = exclude_patterns;

    auto_work_queue work_queue_t queue = {0};
    work_queue_init(&queue);
    queue.pending_items = 1; // Prevent workers from exiting while we are still discovering

    if (optind >= argc) {
        if (disc_cfg.recursive) {
            discover_files(".", &disc_cfg, &queue);
        } else {
            work_queue_push(&queue, "-");
        }
    } else {
        for (; optind < argc; optind++) {
            if (strcmp(argv[optind], "-") == 0) {
                work_queue_push(&queue, "-");
                continue;
            }
            struct stat path_stat;
            if (lstat(argv[optind], &path_stat) == 0) {
                if (S_ISDIR(path_stat.st_mode)) {
                    if (disc_cfg.recursive || strcmp(argv[optind], ".") == 0) {
                        discover_files(argv[optind], &disc_cfg, &queue);
                    }
                } else {
                    work_queue_push(&queue, argv[optind]);
                }
            }
        }
    }

    pthread_t workers[num_workers];
    worker_args_t wargs = { .queue = &queue, .grep_config = &grep_cfg, .discovery_config = &disc_cfg };

    for (int i = 0; i < num_workers; i++) {
        pthread_create(&workers[i], NULL, worker_thread, &wargs);
    }

    // Now that initial discovery is done and workers are started, decrement the sentinel
    work_queue_item_done(&queue);

    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    return 0;
}
