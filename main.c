#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_INPUT_FILES 32
#define MAX_TOTAL_SIZE 200  // 200 MBytes

// Function prototypes
long getTotalSize(char** bArguments, int bArgumentCount);
void handleArguments(int argc, char* argv[], char*** bArguments, int* bArgumentCount, char*** aArguments, int* aArgumentCount, char** oArgument);


int main(int argc, char* argv[]) {
    char** bArguments = NULL;
    int bArgumentCount = 0;

    char** aArguments = NULL;
    int aArgumentCount = 0;

    char* oArgument = NULL;

    handleArguments(argc, argv, &bArguments, &bArgumentCount, &aArguments, &aArgumentCount, &oArgument);

    if (bArguments != NULL) {
        const char* outputFileName = (oArgument != NULL) ? oArgument : "a.sau";
        long totalSize = getTotalSize(bArguments, bArgumentCount);
        if (totalSize > (MAX_TOTAL_SIZE * 1024 * 1024)) {
            fprintf(stderr, "Error: Total size of files exceeds the maximum allowed size of %d MB\n", MAX_TOTAL_SIZE);
            exit(1);
        }
		
		
        printf("The files have been merged.");
    }else if (aArguments != NULL){
		
		
    }

    for (int i = 0; i < aArgumentCount; ++i) {
        free(aArguments[i]);
    }
    free(aArguments);
    aArguments = NULL;

    for (int k = 0; k < bArgumentCount; ++k) {
        free(bArguments[k]);
    }
    free(bArguments);
    bArguments = NULL;

    free(oArgument);
    oArgument = NULL;

    return 0;
}

long getTotalSize(char** bArguments, int bArgumentCount) {
    long totalSize = 0;

    for (int i = 0; i < bArgumentCount; ++i) {
        struct stat fileStat;
        if (stat(bArguments[i], &fileStat) == 0) {
            totalSize += fileStat.st_size;
        } else {
            fprintf(stderr, "Error: Unable to get file size for %s\n", bArguments[i]);
        }
    }

    return totalSize;
}

void handleArguments(int argc, char* argv[], char*** bArguments, int* bArgumentCount, char*** aArguments, int* aArgumentCount, char** oArgument) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            if (*aArgumentCount > 0) {
                fprintf(stderr, "Error: -b cannot be used with -a\n");
                exit(1);
            }
            int endB = i + 1;
            while (endB < argc && strncmp(argv[endB], "-", 1) != 0) ++endB;
            *bArgumentCount = endB - i - 1;
            *bArguments = (char**)calloc(*bArgumentCount, sizeof(char*));
            for (int j = 0; j < *bArgumentCount; ++j) {
                (*bArguments)[j] = strdup(argv[i + 1 + j]);
            }
            i = endB - 1;
            if (i > MAX_INPUT_FILES + 1) {
                fprintf(stderr, "Error: More than %d -b files specified\n", MAX_INPUT_FILES);
                exit(1);
            }
        } else if (strcmp(argv[i], "-a") == 0) {
            if (*bArgumentCount > 0 || *oArgument != NULL) {
                fprintf(stderr, "Error: -a cannot be used with -b or -o\n");
                exit(1);
            }
            int endA = i + 1;
            while (endA < argc && strncmp(argv[endA], "-", 1) != 0) ++endA;
            *aArgumentCount = endA - i - 1;
            *aArguments = (char**)calloc(*aArgumentCount, sizeof(char*));
            for (int j = 0; j < *aArgumentCount; ++j) {
                (*aArguments)[j] = strdup(argv[i + 1 + j]);
            }
            i = endA - 1;
            if (*aArgumentCount > 3) {
                fprintf(stderr, "Error: More than 2 -a arguments specified\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (*aArgumentCount > 0) {
                fprintf(stderr, "Error: -o cannot be used with -a\n");
                exit(1);
            }
            if (i + 1 < argc && strncmp(argv[i + 1], "-", 1) != 0) {
                if (i + 2 < argc && strncmp(argv[i + 2], "-", 1) != 0) {
                    fprintf(stderr, "Error: -o requires exactly one argument\n");
                    exit(1);
                }
                *oArgument = strdup(argv[i + 1]);
                i += 1;
            } else {
                fprintf(stderr, "Error: -o requires exactly one argument\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Error: Unexpected argument %s\n", argv[i]);
            exit(1);
        }
    }
}