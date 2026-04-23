/*
NAME:DIKSHA
PROJECT NAME:STEGNOGRAPHY
PROJECT DESCRIPTION:
This project implements image steganography using the Least Significant Bit (LSB) 
technique to securely hide and retrieve data within a BMP image.
Encoding:
In the encoding process, a normal BMP image is selected as the cover image. The secret file data 
is converted into binary form and hidden inside the image by changing the least significant bits of 
the image pixels. Since only small bits are modified, the image looks the same to the human eye. 
The final image with hidden data is saved as a stego image.
Decoding:
In the decoding process, the stego image is opened to extract the hidden data. 
The least significant bits of the image pixels are read and combined to form the original data. A magic 
string is checked to ensure valid hidden information. The extracted data is then written into a 
new file, recreating the original secret file.
*/


#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include<string.h>
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;// Structure to store encoding information
    DecodeInfo decInfo;// Structure to store decoding information
    if(argc<=2)//Check for insufficient arguments for decoding
    {
        printf("ERROR: Invalid arguments\n");
        printf("Usage-For encoding : ./a.out -e [src.bmp] [secret.txt] <dest.bmp>\n");
        printf("For decoding : ./a.out -d [dest.bmp]  <output>\n");
        return 0;
    }
    else if( (strcmp(argv[1],"-e")==0)&& argc<4  )//Check for arguments when encoding
    {
        printf("ERROR: Invalid arguments\n");
        printf("Usage-For encoding : ./a.out -e [src.bmp] [secret.txt] <dest.bmp>\n");
        printf("For decoding : ./a.out -d [dest.bmp]  <output>\n");
        return 0;
    }
    
    if (check_operation_type(argv[1]) == e_unsupported)//Check whether the operation type is supported or not 
    {
        printf("ERROR: Unsupported operation\n");
        return 0;
    }
    else if(check_operation_type(argv[1]) == e_encode)//check whether the operation is encoding
    {
        if(read_and_validate_encode_args(argv,&encInfo)==e_failure)//to check thr encoding arguments
        {
            printf("ERROR: Invalid extensions\n");
            return 0;
        }
        do_encoding(&encInfo);

    }
    else
    //If the operation is decoding
    {
        if(read_and_validate_decode_args(argv,&decInfo)==e_failure)//to check thr decoding arguments
        {
           printf("ERROR: Invalid extensions\n");
           return 0; 
        }
        do_decoding(&decInfo);
    }
}
OperationType check_operation_type(char *symbol)//Function to check whether it is encoding or decoding
{
    if(strcmp(symbol,"-e")==0)//Comparing thr symbol
    return e_encode;
    else if(strcmp(symbol,"-d")==0)
    return  e_decode;
    else
    return e_unsupported;

}