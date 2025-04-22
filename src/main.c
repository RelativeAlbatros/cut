#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct Parameters {
    char delimitor;
    char* file;
    size_t filesize;
    bool logger;
    int target_field;
} Parameters;

void die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void make_sure(bool condition, const char *fmt, ...) {
    if (!condition)
        die(fmt);
}


void print_until_char(const char* buffer, int i, char target) {
    const char* start = buffer + i;
    const char* end = strchr(start, target);
    
    if (end != NULL) {
        size_t length = (size_t)end - (size_t)start;
        printf("%.*s\n", (int)length, start);
    } else {
        // target not found
        printf("%s\n", start);
    }
}

size_t get_filesize(const char* filename) {
    FILE* file = fopen(filename, "r");

    size_t filesize = 0;
    fseek(file, 0, SEEK_END);
    filesize = (size_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    fclose(file);
    return filesize;
}

void log_parameters(Parameters* parameters) {
    char* printed_delimitor = (char[]){parameters->delimitor, '\0'};
    if (parameters->delimitor == '\t') {
        printed_delimitor = "\\t";
    } else if (parameters->delimitor == '\n') {
        printed_delimitor = "\\n";
    }
    printf("\tdelimitor: %s\n"
        "\tfile: %s\n"
        "\tfilesize: %zu\n"
        "\tlogger: %d\n"
        "\ttarget field: %d\n",
        printed_delimitor, parameters->file, parameters->filesize,
        parameters->logger, parameters->target_field
    );
}

Parameters* get_parameters(int argc, char** argv) {
    Parameters* params = malloc(sizeof(Parameters));
    params->delimitor = '\t';
    params->file = "";
    params->filesize = 0;
    params->logger = false;
    params->target_field = 2;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'd') {
                params->delimitor = argv[++i][0];
            } else if (argv[i][1] == 'l') {
                params->logger = true;
            } else if (argv[i][1] == 'f') {
                params->target_field = atoi(argv[++i]);
                make_sure(params->target_field >= 1, "field must be higher than 0");
            }
        } else {
            params->file = argv[i];
            make_sure(access(params->file, F_OK) == 0, "couldn't find file %s", params->file);
            params->filesize = get_filesize(params->file);
        }
    }
    make_sure(strlen(params->file) != 0, "missing argument: file");

    return params;
}

char* cache_file(FILE* file, size_t filesize) {
    char* buffer = malloc(filesize + 1); // +1 for '\0'
    make_sure(buffer != NULL, "error allocating %d", filesize);

    size_t read = fread(buffer, 1, filesize, file);
    make_sure(read == filesize, "buffer read less than filesize.");

    buffer[filesize] = '\0';
    return buffer;
}

void print_by_field(char* buffer, Parameters* params) {
    int current_field, start;

    for (int i = 0; i < (int)params->filesize; i++) {
        current_field = 1;

        while (buffer[i] != '\n') {
            start = i;
            while (current_field != params->target_field) {
                if (buffer[i] == params->delimitor) {
                    current_field++;
                } else if (buffer[i] == '\n') {
                    break;
                }
                i++;
            }
            if (buffer[i] == '\n') {
                print_until_char(buffer, start, '\n');
            } else {
                print_until_char(buffer, i, params->delimitor);
            }
            i++;
            continue;
        }
    }
}

int main(int argc, char** argv) {
    make_sure(argc >= 2, "missing filename");
    Parameters* params = get_parameters(argc, argv);

    FILE *input = fopen(params->file, "r");
    if (!input) {
        die("error reading file: %s", argv[1]);
    }
    char* file_buffer = cache_file(input, params->filesize);
    fclose(input);
    if (params->logger) log_parameters(params);
    print_by_field(file_buffer, params);

    free(file_buffer);
    free(params);
}