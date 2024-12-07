#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <stdlib.h>

// Function definition for read and validate decode args
Status_d read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{

    if (argv[2] == NULL)

    {
        decInfo->d_src_image_fname = "stego.bmp"; // default source image file
    }
    else
    {
        decInfo->d_src_image_fname = argv[2];
    }
    if (argv[3] != NULL)
    {
        decInfo->d_secret_fname = argv[3];
    }
    else
    {
        decInfo->d_secret_fname = "decode.txt"; // default secret file
    }
    if (open_files_dec(decInfo) == e_success) // Calls the open_files_dec function to open the specified files for reading and writing.
    {
        return d_success;
    }
    unsigned char a1[2];
    if ((decInfo->fptr_d_src_image) == NULL)
    {
        printf("Error : Memory is not available\n");
        return d_failure;
    }

    fread(a1, 1, 2, (decInfo->fptr_d_src_image)); // reads the first two bytes of the source image file into the array

    if (a1[0] == 0x42 && a1[1] == 0x4d) // checks if the first two bytes match the BMP file signature
    {
        return d_success;
    }
    else
    {
        printf("Not a BMP file\n");
        return d_failure;
    }
    return d_success; // returns success once the process completes without any issues
}

// Function definition for open files for decoding
Status_d open_files_dec(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_d_src_image = fopen(decInfo->d_src_image_fname, "r"); // open the source file in read mode

    // Do Error handling
    if (decInfo->fptr_d_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->d_src_image_fname);
        return d_failure;
    }
    // Dest file
    decInfo->fptr_d_secret = fopen(decInfo->d_secret_fname, "w"); // open the secret file in write mode

    // Do Error handling
    if (decInfo->fptr_d_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->d_secret_fname);
        return d_failure;
    }

    // If no failure then return e_success
    return d_success;
}

// Function definition for decode magic string
Status_d decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_d_src_image, 54, SEEK_SET); // skip BMP header to access pixel data
    int i = strlen(MAGIC_STRING);                   // check the magic string length

    decInfo->magic_data = malloc(strlen(MAGIC_STRING) + 1); // memory is allocated for magic_data to decode magic string
    decode_data_from_image(strlen(MAGIC_STRING), decInfo->fptr_d_src_image, decInfo);

    decInfo->magic_data[i] = '\0';

    if (strcmp(decInfo->magic_data, MAGIC_STRING) == 0) // compares decoded string with expected MAGIC_STRING
        return d_success;
    else
        return d_failure;
}

// Function definition for decoding data fom image
Status_d decode_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    int i;
    char str[8];
    for (i = 0; i < size; i++)
    {
        fread(str, 8, sizeof(char), fptr_d_src_image); // read 8 bytes from image file
        decode_byte_from_lsb(&decInfo->magic_data[i], str);
    }
    return d_success;
}

// Function definition for decode byte from lsb
Status_d decode_byte_from_lsb(char *data, char *image_buffer)
{
    int bit = 7;             // set bit to 7
    unsigned char ch = 0x00; // initialize ch to 0x00 to indicate an empty byte
    for (int i = 0; i < 8; i++)
    {
        ch = ((image_buffer[i] & 0x01) << bit--) | ch;
    }
    *data = ch;
    return d_success;
}

// Function definition for decode file extn size
Status_d decode_file_extn_size(int size, FILE *fptr_d_src_image)
{
    char str[32];
    int length;

    fread(str, 32, sizeof(char), fptr_d_src_image); // read 32 bytes from source file to str array
    decode_size_from_lsb(str, &length);
    printf("%d", length);
    if (length - 1 == size) // compare the length and size
        return d_success;
    else
        return d_failure;
}

// Function definition decode size from lsb
Status_d decode_size_from_lsb(char *buffer, int *size)
{
    int j = 31; // MSB of 32 bit integer
    int num = 0x00;
    for (int i = 0; i < 32; i++)
    {
        num = ((buffer[i] & 0x01) << j--) | num;
    }
    *size = num;
}

// Function definition for decode secret file extn
Status_d decode_secret_file_extn(char *file_ext, DecodeInfo *decInfo)
{
    file_ext = ".txt";                           // set to .txt
    int i = strlen(file_ext);                    // calculates the length of .txt
    decInfo->d_extn_secret_file = malloc(i + 1); // memory allocation to hold the file extn
    decode_extension_data_from_image(strlen(file_ext), decInfo->fptr_d_src_image, decInfo);

    decInfo->d_extn_secret_file[i] = '\0';
    printf("%s", decInfo->d_extn_secret_file);
    if (strcmp(decInfo->d_extn_secret_file, file_ext) == 0) // compares the file extn with the expected value
        return d_success;
    else
        return d_failure;
}

// Function definition decode extension data from image
Status_d decode_extension_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    for (int i = 0; i < size; i++)
    {
        fread(decInfo->d_src_image_fname, 8, 1, fptr_d_src_image);                         // reads 8 bytes at a time
        decode_byte_from_lsb(&decInfo->d_extn_secret_file[i], decInfo->d_src_image_fname); // decodes LSB of 8bytes into single character
    }
    return d_success;
}

// Function definition for decode secret file size
Status_d decode_secret_file_size(int file_size, DecodeInfo *decInfo)
{
    char str[32];
    fread(str, 32, sizeof(char), decInfo->fptr_d_src_image);
    decode_size_from_lsb(str, &file_size);
    decInfo->size_secret_file = file_size;
    // printf("%d\n", file_size);

    return d_success;
}

// Function definition for decode secret file data
Status_d decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(decInfo->d_src_image_fname, 8, sizeof(char), decInfo->fptr_d_src_image);
        decode_byte_from_lsb(&ch, decInfo->d_src_image_fname);
        fputc(ch, decInfo->fptr_d_secret);
    }
    return d_success;
}

// Function definition for do decoding
Status_d do_decoding(DecodeInfo *decInfo)
{
    if (open_files_dec(decInfo) == d_success)
    {
        printf("Opening files Successfully\n");
        if (decode_magic_string(decInfo) == d_success)
        {
            printf("Decoded magic string Successfully\n");
            if (decode_file_extn_size(strlen(".txt"), decInfo->fptr_d_src_image) == d_success)
            {
                printf("Decoded file extension size Succesfully\n");
                if (decode_secret_file_extn(decInfo->d_extn_secret_file, decInfo) == d_success)
                {
                    printf("Decoded Secret File Extension Succesfully\n");
                    if (decode_secret_file_size(decInfo->size_secret_file, decInfo) == d_success)
                    {
                        printf("Decoded secret file size Successfully\n");
                        if (decode_secret_file_data(decInfo) == d_success)
                        {
                            printf("Decoded secret file data Succuessfully\n");
                        }
                        else
                        {
                            printf("Decoding of secret file data is a failure\n");
                        }
                    }
                    else
                    {
                        printf("Decode of secret file size is a failure\n");
                        return d_failure;
                    }
                }
                else
                {
                    printf("Decode of Secret file extension is a failure\n");
                    return d_failure;
                }
            }
            else
            {
                printf("Decoded of file extension size is a failure\n");
                return d_failure;
            }
        }
        else
        {
            printf("Decoding of magic string is a failure\n");
            return d_failure;
        }
    }
    else
    {
        printf("Open files is a failure\n");
        return d_failure;
    }
    return d_success;
}
