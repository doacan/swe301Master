#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_INPUT_FILES 32
#define MAX_TOTAL_SIZE 200  // 200 MBytes

// Struct to hold file information
struct FileInfo {
    char fileName[256];
    mode_t permissions;
    off_t size;
};

// Struct to hold command line arguments
struct CommandLineArguments {
    char** bArguments;
    int bArgumentCount;
    char** aArguments;
    int aArgumentCount;
    char* oArgument;
};

// Function prototypes
long getTotalSize(struct CommandLineArguments* arguments);
void freeCommandLineArguments(struct CommandLineArguments* arguments);
void handleArguments(int argc, char* argv[], struct CommandLineArguments* arguments);



int main(int argc, char* argv[]) {
    struct CommandLineArguments arguments;
    arguments.bArguments = NULL;
    arguments.bArgumentCount = 0;
    arguments.aArguments = NULL;
    arguments.aArgumentCount = 0;
    arguments.oArgument = NULL;

    handleArguments(argc, argv, &arguments);

    if (arguments.bArguments != NULL) {
        long totalSize = getTotalSize(&arguments);
        if (totalSize > (MAX_TOTAL_SIZE * 1024 * 1024)) {
            fprintf(stderr, "Error: Total size of files exceeds the maximum allowed size of %d MB\n", MAX_TOTAL_SIZE);
            exit(1);
        }
        archiveFiles(&arguments);
        printf("The files have been merged.");
    } else if (arguments.aArguments != NULL) {
        openFiles(&arguments);
    } else if (arguments.oArgument != NULL && arguments.bArguments == NULL) {
        perror("Error: -o argument cannot be used by itself");
    } else {
        perror("Error: _name -o output file name OR -a input_file output_folder\");");
    }

    // Free memory
    freeCommandLineArguments(&arguments);
    return 0;
}


long getTotalSize(struct CommandLineArguments* arguments) {
    long totalSize = 0;

    for (int i = 0; i < arguments->bArgumentCount; ++i) {
        struct stat fileStat;
        if (stat(arguments->bArguments[i], &fileStat) == 0) {
            totalSize += fileStat.st_size;
        } else {
            fprintf(stderr, "Error: Unable to get file size for %s\n", arguments->bArguments[i]);
            freeCommandLineArguments(arguments);
            exit(1);
        }
    }

    return totalSize;
}

void freeCommandLineArguments(struct CommandLineArguments* arguments) {
    // Free memory for -a arguments
    for (int i = 0; i < arguments->aArgumentCount; ++i) {
        free(arguments->aArguments[i]);
    }
    free(arguments->aArguments);
    arguments->aArguments = NULL;

    // Free memory for -b arguments
    for (int k = 0; k < arguments->bArgumentCount; ++k) {
        free(arguments->bArguments[k]);
    }
    free(arguments->bArguments);
    arguments->bArguments = NULL;

    // Free memory for -o argument
    free(arguments->oArgument);
    arguments->oArgument = NULL;
}

void handleArguments(int argc, char* argv[], struct CommandLineArguments* arguments) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            if (arguments->aArgumentCount > 0) {
                perror("Error: -b cannot be used with -a\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }

            int endB = i + 1;
            while (endB < argc && strncmp(argv[endB], "-", 1) != 0) ++endB;
            arguments->bArgumentCount = endB - i - 1;

            // Check if the number of -b arguments is less than 1
            if (arguments->bArgumentCount < 1) {
                fprintf(stderr, "Error: At least one argument is required for -b\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }

            arguments->bArguments = (char**)calloc(arguments->bArgumentCount, sizeof(char*));
            for (int j = 0; j < arguments->bArgumentCount; ++j) {
                arguments->bArguments[j] = strdup(argv[i + 1 + j]);
            }
            i = endB - 1;
            if (i > MAX_INPUT_FILES + 1) {
                fprintf(stderr, "Error: More than %d -b files specified\n", MAX_INPUT_FILES);
                freeCommandLineArguments(arguments);
                exit(1);
            }
        } else if (strcmp(argv[i], "-a") == 0) {
            if (arguments->bArgumentCount > 0 || arguments->oArgument != NULL) {
                perror("Error: -a cannot be used with -b or -o\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }
            int endA = i + 1;
            while (endA < argc && strncmp(argv[endA], "-", 1) != 0) ++endA;
            arguments->aArgumentCount = endA - i - 1;
            // Check if the number of -a arguments is exactly two
            if (arguments->aArgumentCount != 2) {
                fprintf(stderr, "Error: Exactly two arguments are required for -a\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }
            arguments->aArguments = (char**)calloc(arguments->aArgumentCount, sizeof(char*));
            for (int j = 0; j < arguments->aArgumentCount; ++j) {
                arguments->aArguments[j] = strdup(argv[i + 1 + j]);
            }
            i = endA - 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (arguments->aArgumentCount > 0) {
                perror("Error: -o cannot be used with -a\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }
            if (i + 1 < argc && strncmp(argv[i + 1], "-", 1) != 0) {
                if (i + 2 < argc && strncmp(argv[i + 2], "-", 1) != 0) {
                    perror("Error: -o requires exactly one argument\n");
                    freeCommandLineArguments(arguments);
                    exit(1);
                }
                arguments->oArgument = strdup(argv[i + 1]);
                i += 1;
            } else {
                perror("Error: -o requires exactly one argument\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }
        } else {
            fprintf(stderr, "Error: Unexpected argument %s\n", argv[i]);
            freeCommandLineArguments(arguments);
            exit(1);
        }
    }
}