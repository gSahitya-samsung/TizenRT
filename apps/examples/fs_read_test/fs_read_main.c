/****************************************************************************
 *
 * Copyright 2023 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * examples/fs_write_test/fs_write_main.c
 *
 *   Copyright (C) 2008, 2011-2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <tinyara/timer.h>
#include <time.h>
#include <sys/time.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>

#define ROOT 						"/mnt/"
#define SAMPLE_FILE 					ROOT "sample_file"
#define TEST_DIR_SINGLE 				ROOT "single"
#define TEST_FILE_SINGLE 				ROOT "single/test_file"
#define TEST_DIR_MULTIPLE 				ROOT "multiple"
#define TEST_FILE_MULTIPLE 				ROOT "multiple/test_file_"
#define TEST_FILE_NAME_LEN_MAX 				42
#define TEST_ITR 					10
#define TIMER_DEVICE 					"/dev/timer0"
#define FS_PART_SIZE 					512

int frt_fd = -1;
int fs_fd = -1;
// #define OK 0

/****************************************************************************
 * fs_write_main
 ****************************************************************************/


int validate(char *buf, char *fileName, int nChar, int times, int offset)
{
	#ifdef CONFIG_EXAMPLES_FS_WRITE_VALIDATE

	int fd, ret, i, j;
	char *tempBuf;
	bool valid;

	fd = open(fileName, O_RDONLY);

	if(fd < 0) {
		printf("Unable to open file = %s, error = %s\n", fileName, strerror(errno));
		printf("Write Verification Failed\n");
	}

	ret = lseek(fd, offset, SEEK_SET);

	if(ret < 0) {
		printf("Unable to lseek for file = %s, error = %s\n", fileName, strerror(errno));
		goto errHandler;
	}


	tempBuf = (char *) malloc(nChar + 1);

	valid = true;

	for (i = 0; (i < times) && valid; i++) {

		ret = read(fd, tempBuf, nChar);

		if (ret != nChar) {
			printf("%d\n", nChar);
			printf("Unable to read file = %s, error = %s, ret = %d\n",fileName, strerror(errno), ret);
			goto errHandler;
		}


		for (j = 0; (j < nChar) && valid ; j++) {
			if (buf[i] != tempBuf[i]) {
				printf("Write Verification Failed\n");
				printf("Expected to write: %.*s\n", nChar, buf);
				printf("Written: %s\n", tempBuf);
				valid = false;
			}
		}
	}



	if (valid) {
		printf("Write Verification Success\n");
		printf("Expected to write: %.*s\n", nChar, buf);
		printf("Written: %s\n", tempBuf);
	}


	free(tempBuf);
	close(fd);
	return OK;

	errHandler:
		close(fd);
		free(tempBuf);
		printf("Write Verification Failed\n");
		return -errno;
	#else
	return OK;
	#endif

}

int init_sample_file(int buffer_size)
{
	char buf[] = "Writing data into sample file\n";
	int fd = open(SAMPLE_FILE, O_CREAT | O_WRONLY);
	int bufLen, i, wrtLen;

	if (fd < 0) {
		printf("Unable to open sample file: %s, fd = %d\n", SAMPLE_FILE, fd);
		printf("%s\n", strerror(errno));
		return -ENOMEM;
	}

	bufLen = strlen(buf);

	for (i = 0; i < (buffer_size / bufLen) + 1; i++) {

		wrtLen = write(fd, buf, bufLen);

		if (wrtLen != bufLen) {
			printf("Unable to write to sample file, fd = %d, write length = %d", fd, wrtLen);
			close(fd);
			return -errno;
		}
	}

	close(fd);
	printf("Sample File Initialization Complete\n\n");
	return OK;
}

int test()
{
	unsigned int long long totalTime, itrTime;
	int ret, fd, i, bufSize, itr, nFiles, file;
	char readBuffer[1024 + 1];
	char multiFileName[TEST_FILE_NAME_LEN_MAX];
	char fileName[50];
	struct timer_status_s start, end;



	for (bufSize = 64; bufSize <= 1024; bufSize *= 2) {
		totalTime = 0;
		for (itr = 0; itr < 20; itr++) {
			// gettimeofday(&start, NULL);
			ioctl(frt_fd, TCIOC_START, TRUE);
			ioctl(frt_fd, TCIOC_GETSTATUS, (unsigned long)(uintptr_t)&start);
			ioctl(fs_fd, 0, 0);
			fd = open(SAMPLE_FILE, O_RDONLY);

			if (fd < 0) {
				printf("Failed to open sample file for reading\n");
				goto fileCreateError;
			}
			
			for (i = 0; i < 64 * 1024 / bufSize; i++) {

				ret = read(fd, readBuffer, bufSize);

				if (ret != bufSize) {
					printf("Unable to read from file = %s, error = %s\n", SAMPLE_FILE, strerror(errno));
					close(fd);
					goto fileCreateError;
				}
			}

			close(fd);
			ioctl(frt_fd, TCIOC_GETSTATUS, (unsigned long)(uintptr_t)&end);
			ioctl(frt_fd, TCIOC_STOP, 0);
			ioctl(fs_fd, 0, 0);

			// gettimeofday(&end, NULL);

			// itrTime = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
			itrTime =  end.timeleft - start.timeleft;

			printf("Time taken: %llu microseconds\n", itrTime);
			totalTime += itrTime;
		}

		printf("Total time taken for buffer of size: %d bytes: %llu microseconds\n", bufSize, totalTime);
	}


	return OK;

	fileCreateError:
		printf("Error = %s\n", strerror(errno));
		return -errno;

	runtimeError:
		return -1;
}

// #define CONFIG_BUILD_KERNEL 1

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fs_read_main(int argc, char *argv[])
#endif
{
	printf("Testing\n");
	// printf("Fs_Fd = %d\n",fs_fd);
	int ret = init_sample_file(64 * 1024);

	if (ret != OK) {
		printf("Unable to init sample test file, ret = %d\n", ret);
		goto errHandler;
	}

	frt_fd = open("/dev/timer0", O_RDONLY);
	ioctl(frt_fd, TCIOC_SETMODE, MODE_FREERUN);

	fs_fd = open("/mnt/driver.txt", O_RDONLY | O_CREAT);
	test();
	close(frt_fd);
	close(fs_fd);
errHandler:
	return ret;
}
