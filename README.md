# Stegnography
A steganography application developed in C that securely hides secret data within image files using Least Significant Bit (LSB) technique, ensuring data confidentiality without noticeable changes to the original image.

## Features
- Encode secret message into an image
- Decode hidden message from image
- Uses LSB (Least Significant Bit) technique
- Maintains image quality after encoding
- Supports basic image formats (e.g., BMP)

## Technologies Used
- C Programming
- File Handling
- Bit Manipulation
- Structures and Functions

## How It Works
The project uses the Least Significant Bit (LSB) method to hide secret data inside an image.

- Each pixel in an image is made up of bytes.
- The least significant bit (last bit) of each byte is modified to store bits of the secret message.
- Since only the last bit is changed, the difference in the image is not visible to the human eye.

Encoding:
- The secret message is converted into binary
- These bits are embedded into the LSBs of image pixels
- A new image (stego image) is generated with hidden data

Decoding:
- The program reads the LSBs from the stego image
- Extracts the binary data
- Converts it back to the original message

## How to Run
1.Compile the code by gcc *.c -o stegonography
2.Commant line arguement ./stegonography -e beautiful.bmp secret.txt output.bmp (For encoding)
3../stegonography -d output.bmp extracted.txt (For decoding)

