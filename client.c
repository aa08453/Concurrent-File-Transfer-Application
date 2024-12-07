#include <arpa/inet.h>
#include "definitions.h"

int client_fd;

/* evaluates integrity of transferred file by checksum recalcuation and comparision*/
void integrity_check(int socket_fd, char* file)
{
    unsigned char reassembled_digest[SHA256_DIGEST_LENGTH];
    char reassembled_checksum[65];

    // compute checksum at client end
    compute_sha256(file, reassembled_digest);
    digest_to_hex(reassembled_digest, reassembled_checksum);

    char original_checksum[65];

    // get the original checksum from the server
    recv(socket_fd, original_checksum, sizeof(original_checksum), 0);
    printf("Original File SHA256 received from server: %s\n", original_checksum);

    printf("\nReassembled File SHA256: %s\n", reassembled_checksum);

    // comparision of checksums
    if (strcmp(original_checksum, reassembled_checksum) == 0) 
        printf("\nData Integrity Check: PASSED\n");
    else 
        printf("\nData Integrity Check: FAILED\n");
    printf("\n========================================================================\n\n");
}

/* reassembles received chunks from user in a new file*/
void receive_file(int socket_fd, const char* path, int num_chunks) 
{
    FILE* file = fopen(path, "wb");
    if (!file) 
    {
        fprintf(stderr, "Error opening file for writing");
        return;
    }

    chunks_t* chunks = malloc(num_chunks * sizeof(chunks_t));
    if (!chunks)
    {
        fprintf(stderr, "Memory allocation for chunks failed");
        fclose(file);
        return;
    }

    int chunk_count = 0;
    while (chunk_count < num_chunks) 
    {
        int id, size;

        // receive metadata: chunk id and size
        if (recv(socket_fd, &id, sizeof(id), 0) <= 0 || recv(socket_fd, &size, sizeof(size), 0) <= 0) 
        {
            fprintf(stderr, "Failed to receive chunk metadata");
            break;
        }
        chunks[id].id = id;
        chunks[id].size = size;

        // allocate memory for chunk data
        chunks[id].data = malloc(size);
        if (!chunks[id].data) 
        {
            fprintf(stderr, "Memory allocation for chunk data failed");
            break;
        }

        // receive chunk data
        int bytes_received = 0;
        // loop to account for partial transfers of bytes
        while (bytes_received < size) 
        {
            int n = recv(socket_fd, chunks[id].data + bytes_received, size - bytes_received, 0);
            if (n <= 0) 
            {
                fprintf(stderr, "Failed to receive chunk data");
                break;
            }
            bytes_received += n;
        }

        chunk_count++;
    }

    // write received chunks to the output file in order
    for (int i = 0; i < num_chunks; i++) 
    {
        if (chunks[i].data) 
        {
            fwrite(chunks[i].data, 1, chunks[i].size, file);
            free(chunks[i].data);
        }
    }

    free(chunks);
    fclose(file);
    printf("File received and written to: %s\n\n", path);
}

int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "Invalid number of arguments passed\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serv_addr;

    // socket creation
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        fprintf(stderr, "Client socket creation failed");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // convert IPv4 address to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {
        fprintf(stderr, "Invalid address");
        return EXIT_FAILURE;
    }

    // establish connection with server socket
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        fprintf(stderr, "Connection failed");
        return EXIT_FAILURE;
    }

    // storing command line arguments in args struct
    args_t args;
    strncpy(args.file, argv[1], sizeof(args.file) - 1); // ensure null-termination
    args.file[sizeof(args.file) - 1] = '\0';
    args.num_threads = atoi(argv[2]);

    // send request to server
    send(client_fd, &args, sizeof(args), 0);

    // create reassembled file path
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "assembled_files/new_%s", strrchr(args.file, '/') ? strrchr(args.file, '/') + 1 : args.file);

    // reassemble and validate data for server in new created file
    receive_file(client_fd, output_path, args.num_threads);
    integrity_check(client_fd, output_path);

    close(client_fd);
    return 0;
}
