
# File Archiver and Extractor

## Overview

This program allows users to archive multiple files into a single binary file and extract the files from a pre-existing archive file. It supports two main functionalities:

1. **Archive Files:** Combine multiple input files into a single binary archive file.
2. **Extract Files:** Retrieve individual files from a binary archive file and save them in a specified output folder.

## Usage

### Archiving Files

To archive files, use the following command:

```bash
./tarsau -b t1 t2 t3 t4.txt t5.dat -o s1.sau
```

- `-b`: Indicates that the following arguments are the names of files to be archived.
- ` t1 t2 t3 t4.txt t5.dat `: List of input files to be archived.
- `-o s1.sau`: Specifies the output archive file name (optional, default is "a.sau").

**Note:** The total size of input files should not exceed 200 MB.

### Extracting Files

To extract files from an archive, use the following command:

```bash
./tarsau -a s1.sau d1
```

- `-a`: Indicates that the following arguments are related to extracting files.
- `s1.sau`: The name of the archive file to extract files from.
- `d1`: The folder where extracted files will be stored.

## Build

Compile the program using a C compiler. For example, using `gcc`:

```bash
gcc -o tarsau main.c
```

## Examples

### Archiving Files

```bash
./tarsau -b file1.txt file2.jpg file3.doc -o s1.sau
```

This command archives `file1.txt`, `file2.jpg`, and `file3.doc` into `s1.sau`.

### Extracting Files

```bash
./tarsau -a s1.sau d1
```

This command extracts files from `s1.sau` and saves them in `d1`.

## Error Handling

The program provides informative error messages in case of invalid inputs or file-related errors.

## Limitations

- The total size of input files should not exceed 200 MB.
- The program assumes a maximum of 32 input files for archiving.

## License

This program is distributed under the [MIT License](LICENSE).


