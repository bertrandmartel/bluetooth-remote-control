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
#ifndef UNPACK_H__
#define UNPACK_H__

#define BLOCK_SIZE (8192)

/* for Adler-32 checksum algorithm, see RFC 1950 Section 8.2 */
#define ADLER32_BASE 65521

uint32_t store_data_pstorage();

int detect_magic();

void read_memory_chunk(uint8_t * output_buffer, int length, uint8_t *block_offset, uint16_t *bit_offset);

void read_chunk_header(uint8_t *block_offset_var, uint16_t *bit_offset, uint16_t* id, uint16_t* options, uint32_t* size,
                       uint32_t* checksum, uint32_t* extra);

int unpack_file();

#endif // UNPACK_H__