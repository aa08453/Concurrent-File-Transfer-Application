// common preprocessor directives
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define PORT 8080


typedef struct // for containing command line arguments of user
{
    char file[256];
    int num_threads;
} args_t;

//
typedef struct // for file chunks
{
    unsigned char* data;
    int id;
    int size;
} chunks_t;

// compute checksum function
void compute_sha256(const char *file_path, unsigned char *out_digest) {
    FILE *file = fopen(file_path, "rb");
    if (!file) 
    {
        perror("Unable to open file");
        return;
    }

    // Create and initialize the EVP context
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) 
    {
        perror("EVP_MD_CTX_new failed");
        fclose(file);
        return;
    }

    // Initialize the digest operation for SHA-256
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) 
    {
        perror("EVP_DigestInit_ex failed");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return;
    }

    unsigned char buffer[4096];
    size_t bytes_read;

    // Read the file and update the digest with its contents
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) 
    {
        if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) 
        {
            perror("EVP_DigestUpdate failed");
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            return;
        }
    }

    // Finalize the digest and retrieve the result
    unsigned int digest_length;
    if (EVP_DigestFinal_ex(mdctx, out_digest, &digest_length) != 1) 
    {
        perror("EVP_DigestFinal_ex failed");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return;
    }

    // Cleanup
    EVP_MD_CTX_free(mdctx);
    fclose(file);
}

// convert checksum to hexadecimal form
void digest_to_hex(unsigned char *digest, char *output) 
{
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + (i * 2), "%02x", digest[i]);
    output[64] = 0;
}