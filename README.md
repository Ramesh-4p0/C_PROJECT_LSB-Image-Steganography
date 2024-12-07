# C_PROJECT_LSB-Image-Steganography
Description:
Developed a command-line application in C to implement Least Significant Bit (LSB) steganography, enabling users to hide and extract secret messages within BMP image files.

Key Features:

Message Encoding: Analyzes BMP file capacity to ensure the message fits within the image data.
Magic String Identifier: Embeds a unique identifier to verify if an image contains hidden data.
Message Decoding: Extracts hidden messages from steganographic images.
Command-Line Interface: Accepts input and output file paths, and operation type (encode/decode) via arguments.
Tech Stack:

Language: C
Concepts: File I/O, pointers, structures, encryption/decryption
Outcome:

Successfully implemented and tested the application, demonstrating efficient data hiding and retrieval without noticeable changes to the image.
