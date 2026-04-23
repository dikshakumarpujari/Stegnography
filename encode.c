#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include<string.h>
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

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
/*Validates command-line arguments for encoding*/
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(strstr(argv[2],".bmp")==NULL)// Validates source image extension
    return e_failure;
    encInfo->src_image_fname=argv[2];
    if(strstr(argv[3],".txt")==NULL && strstr(argv[3],".c")==NULL && strstr(argv[3],".h")==NULL && strstr(argv[3],".sh")==NULL)// Validate secret file extensions
    return e_failure;
    encInfo->secret_fname=argv[3];
    if(argv[4]==NULL)// checks if destination image not provided
    {
        printf("INFO: Output File not mentioned.Creating dest.bmp as default\n");
        encInfo->stego_image_fname="dest.bmp";
    }
    else
    {
        if(strstr(argv[4],".bmp")==NULL)// Validate destination image extension
        return e_failure;
        encInfo->stego_image_fname=argv[4];
    }
    return e_success;

}
/*Function to Open source image, secret file, and stego image*/
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    printf("INFO: Opening required files\n");
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->src_image_fname);
    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->secret_fname);
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->stego_image_fname);
    printf("INFO: Done\n");
    // No failure return e_success
    
    return e_success;
}
/*Function to Check whether image has enough capacity*/
Status check_capacity(EncodeInfo *encInfo)
{
     
     printf("INFO: checking for %s size\n",encInfo->secret_fname);
     encInfo->size_secret_file =get_file_size(encInfo->fptr_secret);// Get size of the secret file
     printf("INFO: Done.Not Empty\n");
     printf("INFO: checking for %s Capacity to handle secret.txt\n",encInfo->src_image_fname);
     encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);// Get total capacity of the BMP image
     if(encInfo->image_capacity > (16+32+32+32+((encInfo->size_secret_file)*8)))// Required bits calculation
     {
        printf("INFO: Done.Found OK\n");
        return e_success;
     }
     else
     return e_failure;
}
/*Function to copy the BMP header*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char imageBuffer[54];// Buffer to store BMP header (54 bytes)
    rewind(fptr_src_image);// Move file pointer to start of source image
    fread(imageBuffer,sizeof(char),54,fptr_src_image);// Read BMP header from source
    printf("INFO: Copying Image Header\n");
    fwrite(imageBuffer,sizeof(char),54,fptr_dest_image);// Write header to destination image
    if(ftell(fptr_src_image)==ftell(fptr_dest_image))// Check file pointer pointing to the same line
    {
        printf("INFO: Done\n");
        return e_success;
    }
    else
    return e_failure;
}
/*Function to encode the magic string*/
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];// Buffer to store 8 image bytes
    printf("INFO: Encoding Magic String Signature\n");
    for(int i=0;i<2;i++)
    {
        fread(imageBuffer,sizeof(char),8,encInfo->fptr_src_image);// Read 8 bytes
        encode_byte_to_lsb(magic_string[i],imageBuffer);// Encode one character byte to lsb
        fwrite(imageBuffer,sizeof(char),8,encInfo->fptr_stego_image);// Write encoded data
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))// Check file pointer pointing to the same line
    {
        printf("INFO: Done\n");
        return e_success;
    }
    else
    return e_failure;
}
/*Function to check the secret file extension*/
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char  imageBuffer[32];// Buffer for encoding size (32 bits)
    printf("INFO: Encoding %s File Extension size\n",encInfo->secret_fname);
    fread(imageBuffer,sizeof(char),32,encInfo->fptr_src_image);// Read 32 image bytes
    encode_size_to_lsb(size,imageBuffer);
    fwrite(imageBuffer,sizeof(char),32,encInfo->fptr_stego_image);// Write encoded data
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))// Check file pointer pointing to the same line
    {
        printf("INFO: Done\n");
        return  e_success;
    }
    else
    return e_failure;


}
/*Function to encode the file extension*/
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char imageBuffer[8];// Buffer for encoding one byte
    int len=strlen(file_extn);// Length of file extension
    printf("INFO: Encoding %s File Extension\n",encInfo->secret_fname);
    for(int i=0;i<len;i++)
    {
        fread(imageBuffer,sizeof(char),8,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],imageBuffer);// Encode extension character
        fwrite(imageBuffer,sizeof(char),8,encInfo->fptr_stego_image);
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))// Check file pointer pointing to the same line
    {
        printf("INFO: Done\n");
        return  e_success;
    }
    else
    return e_failure;
}
/*Function to encode the secret file size*/
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char  imageBuffer[32];// Buffer for encoding file size
    printf("INFO: Encoding %s File size\n",encInfo->secret_fname);
    fread(imageBuffer,sizeof(char),32,encInfo->fptr_src_image);// Read 32 bytes
    encode_size_to_lsb(file_size,imageBuffer);// Encode file size
    fwrite(imageBuffer,sizeof(char),32,encInfo->fptr_stego_image);// Write encoded size
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))// Check file pointer pointing to the same line
    {
        printf("INFO: Done\n");
    return  e_success;
    }
    else
    return e_failure;
}
/*Function to encode the secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);// Move secret file pointer to start
    fread(encInfo->secret_data,sizeof(char),encInfo->size_secret_file,encInfo->fptr_secret);// Read secret data
    char  imageBuffer[8];// Buffer for encoding one byte
    printf("INFO: Encoding %s File Data\n",encInfo->secret_fname);
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(imageBuffer,sizeof(char),8,encInfo->fptr_src_image);// Read image bytes
        encode_byte_to_lsb(encInfo->secret_data[i],imageBuffer);// Encode data byte
        fwrite(imageBuffer,sizeof(char),8,encInfo->fptr_stego_image);
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))//Check the pointer points to same line
    {
        printf("INFO: Done\n");
    return  e_success;
    }
    else
    return e_failure;

}
/*Function to copy remaining image data*/
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;// Variable to store one byte
    printf("INFO: Copying Left Over Data\n");
    while (fread(&ch, sizeof(char), 1, fptr_src) == 1)// Read bytes
    {
        fwrite(&ch, sizeof(char), 1, fptr_dest);//Write bytes
    }
    printf("INFO: Done\n");
    return e_success;

}
/*Function to encode byte to lsb*/
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
        image_buffer[i]= (image_buffer[i]& 0xFE) |  ((data >> i)& 1);// Replace LSB with data bit
    }
    return e_success;
}
/*Function to encode size to lsb*/
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for(int i=0;i<32;i++)
    {
        imageBuffer[i]= (imageBuffer[i]& 0xFE) |  ((size >> i)& 1);// Encode size bit
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo)==e_failure)// Open required files
    return e_failure;
    printf("INFO: ## Encoding Procedure Started ##\n");
    if(check_capacity(encInfo)==e_failure)// Check image capacity
    return e_failure;
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure) // Copy header
    {
        printf("ERROR: Bmp header coping failed\n");
        return e_failure;
    }
    if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure)// Encode magic string
    {
        printf("ERROR: Magic string encoding failed\n");
        return e_failure;
    }
    char *a=strchr(encInfo->secret_fname,'.');//  To Extract file extension
    strcpy(encInfo->extn_secret_file,a);// Store extension
    int len=strlen(encInfo->extn_secret_file);//To Get extension length
    if(encode_secret_file_extn_size(len,encInfo)==e_failure)// Encode extension size
    {
        printf("ERROR: Encoding secret file extension size failed\n");
        return  e_failure;
    }
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure)// Encode extension
    {
        printf("ERROR: Encoding secret file extension failed\n");
        return e_failure;
    }

    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure)// Encode file size
    {
        printf("ERROR: Encoding secret file size failed\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo) == e_failure)// Encode secret data
    {
        printf("ERROR: Encoding secret file data failed\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)// Copy remaining image data
    {
        printf("ERROR: Encoding remaining image data failed\n");
        return e_failure;
    }
    fclose(encInfo->fptr_src_image);// Close source image
    fclose(encInfo->fptr_secret);// Close secret file
    fclose(encInfo->fptr_stego_image);// Close stego image

    printf("INFO: ##  Encoding Done Successfully ##\n");

    return e_success;
}