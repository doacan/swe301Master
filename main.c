#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void handle_B_Arguments(int start, int end, char* argv[], char*** bArguments, int* bArgumentCount) {
    *bArgumentCount = end - start;
    *bArguments = (char**)malloc((*bArgumentCount) * sizeof(char*));
    for (int i = 0; i < *bArgumentCount; ++i) {
        (*bArguments)[i] = strdup(argv[start + i]);
    }
}

void handle_A_Arguments(int start, int end, char* argv[], char*** aArguments, int* aArgumentCount) {
    *aArgumentCount = end - start;
    *aArguments = (char**)malloc((*aArgumentCount) * sizeof(char*));
    for (int i = 0; i < *aArgumentCount; ++i) {
        (*aArguments)[i] = strdup(argv[start + i]);
    }
}

void freeArgumentArrays(char** a_arguments, int a_argumentCount, char** b_arguments, int b_argumentCount, char* c_argument) {
    for (int i = 0; i < a_argumentCount; ++i) {
        free(a_arguments[i]);
    }
    free(a_arguments);
    for (int k = 0; k < b_argumentCount; ++k) {
        free(b_arguments[k]);
    }
    free(b_arguments);
    free(c_argument);
}


int main(int argc, char* argv[]) {
    char** bArguments = NULL;
    int bArgumentCount = 0;

    char** aArguments = NULL;
    int aArgumentCount = 0;

    char* oArgument = NULL;

    int bFlag = 0;
    int oFlag = 0;
    int aFlag = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            if (aFlag) {
                fprintf(stderr, "Error: -b cannot be used with -a\n");
                freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );
                return 1;
            }
            int endB = i + 1;
            while (endB < argc && strncmp(argv[endB], "-", 1) != 0) ++endB;
            handle_B_Arguments(i + 1, endB, argv, &bArguments, &bArgumentCount);
            i = endB - 1;
            bFlag = 1;
        } else if (strcmp(argv[i], "-a") == 0) {
            if (bFlag || oFlag) {
                fprintf(stderr, "Error: -a cannot be used with -b or -o\n");
                freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );
                return 1;
            }

            int endO = i + 1;
            while (endO < argc && strncmp(argv[endO], "-", 1) != 0) ++endO;
            handle_A_Arguments(i + 1, endO, argv, &aArguments, &aArgumentCount);
            i = endO - 1;
            aFlag = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (aFlag) {
                fprintf(stderr, "Error: -o cannot be used with -a\n");
                freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );
                return 1;
            }
            if (i + 1 < argc && strncmp(argv[i + 1], "-", 1) != 0) {
                if (i + 2 < argc && strncmp(argv[i + 2], "-", 1) != 0) {
                    fprintf(stderr, "Error: -o requires exactly one argument\n");
                    freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );
                    return 1;
                }
                oArgument = strdup(argv[i + 1]);
                i += 1;
            } else {
                fprintf(stderr, "Error: -o requires exactly one argument\n");
                freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );
                return 1;
            }
            oFlag = 1;
        } else {
            fprintf(stderr, "Error: Unexpected argument %s\n", argv[i]);
            freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );
            return 1;
        }
    }

    if (bArguments != NULL) {
		printf("Hello");
    }else if (aArguments != NULL){
		printf("World");
    }

    freeArgumentArrays(aArguments, aArgumentCount,bArguments, bArgumentCount, oArgument );

    return 0;
}
