#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "string.h"
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

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
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

// read and validate
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo)
{
    if (argc >= 4)
    {
        if (strcmp(strstr(argv[2], "."), ".bmp") == 0)
            encInfo->src_image_fname = argv[2];
        encInfo->secret_fname = argv[3];

        strcpy(encInfo->extn_secret_file, strstr(argv[3], "."));

        encInfo->stego_image_fname = "stego.bmp";
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status check_capacity(EncodeInfo *encInfo)
{
    /*calling function to get image size capacity*/
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    /* Calling function get file size and stroing return value */
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    int total_bytes = 54 + strlen(MAGIC_STRING) * 8 + 32 + 32 + 32 + encInfo->size_secret_file * 8;

    /* Checking whether beautiful.bmp image file size is greater than secret file or not */
    if (total_bytes <= encInfo->image_capacity)
        return e_success;
    else
        return e_failure;
}
Status do_encoding(EncodeInfo *encInfo)
{
    // Function call to check capacity function and checking condition whether success or not
    if (open_files(encInfo) == e_success)
    {
        printf("INFO : Open files is Success\n");
    }
    else
    {
        printf("INFO : Open files is failed\n");
        return e_failure;
    }
    // Function call to check capacity function & checking condition wihether  success or not
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO : Check Capacity is Success\n");
    }
    else
    {
        printf("INFO : Check capacity failed\n");
        return e_failure;
    }
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Copy bmp header is Success\n");
    }
    else
    {
        printf("INFO : Copy bmp header failed\n");
        return e_failure;
    }
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO : Encoded Magic string is Success\n");
        strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));
    }
    else
    {
        printf("INFO : Encoding Magic string failed\n");
        return e_failure;
    }
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Got secret file extension size is Success\n");
    }
    else
    {
        printf("INFO : Encoding secret file extension size is failed\n");
        return e_failure;
    }
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
    {
        printf("INFO : Secret file extension is encoded successfully\n");
    }
    else
    {
        printf("INFO : Secret file extension is encoding is failed\n");
        return e_failure;
    }
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("INFO : Got secret file size successfully\n");
    }
    else
    {
        printf("INFo : Encoding secret file size is failed\n");
        return e_failure;
    }
    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO : Secret file data is encode successfull\n");
    }
    else
    {
        printf("INFO : Secret file data encoding failed\n");
        return e_failure;
    }
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Remaing Image data is copied successfully\n");
    }
    else
    {
        printf("INFO : Copying remaing failed\n");
        return e_failure;
    }
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0L, SEEK_END);
    return ftell(fptr);
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buf[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(buf, 1, 54, fptr_src_image);
    fwrite(buf, 1, 54, fptr_dest_image);
    return e_success;
}
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buf[8];
    for (int i = 0; i < size; i++)
    {
        fread(buf, 1, 8, fptr_src_image);
        encode_byte_to_lsb(data[i], buf);
        fwrite(buf, 1, 8, fptr_stego_image);
    }
    return e_success;
}
Status encode_byte_to_lsb(char data, char *buf)
{
    for (int i = 0; i < 8; i++)
    {
        buf[i] = buf[i] & 0xFE;
        buf[i] = buf[i] | ((data & 1 << i) >> i);
    }
    return e_success;
}
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buf[32];
    fread(buf, 32, 1, fptr_src_image);
    encode_size_to_lsb(size, buf);
    fwrite(buf, 32, 1, fptr_stego_image);
    return e_success;
}
Status encode_size_to_lsb(int size, char *buf)
{
    // char buf[32];
    for (int i = 0; i < 32; i++)
    {
        buf[i] = buf[i] & 0xFE;
        buf[i] = buf[i] | ((size & 1 << i) >> i);
        return e_success;
    }
}

Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buf[32];
    fread(buf, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buf);
    fwrite(buf, 1, 32, encInfo->fptr_stego_image);
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}