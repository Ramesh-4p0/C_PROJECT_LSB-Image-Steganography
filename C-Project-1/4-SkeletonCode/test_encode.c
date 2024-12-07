#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"
int main(int argc, char *argv[])
{
    //check the operation  -e or -d
    if(check_operation_type(argv)==e_encode){
        EncodeInfo encInfo;
        printf("Selected Encoding\n");
        if(read_and_validate_encode_args(argv,&encInfo)==e_success){
            printf("Read and validate argument is successful\n");
            if(do_encoding(&encInfo)== e_success){
                printf("Completed the encoding\n");
            }
            else{
                printf("Failed to encode the data\n");
            }
        }
        else{
            printf("Failed to validate the encode arguments\n");
        }

    }
    else if(check_operation_type(argv)==e_decode){
        DecodeInfo decInfo;
        printf("Selected Decoding\n");
         if(read_and_validate_decode_args(argv,&decInfo)==d_success){
            printf("Read and validate argument is successful\n");
            if(do_decoding(&decInfo)== d_success){
                printf("Completed the decoding\n");
            }
              else{
                printf("Failed to decode the data\n");
            }
         }
    }
    else{
        printf("Invalid Option\nUsage\n");
        printf("Encoding: .\a.out -e beautiful.bmp secret.txt\n");
        printf("Decoding: ./a.out -d stego.bmp\n");
    }
    EncodeInfo encInfo;
    uint img_size;

    // Fill with sample filenames
    encInfo.src_image_fname = "beautiful.bmp";
    encInfo.secret_fname = "secret.txt";
    encInfo.stego_image_fname = "stego_img.bmp";

    // Test open_files
    if (open_files(&encInfo) == e_failure)
    {
    	printf("ERROR: %s function failed\n", "open_files" );
    	return 1;
    }
    else
    {
    	printf("SUCCESS: %s function completed\n", "open_files" );
    }

    // Test get_image_size_for_bmp
    img_size = get_image_size_for_bmp(encInfo.fptr_src_image);
    printf("INFO: Image size = %u\n", img_size);

    return 0;
}

OperationType check_operation_type(char *argv[]){
    if(strcmp(argv[1],"-e")==0){
        return e_encode;
    }
    else if (strcmp(argv[1],"-d")==0){
        return e_decode;
    }
    else{
        return e_unsupported;
    }
}