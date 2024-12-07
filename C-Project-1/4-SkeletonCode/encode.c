#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);
    // the width us there at the 18th location so we are pointing there
    //  Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    // width address to store the width,sizeof the datatype this image pixels are of int,how many data is 1 and the image
    printf("width = %u\n", width);
    // we know file pointer moves continously so it goes after next 4 bytes that is 22 location
    //  Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3; // colours for the pixels
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {

        return e_failure;
    }
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}
uint get_file_size(FILE *secret_file)
{
    fseek(secret_file, 0, SEEK_END); // Move the file pointer to the last
    return ftell(secret_file);       // return the position of the file pointer
}
Status check_capacity(EncodeInfo *encInfo)
{
    // size of .bmp file
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    // get the size of secret.txt
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    // printf("%ld",encInfo->size_secret_file);
    if (encInfo->image_capacity > (54 + ((2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status copy_bmp_header(FILE *fptr_src, FILE *fptr_dest)
{
    char header[54];
    // bring the file pointer to first position in bmp beacause its pointing to 18th position the width
    // rewind(fptr_src);
    fseek(fptr_src, 0, SEEK_SET);
    fread(header, sizeof(char), 54, fptr_src);
    fwrite(header, sizeof(char), 54, fptr_dest);
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned char mask = 1 << 7;
    for (int i = 0; i < 8; i++)
    {
        // get a bit from the data and or it with lsb of the rgb
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
}
Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo)
{
    // call encode_byte to_lsb to encode  the character
    // fetch 8 bytes of rgb from beautiful .bmp for size no of times
    for (int i = 0; i < size; i++)
    {
        // read 8 bytes of data
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        encode_byte_to_lsb(data[i], encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
}
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    // every character encoding needs to call the encode_data_to_image
    encode_data_to_image(magic_string, strlen(magic_string), encInfo);
    return e_success;
}
Status encode_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    char str[32];
    fread(str, 32, sizeof(char), fptr_src);
    // every integer encoding should call size to ls function
    encode_size_to_lsb(str, size);
    fwrite(str, 32, sizeof(char), fptr_stego);
    return e_success;
}
Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for (int i = 0; i < 31; i++)
    {
        // get a bit from the data and or it with lsb of the rgb
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask = mask >> 1;
    }
}
Status encode_secret_file_extn(char *file_ext, EncodeInfo *encInfo)
{
    file_ext = ".txt";
    encode_data_to_image(file_ext, strlen(file_ext), encInfo);
    return e_success;
}
Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, 32, sizeof(char), encInfo->fptr_src_image);
    // every integer encoding should call size to ls function
    encode_size_to_lsb(str, size);
    fwrite(str, 32, sizeof(char), encInfo->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    // bring the file pointer to the beginning of secret file
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        // read 8 bytes of rgb from .bmp
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        // read a character from  secret file
        fread(&ch, 1, sizeof(char), encInfo->fptr_secret);
        encode_byte_to_lsb(ch, encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src,FILE *fptr_stego){
    char ch;
    while(fread(&ch,1,1,fptr_src)>0)//till end of the file one one byte yu are writing
    {
        fwrite(&ch,1,1,fptr_stego);
    }
    return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
    // remaining functions are called here
    if (open_files(encInfo) == e_success)
    {
        printf("Opened all the required files\n");
        if (check_capacity(encInfo) == e_success)
        {
            printf("Possible to encode the data\n");
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Copied the header successfully\n");
                // from 54 th byte
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Magic string encoded succesfully\n");
                    if (encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Successfully encode the secret file extension size\n");
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == 0)
                        {
                            printf("Succesfully encoded secret file extensions\n");
                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Encoded the secret file size successfully\n");
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Encoded secret data successfully\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success){
                                        printf("Copied remaining data successfully\n");
                                    }
                                    else{
                                        printf("Failed to copy the remaining data\n ");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode the secret data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode the secret size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to encode the secret file extension\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to encode the sectret file extension size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to encode the magic string\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy the header\n");
                return e_failure;
            }
        }
        else
        {
            printf("Image capacity is not sufficient to encode the data\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open the files\n");
        return e_failure;
    }
    return e_success;
}