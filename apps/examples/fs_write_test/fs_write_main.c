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
//#include <tinyara/timer.h>
#include <time.h>
#include <sys/time.h>

/****************************************************************************
 * fs_write_main
 ****************************************************************************/


void test()
{
	for (int bufSize = 60; bufSize <= 240; bufSize *= 2) {
		int totalTime = 0;
		char writeBuffer[bufSize];

		for (int i = 0; i < bufSize; i++) {
			writeBuffer[i] = 'a';
		}

		for (int itr=0; itr < 20; itr++){

			//time_t start, end;

			struct timeval start, end;
			gettimeofday(&start, NULL);

			//time(&start);

			for(int file = 1; file<10; file++){

				int fd;
				char fileName[20];

				snprintf(fileName, 20, "/mnt/file_%d.txt", file);

				fd = open(fileName, O_WRONLY | O_CREAT);

				if(fd<0){
					printf("Unable to create file, ret = %d\n", errno);
					continue;
				}

				write(fd, writeBuffer, bufSize);
				close(fd);

				for(int i=1; i<200; i++){
					fd = open(fileName, O_WRONLY | O_CREAT);

					if (fd < 0) {
						printf("Unable to create file, ret = %d\n", errno);
						break;
					}


					lseek(fd, 0, SEEK_END);
					write(fd, writeBuffer, bufSize);
					close(fd);
				}

			}

			//time(&end);
			gettimeofday(&end, NULL);

			for(int file=0; file<10; file++){
				char fileName[20];
				snprintf(fileName, 20, "/mnt/file_%d.txt", file);
				unlink(fileName);
			}

			//double itrTime = difftime(end,start);

			int itrTime = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;

			printf("Time taken: %lu microseconds\n", itrTime);
			totalTime+=itrTime;

		}

		printf("Total time taken for buffer of size: %d bytes: %lu microseconds\n",bufSize,totalTime);

	}
}

//#define CONFIG_BUILD_KERNEL 1

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fs_write_main(int argc, char *argv[])
#endif
{
	printf("Beginning Test\n");
	test();
	return 0;
}
