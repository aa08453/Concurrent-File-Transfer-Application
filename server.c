#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <sys/stat.h>
#include "definitions.h"

sem_t socket_sem;
int server_fd, client_socket;
struct sockaddr_in address;
int opt = 1;
socklen_t addrlen = sizeof(address);

/* gets size of the file*/
int get_file_size(const char *path) 
{
    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "File size error");
        exit(EXIT_FAILURE);
    }
    return st.st_size;
}

/* populates data in chunks */
void fill_chunks(const char *path, chunks_t *chunks, int num_chunks, int file_size) 
{
    FILE *file = fopen(path, "rb");
    if (!file) 
    {
        fprintf(stderr, "Unable to open file");
        exit(EXIT_FAILURE);
    }

    // getting size for each chunk
    int chunk_size = ceil((double)file_size / num_chunks);

    // assigning values to chunks
    for (int i = 0; i < num_chunks; i++) 
    {
        // handle case for last chunk which may not be completely used
        int alloc_size = (i == num_chunks - 1) ? (file_size % chunk_size) : chunk_size;
        if (alloc_size == 0) alloc_size = chunk_size; 

        chunks[i].id = i;
        chunks[i].size = alloc_size;
        chunks[i].data = malloc(alloc_size);
        if (!chunks[i].data) 
        {
            fprintf(stderr, "Failed to allocate chunk memory");
            exit(EXIT_FAILURE);
        }
        fread(chunks[i].data, 1, alloc_size, file);
    }
    fclose(file);
}

/* sends the chunk information through the socket to client*/
void* send_chunk(void* arg) 
{
    chunks_t* chunk = (chunks_t*)arg;

    sem_wait(&socket_sem);
    send(client_socket, &chunk->id, sizeof(chunk->id), 0);
    send(client_socket, &chunk->size, sizeof(chunk->size), 0);
    send(client_socket, chunk->data, chunk->size, 0);
    sem_post(&socket_sem);
    return NULL;
}

int main() 
{
    // socket creation
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        fprintf(stderr, "Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    // check for reuse of address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) 
    {
        fprintf(stderr, "setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // attaching socket to specified port and address
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) 
    {
        fprintf(stderr, "Bind failed");
        exit(EXIT_FAILURE);
    }

    // put server in passive mode to wait for connection with client
    if (listen(server_fd, 3) < 0) 
    {
        fprintf(stderr, "Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        // establish connection with client socket
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) 
        {
            fprintf(stderr, "Accept failed");
            continue;
        }

        // recieving command line arguments from client
        args_t args;
        recv(client_socket, &args, sizeof(args), 0);

        // adjusting path for the tests directory, getting file size
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "tests/%s", args.file);
        int file_size = get_file_size(full_path);

        // making chunks of file
        int num_chunks = args.num_threads;
        chunks_t* chunks = malloc(num_chunks * sizeof(chunks_t));
        fill_chunks(full_path, chunks, num_chunks, file_size);

        // threads to send chunks
        pthread_t threads[num_chunks];
        sem_init(&socket_sem, 0, 1);
        for (int i = 0; i < num_chunks; i++) 
            pthread_create(&threads[i], NULL, send_chunk, &chunks[i]);

        for (int i = 0; i < num_chunks; i++) 
        {
            pthread_join(threads[i], NULL);
            free(chunks[i].data);
        }
        
        printf("\nFile sent successfully\n\n");
        free(chunks);
        sem_destroy(&socket_sem);

        // sending checksum of file to client
        unsigned char original_digest[SHA256_DIGEST_LENGTH];
        char original_checksum[65];

        compute_sha256(full_path, original_digest);
        digest_to_hex(original_digest, original_checksum);

        send(client_socket, original_checksum, sizeof(original_checksum), 0);
        printf("Original File SHA256 sent to client: %s\n", original_checksum);

        close(client_socket);
    }

    close(server_fd);
    return 0;
}
