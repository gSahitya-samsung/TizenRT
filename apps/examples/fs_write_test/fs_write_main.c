/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
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
 * examples/hello/hello_main.c
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

#define ROOT                            "/mnt/"
#define SAMPLE_FILE                     ROOT "sample_file"
#define TEST_DIR_SINGLE                 ROOT "single"
#define TEST_FILE_SINGLE                ROOT "single/test_file"
#define TEST_DIR_MULTIPLE               ROOT "multiple"
#define TEST_FILE_MULTIPLE              ROOT "multiple/test_file_"
#define TEST_FILE_NAME_LEN_MAX          42
#define TEST_ITR                        10
#define TIMER_DEVICE 			"/dev/timer0"
#define FS_PART_SIZE			512


// #define OK 0
#define ERR -1

int frt_fd = -1;

/****************************************************************************
 * fs_write_main
 ****************************************************************************/

int init_sample_file(int buffer_size)
{
	char buf[] = "Writing data into sample file\n";
	int fd = open(SAMPLE_FILE, O_CREAT | O_WRONLY);

	if (fd < 0) {
		printf("Unable to open sample file: %s, fd = %d\n", SAMPLE_FILE, fd);
		printf("%s\n",strerror(errno));
		return -ENOMEM;
	}

	int bufLen = strlen(buf);

	for (int i = 0; i < (buffer_size / bufLen) + 1; i++) {

		int wrtLen = write(fd, buf, bufLen);

		if (wrtLen != bufLen) {
			printf("Unable to write to sample file, fd = %d, write length = %d", fd, wrtLen);
			close(fd);
			return ERR;
		}
	}

	close(fd);
	printf("Sample File Initialization Complete\n\n");
	return OK;
}

int create_PPE(const char *dir_path)
{
	int ret = mkdir(dir_path, S_IRWXG | S_IRWXO | S_IRWXU);

	if (ret < 0) {
		printf("Unable to create test directory = %s\n", dir_path);
		printf("%d\n",errno);
		printf("%s\n",strerror(errno));
		if(errno != 17){
			goto errHandler;
		}
	}

	int fd = open(SAMPLE_FILE, O_RDONLY);

	if (fd < 0) {
		printf("Unable to open sample file for reading\n");
		printf("%s\n",strerror(errno));
		printf(SAMPLE_FILE);
		goto errHandler;
	}

	char buf[64 * 1024 + 1];

	ret = read(fd, buf, 64 * 1024);

	if (ret != 64 *1024) {
		printf("Unable to read from Sample File\n, ret = %d", ret);
		close(fd);
		goto errHandler;
	}

	close(fd);

	for (int i = 0; i < ((FS_PART_SIZE - 128) / 64); i++) {
		printf("Writing Garbage file #%d\n", i);

		fd = open(TEST_FILE_SINGLE, O_CREAT | O_WRONLY);

		if (fd < 0) {
			printf("Failed to open garbage file for creation\n");
			goto errHandler;
		}

		ret = write(fd, buf, 64 * 1024);

		if (ret != 64 * 1024) {
			printf("Unable to write to garbage file\n");
			goto errHandler;
		}

		close(fd);
		unlink(TEST_FILE_SINGLE);
	}

	printf("PPE Exits\n");
	return OK;

	errHandler:
		printf("RRE Exits\n");
		return -EIO;

}

// int customWrite(int fd, char *buf, char *fileName, int size)
// {
// 	int ret = write(fd, buf, size);
// 	if (ret != size) {
// 		printf("Unable to write to file %s, fd = %d\n", fileName, ret);
// 		close(fd);
// 		return ERR;
// 	}
// 	return ret;
// }


int create_max_file(char *fileName, long int size)
{
	char buf[] = "Writing data to tha sample file\n";

	int fd = open(fileName, O_CREAT | O_WRONLY);

	if (fd < 0) {
		printf("Unable to open sample file: %s, fd = %d\n", fileName, fd);
		return -ENOMEM;
	}

	while (size>=strlen(buf)) {
		int ret = write(fd, buf, strlen(buf));

		if(ret != strlen(buf)) {
			printf("Unable to write to file %s, fd = %d\n", fileName, ret);
			close(fd);
			return -ENOMEM;
		}
		size -= strlen(buf);
	}

	if (size) {
		int ret = write(fd, buf, size);

		if (ret != size) {
			printf("Unable to write to file %s, ret = %d\n", fileName, ret);
			close(fd);
			return -ENOMEM;
		}

		size = 0;
	}

	close(fd);

	return OK;

}

int big_f_small_o()
{
	char buf[] = "This is data in the temporary buffer. This will be written to the file for testing Filesystems with TizenRT RTOS.\n";
	char fileName[TEST_FILE_NAME_LEN_MAX];

	int ret = create_PPE(TEST_DIR_SINGLE);

	if (ret != OK) {
		printf("Unable to create PPE\n");
		return ERR;
	}

	ret = mkdir(TEST_DIR_MULTIPLE, 0777);
	snprintf(fileName, TEST_FILE_NAME_LEN_MAX, "%s%d", TEST_FILE_MULTIPLE, 0);

	ret = create_max_file(fileName, 200 * 1024);

	if (ret == OK) {
		printf("File of size 200 KB created\n");
	}

	long int pos = 0;
	unsigned long int test_time = 0;

	int arr[] = {20, 50, 75, 100, 150, 200, -1};
	int n = sizeof(arr) / sizeof(arr[0]);

	for (int i = 0;  i < n; i++) {
		struct timeval start, end;
		gettimeofday(&start, NULL);

		for (int j = 0; j<50; j++) {
			int fd = open(fileName, O_WRONLY);

			if (fd < 0) {
				printf("Unable to open file %s\n", fileName);
				return ERR;
			}

			lseek(fd, pos, SEEK_SET);

			ret = write(fd, buf, strlen(buf));


			if (ret != strlen(buf)) {
				printf("Failed to write at pos = %ld, j = %d\n", pos, i);
				close(fd);
				return ERR;
			}

			close(fd);
		}

		gettimeofday(&end, NULL);

		unsigned long int total_time = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;

		printf("#############################################################################################\n");
		printf("Average time taken to overwrite %d bytes at position %ld is: %llu\n", strlen(buf), pos, total_time / 50);
		printf("#############################################################################################\n");
		pos = arr[i] * 1024;
	}

	return OK;
}

void test()
{

	char writeBuffer[240+1];

	int ret = create_PPE(TEST_DIR_SINGLE);

	if (ret != OK) {
            printf("Unable to create PPE\n");
            goto fileCreateError;
    }

	printf("PPE Complete\n");

	int fd = open(SAMPLE_FILE, O_RDONLY);

	if (fd < 0) {
        printf("Failed to open sample file for reading\n");
        goto fileCreateError;
    }

	ret = read(fd, writeBuffer, 240);

	if (ret != 240) {
		printf("Unable to read from sample file\n");
		printf("%s\n",strerror(errno));
	}

	close(fd);

	ret = mkdir(TEST_DIR_MULTIPLE, S_IRWXG | S_IRWXO | S_IRWXU);

	if (ret < 0) {
		printf("Unable to create test directory = %s\n", TEST_DIR_MULTIPLE);
		printf("%d\n",errno);
		printf("%s\n",strerror(errno));
		if(errno != 17){
			goto fileCreateError;
		}
	}

	for (int bufSize = 60; bufSize <= 60; bufSize *= 2) {
		int totalTime = 0;
		//memset(writeBuffer, 'a', bufSize * sizeof(char));

		for (int itr = 0; itr < 20; itr++) {

			//time_t start, end;

			struct timeval start, end;
			gettimeofday(&start, NULL);

			//time(&start);

			for(int file = 1; file < 10; file ++){

				// int fd;
				char fileName[30];

				snprintf(fileName, 30, "%s%d.txt", TEST_FILE_MULTIPLE, file);

				fd = open(fileName, O_WRONLY | O_CREAT);

				if(fd<0){
					printf("Unable to create file, ret = %d\n", errno);
					goto fileCreateError;
				}

				write(fd, writeBuffer, bufSize);
				close(fd);

				for(int i=1; i<200; i++){
					fd = open(fileName, O_WRONLY);

					if (fd < 0) {
						printf("Unable to create file, ret = %d\n", errno);
						goto fileCreateError;
					}


					lseek(fd, 0, SEEK_END);
					write(fd, writeBuffer, bufSize);
					close(fd);
				}

			}

			//time(&end);
			gettimeofday(&end, NULL);

			// for(int file = 0; file < 10; file++){
			// 	char fileName[30];
			// 	snprintf(fileName, 30, "%s%d.txt", TEST_FILE_MULTIPLE, file);
			// 	unlink(fileName);
			// }

			//double itrTime = difftime(end,start);

			int itrTime = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;

			printf("Time taken: %lu microseconds\n", itrTime);
			totalTime+=itrTime;

		}

		printf("Total time taken for buffer of size: %d bytes: %lu microseconds\n", bufSize, totalTime);

	}

	fileCreateError:
		return ;
}

// #define CONFIG_BUILD_KERNEL 1

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fs_write_main(int argc, char *argv[])
#endif
{
	int ret = init_sample_file(64 * 1024);

	if (ret != OK) {
		printf("Unable to init sample test file, ret = %d\n", ret);
		goto errHandler;
	}

	test();

	// ret = big_f_small_o();

	// if (ret != OK) {
	// 	printf("Big_F_Small_O test failed, ret = %d\n", ret);
	// 	goto errHandler;
	// }

	// printf("FS Performance Test Completed successfully\n");
	// ret = OK;

	errHandler:
		return ret;

	// return 0;
}
