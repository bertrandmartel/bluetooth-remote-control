/************************************************************************************
 * The MIT License (MIT)                                                            *
 *                                                                                  *
 * Copyright (c) 2016 Bertrand Martel                                               *
 *                                                                                  *
 * Permission is hereby granted, free of charge, to any person obtaining a copy     *
 * of this software and associated documentation files (the "Software"), to deal    *
 * in the Software without restriction, including without limitation the rights     *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell        *
 * copies of the Software, and to permit persons to whom the Software is            *
 * furnished to do so, subject to the following conditions:                         *
 *                                                                                  *
 * The above copyright notice and this permission notice shall be included in       *
 * all copies or substantial portions of the Software.                              *
 *                                                                                  *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR       *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,         *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE      *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER           *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,    *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN        *
 * THE SOFTWARE.                                                                    *
 ************************************************************************************/
#ifndef UNPACK__
#define UNPACK__

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "fastlz.h"
#include "adafruit1_8_oled_library.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include "bsp.h"
#include "unpack.h"

/* magic identifier for 6pack file */
unsigned char sixpack_magic[8] = {137, '6', 'P', 'K', 13, 10, 26, 10};

pstorage_handle_t  pstorage_handle;
uint8_t *image_part = 0;
uint8_t *image_part2 = 0;
bool image_part_select = false;
uint8_t block_offset = 0;
uint32_t image_index = 0;
uint16_t frame_offset = 0;
uint8_t block_max = 0;
uint8_t last_value = 0;

uint32_t store_data_pstorage() {

    uint32_t retval;

    pstorage_handle_t block_handle;

    SEGGER_RTT_printf(0, "\x1B[32mstoring data in block number %d\x1B[0m\n", block_offset);

    retval = pstorage_block_identifier_get(&pstorage_handle, block_offset, &block_handle);

    if (retval == NRF_SUCCESS) {

        SEGGER_RTT_printf(0, "\x1B[32mwriting %d blocks\x1B[0m\n", frame_offset);

        uint8_t align_offset = frame_offset % 4;

        if (!image_part_select) {
            pstorage_store(&block_handle, image_part, frame_offset + align_offset, 0);
        }
        else {
            pstorage_store(&block_handle, image_part2, frame_offset + align_offset, 0);
        }
        block_offset++;
    }
    else {
        SEGGER_RTT_printf(0, "\x1B[32mpstorage_block_identifier_get FAILURE\x1B[0m\n");
    }

    image_part_select = !image_part_select;

    return retval;
}

/* for Adler-32 checksum algorithm, see RFC 1950 Section 8.2 */
#define ADLER32_BASE 65521

static inline unsigned long update_adler32(unsigned long checksum, const void *buf, int len)
{
    const unsigned char* ptr = (const unsigned char*)buf;
    unsigned long s1 = checksum & 0xffff;
    unsigned long s2 = (checksum >> 16) & 0xffff;

    while (len > 0)
    {
        unsigned k = len < 5552 ? len : 5552;
        len -= k;

        while (k >= 8)
        {
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            s1 += *ptr++; s2 += s1;
            k -= 8;
        }

        while (k-- > 0)
        {
            s1 += *ptr++; s2 += s1;
        }
        s1 = s1 % ADLER32_BASE;
        s2 = s2 % ADLER32_BASE;
    }
    return (s2 << 16) + s1;
}

/* return non-zero if magic sequence is detected */
/* warning: reset the read pointer to the beginning of the file */
int detect_magic()
{
    uint8_t buffer[8];
    //size_t bytes_read;
    uint8_t c;
    uint32_t retval;

    pstorage_handle_t block_handle;

    retval = pstorage_block_identifier_get(&pstorage_handle, 0, &block_handle);

    if (retval == NRF_SUCCESS) {
        retval = pstorage_load(buffer, &block_handle, 8, 0);
    }
    else {
        SEGGER_RTT_printf(0, "\x1B[32mpstorage_block_identifier_get FAILURE\x1B[0m\n");
    }

    for (c = 0; c < 8; c++) {
        if (buffer[c] != sixpack_magic[c])
            return 0;
    }
    return -1;
}

static inline unsigned long readU16( const unsigned char* ptr )
{
    return ptr[0] + (ptr[1] << 8);
}

static inline unsigned long readU32( const unsigned char* ptr )
{
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 16) + (ptr[3] << 24);
}

void read_memory_chunk(uint8_t * output_buffer, int length, uint8_t *block_offset, uint16_t *bit_offset) {

    SEGGER_RTT_printf(0, "\x1B[32mread_memory_chunk with length : %d\x1B[0m\n", length);
    SEGGER_RTT_printf(0, "\x1B[32m.................\x1B[0m\n");

    uint8_t align_offset = *bit_offset % 4;

    int remain = length + *bit_offset;

    SEGGER_RTT_printf(0, "\x1B[32mbit_offset : %d | align_offset: %d | remain : %d\x1B[0m\n", *bit_offset, align_offset, remain);

    int end_index = 0;

    if (remain >= 1024) {
        end_index = remain - 1024;
    }

    pstorage_handle_t block_handle;

    uint32_t retval = pstorage_block_identifier_get(&pstorage_handle, *block_offset, &block_handle);

    SEGGER_RTT_printf(0, "\x1B[32mindex value : %d\x1B[0m\n", end_index);

    if (retval == NRF_SUCCESS) {

        if (end_index == 0) {

            uint8_t *tmp_buffer = (uint8_t*)malloc(sizeof(uint8_t) * (length + align_offset));
            memset(tmp_buffer, 0, length + align_offset);

            SEGGER_RTT_printf(0, "\x1B[32mrequesting a length of : %d\x1B[0m\n", (length + align_offset));

            retval = pstorage_load(tmp_buffer, &block_handle, length + align_offset, *bit_offset - align_offset);

            if (retval != NRF_SUCCESS) {
                SEGGER_RTT_printf(0, "\x1B[32mpstorage_load FAILURE1\x1B[0m\n");
            }

            for (int i  = align_offset; i < (length + align_offset); i++) {
                output_buffer[i - align_offset] = tmp_buffer[i];
            }

            (*bit_offset) += length;

            free(tmp_buffer);
            tmp_buffer = 0;
        }
        else {

            int limit = 1024 - *bit_offset;

            uint8_t *tmp_buffer = (uint8_t*)malloc(sizeof(uint8_t) * (limit + align_offset));
            memset(tmp_buffer, 0, limit + align_offset);

            SEGGER_RTT_printf(0, "\x1B[32mrequesting a length of : %d\x1B[0m\n", (limit + align_offset));

            retval = pstorage_load(tmp_buffer, &block_handle, limit + align_offset, *bit_offset - align_offset);

            if (retval != NRF_SUCCESS) {
                SEGGER_RTT_printf(0, "\x1B[32mpstorage_load FAILURE2\x1B[0m\n");
            }

            int index = 0;

            for (index  = align_offset; index < (limit + align_offset); index++) {
                output_buffer[index - align_offset] = tmp_buffer[index];
            }

            (*bit_offset) += limit;

            free(tmp_buffer);
            tmp_buffer = 0;

            (*block_offset)++;

            retval = pstorage_block_identifier_get(&pstorage_handle, *block_offset, &block_handle);

            if (retval == NRF_SUCCESS) {

                pstorage_size_t data_length = length - limit;

                uint32_t remain_data =  0;
                uint32_t iteration_num = 0;

                if (data_length > 1024) {

                    iteration_num = data_length / 1024;
                    remain_data = data_length % 1024;

                    for (uint8_t i = 0; i  < iteration_num; i++) {

                        SEGGER_RTT_printf(0, "\x1B[32mrequesting a length of : 1024\x1B[0m\n");

                        tmp_buffer = (uint8_t*)malloc(sizeof(uint8_t) * 1024);
                        memset(tmp_buffer, 0, 1024);

                        retval = pstorage_load(tmp_buffer, &block_handle, 1024, 0);

                        if (retval != NRF_SUCCESS) {
                            SEGGER_RTT_printf(0, "\x1B[32mpstorage_load FAILURE3\x1B[0m\n");
                        }

                        for (int i  = 0; i < 1024; i++) {
                            output_buffer[limit + i] = tmp_buffer[i];
                        }

                        free(tmp_buffer);
                        tmp_buffer = 0;

                        limit += 1024;

                        (*block_offset)++;

                        retval = pstorage_block_identifier_get(&pstorage_handle, *block_offset, &block_handle);
                    }

                    if (remain_data != 0) {

                        uint8_t align_offset3 = remain_data % 4;

                        SEGGER_RTT_printf(0, "\x1B[32mrequesting a length of : %d\x1B[0m\n", (remain_data + align_offset3));

                        tmp_buffer = (uint8_t*)malloc(sizeof(uint8_t) * (remain_data + align_offset3));
                        memset(tmp_buffer, 0, remain_data + align_offset3);

                        retval = pstorage_load(tmp_buffer, &block_handle, remain_data + align_offset3, 0);

                        if (retval != NRF_SUCCESS) {
                            SEGGER_RTT_printf(0, "\x1B[32mpstorage_load FAILURE4\x1B[0m\n");
                        }

                        SEGGER_RTT_printf(0, "\x1B[32mblock_max : %d, current block : %d\x1B[0m\n", block_max, *block_offset);

                        if (*block_offset == block_max) {
                            //TODO : fix this bug => last value stored in pstorage is incorrect (0xFF)
                            tmp_buffer[remain_data - 1] = last_value;
                        }

                        for (int i  = 0; i < remain_data; i++) {
                            output_buffer[limit + i] = tmp_buffer[i];
                        }

                        free(tmp_buffer);
                        tmp_buffer = 0;

                    }
                    (*bit_offset) = remain_data;
                }
                else {

                    align_offset = data_length % 4;

                    tmp_buffer = (uint8_t*)malloc(sizeof(uint8_t) * (data_length + align_offset));
                    memset(tmp_buffer, 0, data_length + align_offset);

                    retval = pstorage_load(tmp_buffer, &block_handle, data_length + align_offset, 0);

                    if (retval != NRF_SUCCESS) {
                        SEGGER_RTT_printf(0, "\x1B[32mpstorage_load FAILURE3\x1B[0m\n");
                    }

                    for (int i  = 0; i < data_length; i++) {
                        output_buffer[limit + i] = tmp_buffer[i];
                    }

                    (*bit_offset) = data_length;

                    free(tmp_buffer);
                    tmp_buffer = 0;
                }
            }
            else {
                SEGGER_RTT_printf(0, "\x1B[32mpstorage_block_identifier_get FAILURE\x1B[0m\n");
            }
        }
    }
    else {
        SEGGER_RTT_printf(0, "\x1B[32mpstorage_block_identifier_get FAILURE\x1B[0m\n");
    }
}


void read_chunk_header(uint8_t *block_offset_var, uint16_t *bit_offset, uint16_t* id, uint16_t* options, uint32_t* size,
                       uint32_t* checksum, uint32_t* extra)
{
    SEGGER_RTT_printf(0, "\x1B[32mbit_offset : %d & block_offset : %d\x1B[0m\n", *bit_offset, *block_offset_var);

    uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t) * 16);
    memset(buffer, 0, 16);

    read_memory_chunk(buffer, 16, block_offset_var, bit_offset);

    *id = readU16(buffer) & 0xffff;
    *options = readU16(buffer + 2) & 0xffff;
    *size = readU32(buffer + 4) & 0xffffffff;
    *checksum = readU32(buffer + 8) & 0xffffffff;
    *extra = readU32(buffer + 12) & 0xffffffff;

    free(buffer);
    buffer = 0;
}

int unpack_file()
{
    uint32_t count = 0;
    uint8_t block_offset_unpack = 0;
    uint16_t bit_offset = 0;
    uint16_t chunk_id = 0;
    uint16_t chunk_options = 0;
    uint32_t chunk_size = 0;
    uint32_t chunk_checksum = 0;
    uint32_t chunk_extra = 0;
    uint32_t checksum = 0;
    uint16_t c = 0;
    uint32_t decompressed_size = 0;
    uint32_t name_length = 0;
    uint8_t* output_file = 0;
    uint8_t* compressed_buffer = 0;
    uint8_t* decompressed_buffer = 0;

    /* not a 6pack archive? */
    if (!detect_magic())
    {
        SEGGER_RTT_printf(0, "\x1B[32mError: not a 6pack archive!\x1B[0m\n");
        return -1;
    }

    bit_offset += 8;

    SEGGER_RTT_printf(0, "\x1B[32mArchive processing : %d octets\x1B[0m\n", image_index);

    /* initialize */
    output_file = 0;
    decompressed_size = 0;
    compressed_buffer = 0;
    decompressed_buffer = 0;

    set_bitmap_stream();

    /* main loop */
    for (;;)
    {
        SEGGER_RTT_printf(0, "\x1B[32mcurrent count : %d & image_index : %d\x1B[0m\n", count, image_index);
        /* end of file? */
        if (count > image_index) {
            break;
        }

        //read from block block_offset_unpack
        read_chunk_header(&block_offset_unpack, &bit_offset, &chunk_id, &chunk_options,
                          &chunk_size, &chunk_checksum, &chunk_extra);

        SEGGER_RTT_printf(0, "\x1B[32mchunk_id       : %d\x1B[0m\n", chunk_id);
        SEGGER_RTT_printf(0, "\x1B[32mchunk_options  : %d\x1B[0m\n", chunk_options);
        SEGGER_RTT_printf(0, "\x1B[32mchunk_size     : %d\x1B[0m\n", chunk_size);
        SEGGER_RTT_printf(0, "\x1B[32mchunk_checksum : %d\x1B[0m\n", chunk_checksum);
        SEGGER_RTT_printf(0, "\x1B[32mchunk_extra    : %d\x1B[0m\n", chunk_extra);

        if ((chunk_id == 1) && (chunk_size > 10) && (chunk_size < BLOCK_SIZE))
        {
            free(output_file);
            output_file = 0;

            uint8_t * buffer = (uint8_t*)malloc(sizeof(uint8_t) * chunk_size);
            read_memory_chunk(buffer, chunk_size, &block_offset_unpack, &bit_offset);

            checksum = update_adler32(1L, buffer, chunk_size);
            if (checksum != chunk_checksum)
            {
                free(output_file);
                SEGGER_RTT_printf(0, "\nError: checksum mismatch!\n");
                SEGGER_RTT_printf(0, "Got %08lX Expecting %08lX\n", checksum, chunk_checksum);
                return -1;
            }
            decompressed_size = readU32(buffer);

            name_length = (uint16_t)readU16(buffer + 8);

            if (name_length > (uint16_t)chunk_size - 10) {
                name_length = chunk_size - 10;
            }

            output_file = (uint8_t*)malloc(sizeof(uint8_t) * (name_length + 1));
            memset(output_file, 0, name_length + 1);

            for (c = 0; c < name_length; c++) {
                output_file[c] = buffer[10 + c];
            }
            free(buffer);

            SEGGER_RTT_printf(0, "\x1B[32mfile name : %s\x1B[0m\n", output_file);

            // PASS THIS : check if already exists
            // PASS THIS : create the file
            // PASS THIS : progress
        }

        if ((chunk_id == 17) && output_file && decompressed_size)
        {

            uint32_t remaining;

            /* uncompressed */
            switch (chunk_options)
            {
            /* stored, simply copy to output */
            case 0:

                /* read one block at at time, write and update checksum */

                remaining = chunk_size;
                checksum = 1L;
                for (;;)
                {
                    unsigned long r = (BLOCK_SIZE < remaining) ? BLOCK_SIZE : remaining;

                    if (bit_offset > 0) {
                        uint32_t block = BLOCK_SIZE - bit_offset - 1;
                        r = (block < remaining) ? block : remaining;
                    }
                    else {
                        r = (BLOCK_SIZE < remaining) ? BLOCK_SIZE : remaining;
                    }

                    uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t) * r);
                    read_memory_chunk(buffer, r, &block_offset_unpack, &bit_offset);

                    draw_bitmap_st7735_stream(buffer, r);

                    checksum = update_adler32(checksum, buffer, r);
                    free(buffer);

                    remaining -= r;

                    if (remaining <= 0)
                        break;
                }

                /* verify everything is written correctly */
                if (checksum != chunk_checksum)
                {
                    SEGGER_RTT_printf(0, "\nError: checksum mismatch. Aborted.\n");
                    SEGGER_RTT_printf(0, "Got %08lX Expecting %08lX\n", checksum, chunk_checksum);
                    free(output_file);
                    return -1;
                }
                break;

            /* compressed using FastLZ */
            case 1:

                compressed_buffer = (uint8_t*)malloc(sizeof(uint8_t) * chunk_size);
                decompressed_buffer = (uint8_t*)malloc(sizeof(uint8_t) * chunk_extra);

                read_memory_chunk(compressed_buffer, chunk_size, &block_offset_unpack, &bit_offset);

                checksum = update_adler32(1L, compressed_buffer, chunk_size);

                /* verify that the chunk data is correct */
                if (checksum != chunk_checksum)
                {
                    SEGGER_RTT_printf(0, "Error: checksum mismatch. Skipped.\n");
                    SEGGER_RTT_printf(0, "Got %08lX Expecting %08lX\n", checksum, chunk_checksum);
                    free(output_file);
                    free(compressed_buffer);
                    free(decompressed_buffer);
                    return -1;
                }
                else
                {
                    /* decompress and verify */
                    remaining = fastlz_decompress(compressed_buffer, chunk_size, decompressed_buffer, chunk_extra);
                    free(compressed_buffer);

                    if (remaining != chunk_extra)
                    {
                        SEGGER_RTT_printf(0, "\nError: decompression failed. Skipped.\n");
                    }
                    else {
                        draw_bitmap_st7735_stream(decompressed_buffer, chunk_extra);
                    }
                    free(decompressed_buffer);
                }
                break;

            default:
                SEGGER_RTT_printf(0, "\nError: unknown compression method (%d)\n", chunk_options);
                free(output_file);
                output_file = 0;
                break;
            }
        }
        /* position of next chunk */
        count += 16 + chunk_size;
    }

    /* free allocated stuff */
    free(output_file);

    SEGGER_RTT_printf(0, "\nFINISHED\n");

    /* so far so good */
    return 0;
}

#endif // UNPACK__