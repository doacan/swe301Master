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
void archiveFiles(struct CommandLineArguments* arguments);

int main(int argc, char* argv[]) {
    struct CommandLineArguments arguments;
    arguments.bArguments = NULL;
    arguments.bArgumentCount = 0;
    arguments.aArguments = NULL;
    arguments.aArgumentCount = 0;
    arguments.oArgument = NULL;

    // Parse command line arguments
    handleArguments(argc, argv, &arguments);

    if (arguments.bArguments != NULL) {
        // Calculate total size of files to be archived
        long totalSize = getTotalSize(&arguments);

        // Check if total size exceeds the maximum allowed size
        if (totalSize > (MAX_TOTAL_SIZE * 1024 * 1024)) {
            fprintf(stderr, "Error: Total size of files exceeds the maximum allowed size of %d MB\n", MAX_TOTAL_SIZE);
            exit(1);
        }

        // Archive the files
        archiveFiles(&arguments);
        printf("The files have been merged.\n");
    } else if (arguments.aArguments != NULL) {
        // Open and extract files from the archive
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

// Calculate the total size of files to be archived
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

// Free memory allocated for command line arguments
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

// Parse and handle command line arguments
void handleArguments(int argc, char* argv[], struct CommandLineArguments* arguments) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            // Check for conflicting options
            if (arguments->aArgumentCount > 0) {
                perror("Error: -b cannot be used with -a\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }

            // Parse -b arguments
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
            // Check for conflicting options
            if (arguments->bArgumentCount > 0 || arguments->oArgument != NULL) {
                perror("Error: -a cannot be used with -b or -o\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }

            // Parse -a arguments
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
            // Check for conflicting options
            if (arguments->aArgumentCount > 0) {
                perror("Error: -o cannot be used with -a\n");
                freeCommandLineArguments(arguments);
                exit(1);
            }

            // Parse -o argument
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

void archiveFiles(struct CommandLineArguments* arguments) {
    // Determine the output file name
    const char* outputFileName = (arguments->oArgument != NULL) ? arguments->oArgument : "a.sau";
    
    // Open the output file for writing, create if not exists, truncate if exists
    int outFile = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    // Check if the output file was opened successfully
    if (outFile == -1) {
        perror("Error: Unable to create or open output file");
        exit(1);
    }

    // Loop through each input file specified with -b argument
    for (int i = 0; i < arguments->bArgumentCount; ++i) {
        // Open the current input file for reading
        int file = open(arguments->bArguments[i], O_RDONLY);

        // Check if the input file was opened successfully
        if (file != -1) {
            // Initialize variables for character counting and text file determination
            off_t charCount = 0;
            char buffer[1];
            ssize_t bytesRead;
            int isTextFile = 1;  // Assume it's a text file initially
            int nonPrintableCount = 0;  // Counter for non-printable ASCII characters

            // Loop through the contents of the input file
            while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
                charCount += bytesRead;

                // Check if the current character is not a printable ASCII character or numeric digit
                if ((buffer[0] < 32 || buffer[0] > 126) || (buffer[0] >= 48 && buffer[0] <= 57)) {
                    nonPrintableCount++;

                    // Calculate the tolerance as a percentage of non-printable characters
                    double tolerancePercentage = (nonPrintableCount * 100.0) / (double)charCount;

                    // If the tolerance exceeds a certain threshold, consider it not a text file
                    if (tolerancePercentage > 80.0) {
                        isTextFile = 0;  // Not a text file
                        break;
                    }
                }
            }

            // Check if the input file is determined as a text file
            if (!isTextFile) {
                fprintf(stderr, "Error: %s input file format is incompatible!\n", arguments->bArguments[i]);
                close(file);
                close(outFile);
                exit(1);
            }

            // Get file information and write to the output file
            struct stat fileStat;
            if (fstat(file, &fileStat) == 0) {
                dprintf(outFile, "|%s,%o,%ld", arguments->bArguments[i], fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO), charCount);
            }
            else {
                perror("Error: Unable to get file permissions");
                exit(1);
            }

            // Close the input file
            close(file);
        }
        else {
            perror("Error: Unable to open file");
            exit(1);
        }
    }

    // Separate the file metadata from the file content in the output file
    dprintf(outFile, "|");

    // Transfer the content of each input file into 1 byte per character in 8-bit binary ASCII format and add to the output file
    for (int i = 0; i < arguments->bArgumentCount; ++i) {
        // Open the current input file for reading
        int file = open(arguments->bArguments[i], O_RDONLY);

        // Check if the input file was opened successfully
        if (file != -1) {
            char buffer[1];

            // Loop through the contents of the input file
            while (read(file, buffer, sizeof(buffer)) > 0) {
                // Extract each bit of the byte and write it to the output file
                for (int j = 7; j >= 0; --j) {
                    int bit = ((buffer[0] >> j) & 1) + '0';
                    write(outFile, &bit, 1);
                }
            }

            // Close the input file
            close(file);
        }
        else {
            perror("Error: Unable to open file");
        }
    }

    // Close the output file
    close(outFile);
}

    const char* outputFileName = (arguments->oArgument != NULL) ? arguments->oArgument: "a.sau";
    int outFile = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outFile == -1) {
        perror("Error: Unable to create or open output file");
        exit(1);
    }

    for (int i = 0; i < arguments->bArgumentCount ; ++i) {
        int file = open(arguments->bArguments[i], O_RDONLY);
        if (file != -1) {
            off_t charCount = 0;
            char buffer[1];
            ssize_t bytesRead;
            int isTextFile = 1;  // Assume it's a text file initially
            int nonPrintableCount = 0;  // Counter for non-printable ASCII characters
            while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
                charCount += bytesRead;
                // Check if the current character is not a printable ASCII character
                if ((buffer[0] < 32 || buffer[0] > 126) || (buffer[0] >= 48 && buffer[0] <= 57)) {
                    nonPrintableCount++;
                    // Calculate the tolerance as a percentage of non-printable characters
                    double tolerancePercentage = (nonPrintableCount * 100.0) / (double)charCount;
                    // If the tolerance exceeds a certain threshold, consider it not a text file
                    if (tolerancePercentage > 80.0) {
                        isTextFile = 0;  // Not a text file
                        break;
                    }
                }
            }
            if (!isTextFile) {
                fprintf(stderr, "Error: %s input file format is incompatible!\n", arguments->bArguments[i]);
                close(file);
                close(outFile);
                exit(1);
            }
            struct stat fileStat;
            if (fstat(file, &fileStat) == 0) {
                dprintf(outFile, "|%s,%o,%ld", arguments->bArguments[i], fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO), charCount);
            } else {
                perror("Error: Unable to get file permissions");
                exit(1);
            }

            close(file);
        } else {
            perror("Error: Unable to open file");
            exit(1);
        }
    }

    dprintf(outFile, "|");

    // Transfer the content of each file into 1 byte per character in 8-bit binary ASCII format and add to the output file
    for (int i = 0; i < arguments->bArgumentCount ; ++i) {
        int file = open(arguments->bArguments[i], O_RDONLY);
        if (file != -1) {
            char buffer[1];
            while (read(file, buffer, sizeof(buffer)) > 0) {
                for (int j = 7; j >= 0; --j) {
                    // Extract each bit of the byte and write it to the output file
                    int bit = ((buffer[0] >> j) & 1) + '0';
                    write(outFile, &bit, 1);
                }
            }
            close(file);
        } else {
            perror("Error: Unable to open file");
        }
    }
    close(outFile);  // Close the output file
}