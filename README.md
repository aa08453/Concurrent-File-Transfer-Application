# Concurrent File Transfer Application - README

## Overview
This program implements client-server communication using sockets. The client requests a file from the server, which sends the file in chunks using multiple threads. On the client side, the file chunks are reassembled into the original file. To ensure the integrity of the transferred data, SHA-256 checksums are computed and compared on both the server and client sides.

## Features
- Socket-based communication between client and server.
- File transfer in chunks using multithreading on the server.
- File reassembly on the client side.
- Data integrity verification using SHA-256 hashing.
- Supports multiple test files for verification.

## Directory Structure
- **`tests/`**: Contains test files used for testing the file transfer.
- **`assembled_files/`**: Stores reassembled files on the client side after transfer.
- **`server.c/`**: Implementation of the code for server program.
- **`client.c/`**: Implementation of the code for the client program.
- **`Makefile`**: Automates the build process.
- **`tests.sh`**: Shell script to execute predefined test cases.

## Prerequisites
Ensure OpenSSL is installed on your system for SHA-256 functionality. If it is not installed, you can use the following commands to install it:

### For Ubuntu/Debian-based systems:
```bash
sudo apt update
sudo apt install libssl-dev
```

## Setup
1. Clone the repository or download the program files.
2. Ensure the `tests/` directory contains the files to be transferred.
3. Ensure the `assembled_files/` directory exists for storing the reassembled files.

## Build and Running the Program
1. To compile the program, use the following command:
    ```bash
    make all
    ```
2. Execute the test script to run predefined test cases:
   ```bash
   bash tests.sh
   ```
3. After running, the reassembled files will appear in the `assembled_files/` directory.
4. The terminal will display messages indicating the success of the file transfer and the integrity verification checks.

## Testing
The `tests.sh` script automatically performs the following steps:
1. Initiates the server program.
2. Executes the client to request files from the server.
3. Verifies the integrity of reassembled files against the original files in the `tests/` directory using SHA-256.

## Notes
- Ensure all test files are placed in the `tests/` directory before running `tests.sh`. Some test files have been provided beforehand for ease.
- Verify that `openssl` is correctly installed to avoid checksum computation errors.
- This program assumes single-client usage for the current implementation.

## References
1. [Socket Programming in C](https://www.geeksforgeeks.org/socket-programming-cc/)
2. [Getting File Size in C](https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c)
3. [Usage of `strncpy`](https://stackoverflow.com/questions/52207214/how-to-use-strncpy-correctly), [GeeksforGeeks on `strncpy`](https://www.geeksforgeeks.org/strncpy-function-in-c/)
4. [SHA-256 with OpenSSL](https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c)
5. [Sample Files for Testing](https://sample-videos.com/download-sample-jpg-image.php)
6. ChatGPT has been used to generate and use SHA-256 functions in the `definitions.h` file.
