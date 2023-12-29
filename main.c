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
void openFiles(struct CommandLineArguments* arguments);

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


void openFiles(struct CommandLineArguments* arguments) {
    // Check if the file extension is ".sau"
    char* fileExtension = strrchr(arguments->aArguments[0], '.');
    if (fileExtension == NULL || strcmp(fileExtension, ".sau") != 0) {
        perror("Archive file is inappropriate or corrupt!\n");
        freeCommandLineArguments(arguments);
        exit(1);
    }

    // Open the file for reading
    int file = open(arguments->aArguments[0], O_RDONLY);
    if (file == -1) {
        perror("Error: Unable to open file");
        freeCommandLineArguments(arguments);
        exit(1);
    }

    // Get the file size
    struct stat fileStat;
    if (fstat(file, &fileStat) == -1) {
        perror("Error: Unable to get file size");
        freeCommandLineArguments(arguments);
        close(file);
        exit(1);
    }

    size_t bufferSize = fileStat.st_size;
    char* buffer = (char*)malloc(bufferSize);
    if (buffer == NULL) {
        perror("Error: Unable to allocate buffer");
        freeCommandLineArguments(arguments);
        close(file);
        exit(1);
    }

    ssize_t bytesRead = read(file, buffer, bufferSize);
    close(file);

    if (bytesRead == -1) {
        perror("Error: Unable to read file");
        freeCommandLineArguments(arguments);
        free(buffer);
        exit(1);
    }

    struct stat folderStat;
    // Check if the destination folder exists, and create it if not
    if (stat(arguments->aArguments[1], &folderStat) == -1 && mkdir(arguments->aArguments[1], 0777) == -1) {
        perror("Error: Unable to create folder");
        freeCommandLineArguments(arguments);
        free(buffer);
        exit(1);
    }

    char* lastPipe = strrchr(buffer, '|');
    if (lastPipe == NULL) {
        perror("Archive file is inappropriate or corrupt!");
        freeCommandLineArguments(arguments);
        free(buffer);
        exit(1);
    }

    // Calculate the index by subtracting the pointer difference
    size_t lastIndex = lastPipe - buffer;
    char* pipePosition = strchr(buffer, '|');
    long index = pipePosition - buffer;
    size_t offset = lastIndex + 1;

    // Iterate through the archive file to extract file information and write files
    while ((size_t)index < lastIndex - 1) {
        struct FileInfo fileInfo;
        // Parse file information from the buffer
        sscanf(buffer + index, "|%[^,],%o,%ld|", fileInfo.fileName, &fileInfo.permissions, &fileInfo.size);
        pipePosition = strchr(pipePosition + 1, '|');
        index = pipePosition - buffer;
        char filePath[512];
        // Create the full path for the file in the destination folder
        snprintf(filePath, sizeof(filePath), "%s/%s", arguments->aArguments[1], fileInfo.fileName);
        // Open the file for writing
        int outFile = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, fileInfo.permissions);
        if (outFile == -1) {
            // Print file information and error messages
            printf("%s,%u,%ld", fileInfo.fileName, fileInfo.permissions, fileInfo.size);
            perror("Error: Unable to create file");
            perror("Archive file is inappropriate or corrupt!");
            // Cleanup and exit
            freeCommandLineArguments(arguments);
            exit(1);
        }
        size_t dataSize = fileInfo.size * 8;
        char substring[dataSize];
        // Copy the binary data from the buffer
        strncpy(substring, buffer + offset, fileInfo.size * 8);
        offset += fileInfo.size * 8;
        // Convert binary ASCII to characters and write to the file
        for (size_t i = 0; i < dataSize; i += 8) {
            char binaryChar[9];
            strncpy(binaryChar, substring + i, 8);
            binaryChar[8] = '\0';
            // Convert binary string to integer
            long asciiValue = strtol(binaryChar, NULL, 2);
            // Write the ASCII value to the file
            write(outFile, &asciiValue, 1);
        }
        close(outFile);
        // Print file name and separator based on whether there are more files
        printf("%s%s", fileInfo.fileName, ((size_t)index < lastIndex - 1) ? ", " : " ");
    }
    // Print a message indicating the number of files opened
    printf("files opened in the %s directory.\n", arguments->aArguments[1]);
    // Cleanup allocated memory
    free(buffer);
}