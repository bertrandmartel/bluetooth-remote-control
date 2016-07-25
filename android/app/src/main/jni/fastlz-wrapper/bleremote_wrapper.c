/**********************************************************************************
 * This file is part of BLE Remote   .                                             *
 * <p/>                                                                            *
 * Copyright (C) 2016  Bertrand Martel                                             *
 * <p/>                                                                            *
 * BLE Remote is free software: you can redistribute it and/or modify              *
 * it under the terms of the GNU General Public License as published by            *
 * the Free Software Foundation, either version 3 of the License, or               *
 * (at your option) any later version.                                             *
 * <p/>                                                                            *
 * BLE Remote is distributed in the hope that it will be useful,                   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   *
 * GNU General Public License for more details.                                    *
 * <p/>                                                                            *
 * You should have received a copy of the GNU General Public License               *
 * along with BLE Remote. If not, see <http://www.gnu.org/licenses/>.              *
 */
/*
  6PACK - file compressor using FastLZ (lightning-fast compression library)

  Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
/**
	bleremote_wrapper.c

	Android wrapper for BLE Remote

	@author Bertrand Martel
	@version 0.1
*/
#include "android/log.h"
#include <jni.h>
#include "fastlz.h"
#include <stdio.h>
#include <stdlib.h>

#undef PATH_SEPARATOR

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR '/'
#endif

/* magic identifier for 6pack file */
static unsigned char sixpack_magic[8] = {137, '6', 'P', 'K', 13, 10, 26, 10};

#define BLOCK_SIZE (8192)

/* prototypes */
static inline unsigned long update_adler32(unsigned long checksum, const void *buf, int len);
void usage(void);
int detect_magic(FILE *f);
void write_magic(FILE *f);
void write_chunk_header(FILE* f, int id, int options, unsigned long size,
unsigned long checksum, unsigned long extra);
unsigned long block_compress(const unsigned char* input, unsigned long length, unsigned char* output);
int pack_file_compressed(const char* input_file, int method, int level, FILE* f);
int pack_file(int compress_level, const char* input_file, const char* output_file);

/* for Adler-32 checksum algorithm, see RFC 1950 Section 8.2 */
#define ADLER32_BASE 65521
static inline unsigned long update_adler32(unsigned long checksum, const void *buf, int len)
{
  const unsigned char* ptr = (const unsigned char*)buf;
  unsigned long s1 = checksum & 0xffff;
  unsigned long s2 = (checksum >> 16) & 0xffff;

  while(len>0)
  {
    unsigned k = len < 5552 ? len : 5552;
    len -= k;

    while(k >= 8)
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

    while(k-- > 0)
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
int detect_magic(FILE *f)
{
  unsigned char buffer[8];
  size_t bytes_read;
  int c;

  fseek(f, SEEK_SET, 0);
  bytes_read = fread(buffer, 1, 8, f);
  fseek(f, SEEK_SET, 0);
  if(bytes_read < 8)
    return 0;

  for(c = 0; c < 8; c++)
    if(buffer[c] != sixpack_magic[c])
      return 0;

  return -1;
}

void write_magic(FILE *f)
{
  fwrite(sixpack_magic, 8, 1, f);
}

void write_chunk_header(FILE* f, int id, int options, unsigned long size,
  unsigned long checksum, unsigned long extra)
{
  unsigned char buffer[16];

  buffer[0] = id & 255;
  buffer[1] = id >> 8;
  buffer[2] = options & 255;
  buffer[3] = options >> 8;
  buffer[4] = size & 255;
  buffer[5] = (size >> 8) & 255;
  buffer[6] = (size >> 16) & 255;
  buffer[7] = (size >> 24) & 255;
  buffer[8] = checksum & 255;
  buffer[9] = (checksum >> 8) & 255;
  buffer[10] = (checksum >> 16) & 255;
  buffer[11] = (checksum >> 24) & 255;
  buffer[12] = extra & 255;
  buffer[13] = (extra >> 8) & 255;
  buffer[14] = (extra >> 16) & 255;
  buffer[15] = (extra >> 24) & 255;

  fwrite(buffer, 16, 1, f);
}

int pack_file_compressed(const char* input_file, int method, int level, FILE* f)
{
  FILE* in;
  unsigned long fsize;
  unsigned long checksum;
  const char* shown_name;
  unsigned char buffer[BLOCK_SIZE];
  unsigned char result[BLOCK_SIZE*2]; /* FIXME twice is too large */
  unsigned char progress[20];
  int c;
  unsigned long percent;
  unsigned long total_read;
  unsigned long total_compressed;
  int chunk_size;

  /* sanity check */
  in = fopen(input_file, "rb");
  if(!in)
  {
    printf("Error: could not open %s\n", input_file);
    return -1;
  }

  /* find size of the file */
  fseek(in, 0, SEEK_END);
  fsize = ftell(in);
  fseek(in, 0, SEEK_SET);

  /* already a 6pack archive? */
  if(detect_magic(in))
  {
    printf("Error: file %s is already a 6pack archive!\n", input_file);
    fclose(in);
    return -1;
  }

  /* truncate directory prefix, e.g. "foo/bar/FILE.txt" becomes "FILE.txt" */
  shown_name = input_file + strlen(input_file) - 1;
  while(shown_name > input_file)
    if(*(shown_name-1) == PATH_SEPARATOR)
      break;
    else
      shown_name--;

  /* chunk for File Entry */
  buffer[0] = fsize & 255;
  buffer[1] = (fsize >> 8) & 255;
  buffer[2] = (fsize >> 16) & 255;
  buffer[3] = (fsize >> 24) & 255;
#if 0
  buffer[4] = (fsize >> 32) & 255;
  buffer[5] = (fsize >> 40) & 255;
  buffer[6] = (fsize >> 48) & 255;
  buffer[7] = (fsize >> 56) & 255;
#else
  /* because fsize is only 32-bit */
  buffer[4] = 0;
  buffer[5] = 0;
  buffer[6] = 0;
  buffer[7] = 0;
#endif
  buffer[8] = (strlen(shown_name)+1) & 255;
  buffer[9] = (strlen(shown_name)+1) >> 8;
  checksum = 1L;
  checksum = update_adler32(checksum, buffer, 10);
  checksum = update_adler32(checksum, shown_name, strlen(shown_name)+1);
  write_chunk_header(f, 1, 0, 10+strlen(shown_name)+1, checksum, 0);
  fwrite(buffer, 10, 1, f);
  fwrite(shown_name, strlen(shown_name)+1, 1, f);
  total_compressed = 16 + 10 + strlen(shown_name)+1;

  /* for progress status */
  memset(progress, ' ', 20);
  if(strlen(shown_name) < 16)
    for(c = 0; c < (int)strlen(shown_name); c++)
      progress[c] = shown_name[c];
  else
  {
    for(c = 0; c < 13; c++)
      progress[c] = shown_name[c];
    progress[13] = '.';
    progress[14] = '.';
    progress[15] = ' ';
  }
  progress[16] = '[';
  progress[17] = 0;
  printf("%s", progress);
  for(c = 0; c < 50; c++)
    printf(".");
  printf("]\r");
  printf("%s", progress);

  /* read file and place in archive */
  total_read = 0;
  percent = 0;
  for(;;)
  {
    int compress_method = method;
    int last_percent = (int)percent;
    size_t bytes_read = fread(buffer, 1, BLOCK_SIZE, in);
    if(bytes_read == 0)
      break;
    total_read += bytes_read;

    /* for progress */
    if(fsize < (1<<24))
      percent = total_read * 100 / fsize;
    else
      percent = total_read/256 * 100 / (fsize >>8);
    percent >>= 1;
    while(last_percent < (int)percent)
    {
      printf("#");
      last_percent++;
    }

    /* too small, don't bother to compress */
    if(bytes_read < 32)
      compress_method = 0;

    /* write to output */
    switch(compress_method)
    {
      /* FastLZ */
      case 1:
        chunk_size = fastlz_compress_level(level, buffer, bytes_read, result);
        checksum = update_adler32(1L, result, chunk_size);
        write_chunk_header(f, 17, 1, chunk_size, checksum, bytes_read);
        fwrite(result, 1, chunk_size, f);
        total_compressed += 16;
        total_compressed += chunk_size;
        break;

      /* uncompressed, also fallback method */
      case 0:
      default:
        checksum = 1L;
        checksum = update_adler32(checksum, buffer, bytes_read);
        write_chunk_header(f, 17, 0, bytes_read, checksum, bytes_read);
        fwrite(buffer, 1, bytes_read, f);
        total_compressed += 16;
        total_compressed += bytes_read;
        break;
    }
  }

  fclose(in);
  if(total_read != fsize)
  {
    printf("\n");
    printf("Error: reading %s failed!\n", input_file);
    return -1;
  }
  else
  {
    printf("] ");
    if(total_compressed < fsize)
    {
      if(fsize < (1<<20))
        percent = total_compressed * 1000 / fsize;
      else
        percent = total_compressed/256 * 1000 / (fsize >>8);
      percent = 1000 - percent;
      printf("%2d.%d%% saved", (int)percent/10, (int)percent%10);
    }
    printf("\n");
  }

  return 0;
}

int pack_file(int compress_level, const char* input_file, const char* output_file)
{
  FILE* f;
  int result;

  f = fopen(output_file, "rb");
  if(f)
  {
    fclose(f);
    printf("Error: file %s already exists. Aborted.\n\n", output_file);
    return -1;
  }

  f = fopen(output_file, "wb");
  if(!f)
  {
    printf("Error: could not create %s. Aborted.\n\n", output_file);
    return -1;
  }

  write_magic(f);

  result = pack_file_compressed(input_file, 1, compress_level, f);
  fclose(f);

  return result;
}

JNIEXPORT jint JNICALL Java_com_github_akinaru_bleremote_service_BleDisplayRemoteService_pack(JNIEnv* env, 
	jobject obj,jstring input_file_str,jstring output_file_str)
{
	const char *input_file = (*env)->GetStringUTFChars(env, input_file_str, 0);
	const char *output_file = (*env)->GetStringUTFChars(env, output_file_str, 0);

	int ret = pack_file(2, input_file, output_file);

	(*env)->ReleaseStringUTFChars(env, input_file_str, input_file);
	(*env)->ReleaseStringUTFChars(env, output_file_str, output_file);
	
	return ret;
}
