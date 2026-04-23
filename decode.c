#include <stdio.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#include <string.h>

// Function to read and validate command-line arguments
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2],".bmp")==NULL)// Check if the argument contains ".bmp"
    return e_failure;
    decInfo->stego_image_fname=argv[2];// Store stego image file name
    if(argv[3]==NULL)// Check output file name is not given
    {
        printf("INFO: Output File not mentioned.Creating default file name as output\n");
        decInfo->secret_fname="output";// Store Default output file name
    }
    else
    {
        if(strchr(argv[3],'.')!=NULL)//To check output file contains extension
        decInfo->secret_fname=strtok(argv[3],"."); // Remove extension
        else
        decInfo->secret_fname=argv[3];// Store output file name
    }
    return e_success;

}
/*// Function to open required files*/
Status open_file(DecodeInfo *decInfo)
{
    // stegno Image file
    printf("INFO: Opening required files\n");
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");// Open stego image file in read mode
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");// Print error
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",decInfo->stego_image_fname);

    return e_success;
}
/*Function to decode magic string*/
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    
    char imageBuffer[8];// Buffer to read 8 bytes from image
    printf("INFO: Decoding Magic String Signature\n");
    for(int i=0;i<2;i++)//Loop to Decode each character of magic string
    {
        char data=0;// Variable to store decoded character
        fread(imageBuffer,sizeof(char),8,decInfo->fptr_stego_image);// Read 8 bytes from image
        decode_byte_to_lsb(&data,imageBuffer);// Decode one byte from LSBs
        if(data!=magic_string[i])// Compare decoded data with magic string
        return e_failure; 
    }
    return e_success;
}
/*Function to decode secret file extension size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char  imageBuffer[32];// Buffer to read 32 bytes
    printf("INFO: Decoding %s File Extension size\n",decInfo->secret_fname);
    fread(imageBuffer,sizeof(char),32,decInfo->fptr_stego_image);// Read 32 bytes from image
    decInfo->size_extn_secret_file=0;// Initialize extension size to zero
    decode_size_to_lsb(imageBuffer,&decInfo->size_extn_secret_file);// Decode size from LSBs
    printf("INFO: Done\n");
    return  e_success;
}
/*Function to decode secret file extension*/
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char imageBuffer[8];// Buffer to read 8 bytes
    printf("INFO: Decoding %s File Extension\n",decInfo->secret_fname);
    for(int i=0;i<decInfo->size_extn_secret_file;i++)// Loop for extension size
    {
        char data=0;// Variable to store decoded character
        fread(imageBuffer,sizeof(char),8,decInfo->fptr_stego_image);// Read 8 bytes from image
        decode_byte_to_lsb(&data,imageBuffer);// Decode one byte
        decInfo->extn_secret_file[i]=data;// Store decoded extension character
    }
    printf("INFO: Done\n");
    return  e_success;
    
}
// Function to decode secret file size
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char  imageBuffer[32];// Buffer to read 32 bytes
    printf("INFO: Decoding %s File size\n",decInfo->secret_fname);
    fread(imageBuffer,sizeof(char),32,decInfo->fptr_stego_image);// Read 32 bytes from image
    decInfo->size_secret_file=0;// Initialize secret file size
    decode_size_to_lsb(imageBuffer,&decInfo->size_secret_file);// Decode file size from LSBs
    printf("INFO: Done\n");
    return  e_success;
    
}
/*Function to decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char  imageBuffer[8];// Buffer to read 8 bytes
    printf("INFO: Decoding %s File Data\n",decInfo->secret_fname);
    for(int i=0;i<decInfo->size_secret_file;i++)// Loop for entire secret file size
    {
        char data=0;// Variable to store decoded byte
        fread(imageBuffer,sizeof(char),8,decInfo->fptr_stego_image);// Read 8 bytes from image
        decode_byte_to_lsb(&data,imageBuffer);// Decode one byte
        fprintf(decInfo->fptr_secret,"%c",data);// Write decoded character into output file
    }
    if(ftell(decInfo->fptr_secret)==decInfo->size_secret_file)// Check if file size written correctly
    {
        printf("INFO: Done\n");
        return  e_success;
    }
    else
    return e_failure;

}

/*Function to decode one byte from LSBs*/
Status decode_byte_to_lsb(char* data, char *image_buffer)
{
    *data=0;
    for(int i=0;i<8;i++)
    {
        *data=((image_buffer[i]& 1)<<i) | (*data);// Extract LSB and store in correct bit position
    }
    return e_success;
}
/*Function to decode size (32 bits) from LSBs*/
Status decode_size_to_lsb(char *imageBuffer,int* data)
{
    *data=0;
    for(int i=0;i<32;i++)
    {
        *data = (*data) | ((imageBuffer[i] & 1)<<i) ;// Extract LSB and store in correct bit position
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(open_file(decInfo)==e_failure)// Open required files
    return e_failure;
    printf("INFO: ## Decoding Procedure Started ##\n");
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);// Skip BMP header (54 bytes)
    if(decode_magic_string(MAGIC_STRING,decInfo)==e_failure)// Decode magic string
    return e_failure;

    if(decode_secret_file_extn_size(decInfo)==e_failure)// Decode extension size
    return  e_failure;

    if(decode_secret_file_extn(decInfo)==e_failure) // Decode extension
    return e_failure;

    sprintf(decInfo->temp,"%s%s",decInfo->secret_fname,decInfo->extn_secret_file);// Combine file name and extension
    decInfo->secret_fname=decInfo->temp;// Update secret file name
    decInfo->fptr_secret=fopen(decInfo->secret_fname,"w"); // Open output file in write mode
    if(decInfo->fptr_secret==NULL)// Check if output file opening failed
    printf("ERROR:unable to open %s file\n",decInfo->secret_fname);

    if(decode_secret_file_size(decInfo)==e_failure)// Decode secret file size
    return e_failure;

    if(decode_secret_file_data(decInfo) == e_failure)// Decode secret file data
    return e_failure;

// Close files
    fclose(decInfo->fptr_secret);
    fclose(decInfo->fptr_stego_image);
    printf("INFO: ##  Decoding Done Successfully ##\n");

    return e_success;
}