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
 * examples/powercut_test/powercut_main.c
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
#define TEST_FILE 					ROOT "sample_file"
#define SIZE_1 						20000
#define SIZE_2 						30000

/****************************************************************************
 * powercut_main
 ****************************************************************************/

int initialize(char** data_source_1, char** data_source_2, char** data_source_3)
{
	int ret = 0;
	// Alocate data sources
	*data_source_1 = (char *) malloc(SIZE_1 * sizeof(char) + 1);
	*data_source_2 = (char *) malloc(SIZE_1 * sizeof(char) + 1);
	*data_source_3 = (char *) malloc(SIZE_2 * sizeof(char) + 1);
	if(*data_source_1 == NULL || *data_source_2 == NULL || *data_source_3 == NULL) {
		ret = -1;
		goto errout;
	}
	memset(*data_source_1, '1', SIZE_1 * sizeof(char));
	memset(*data_source_2, '2', SIZE_1 * sizeof(char));
	memset(*data_source_3, '3', SIZE_2 * sizeof(char));
	errout:
		return ret;
}

int operation_recovery(char* data_source_1, char* data_source_2, int* op_no)
{
	int ret = 0;
	int fd = -1;
	int file_size, i, j;
	char* buffer = NULL;
	struct stat file_status;
	// check if file present
	ret = stat(TEST_FILE, &file_status);
    if (ret < 0) {
		if(errno == ENOENT) {
			*op_no = 1;
			printf("File Not Present\n");
			ret = OK;
		}
		else{
			printf("Some Error\n");
		}
        goto errout;
    }
	// get length of the file
	file_size = file_status.st_size;
	if(file_size < SIZE_1) {
		fd = open(TEST_FILE, O_RDWR);
		if(fd < 0) {
			ret = fd;
			goto errout;
		}
		buffer = (char *) malloc(sizeof(char) + 1);
		if(buffer == NULL) {
			ret = -1;
			goto errout;
		}
		ret = read(fd, buffer, 1);
		if (ret < 0) {
			goto errout;
		}
		if(buffer[0]=='4'){
			printf("partially deleted file\n");
		}
		else if(buffer[0]=='1'){
			printf("partially created file\n");
		}
		else{
			printf("partially created file with first character = %c\n", buffer[0]);
		}
		*op_no = 4;
		goto errout;
	}
	else if(file_size == SIZE_1) {
		fd = open(TEST_FILE, O_RDWR);
		if(fd < 0) {
			ret = fd;
			goto errout;
		}
		buffer = (char *) malloc(sizeof(char) + 1);
		if(buffer == NULL) {
			ret = -1;
			goto errout;
		}
		ret = read(fd, buffer, 1);
		if (ret < 0) {
			goto errout;
		}
		if (buffer[0] == '1') {
			printf("Overwrite Case with 1 as first character\n");
			*op_no = 2;
			goto errout;
		}
		else if(buffer[0] == '2') {
			printf("Overwrite Case with 2 as first character\n");
			free(buffer);
			buffer = (char *) malloc(file_size * sizeof(char) + 1);
			if(buffer == NULL) {
				ret = -1;
				goto errout;
			}
			ret = read(fd, &buffer[1], file_size - 1);
			if (ret < 0) {
				goto errout;
			}
			for(i = 1; i < file_size; i++) {
				if(buffer[i] != data_source_2[i]) {
					break;
				}
			}
			if(i != file_size) {
				printf("partially overwrite fail\n");
				ret = lseek(fd, 0, SEEK_SET);
				if(ret < 0) {
					goto errout;
				}
				// overwrite file with data_source_2
				ret = write(fd, data_source_2, SIZE_1);
				if (ret != SIZE_1) {
					goto errout;
				}
			}
			else {
				printf("No partial overwrite\n");
			}
		}
		else{
			printf("Overwrite Case with first character as %c\n", buffer[0]);
		}
		*op_no = 3;
		goto errout;
	}
	else if(file_size > SIZE_1 && file_size < (SIZE_1 + SIZE_2)) {
		printf("partially appended file\n");
	}
	else{
		printf("Normal appended file\n");
	}
	*op_no = 4;
	errout:
		if(fd >= 0) {
			close(fd);
		}
		if(buffer != NULL) {
			free(buffer);
		}
		return ret;
}

int operation_loop(int* op_no, char* data_source_1, char* data_source_2, char* data_source_3)
{
	int fd = -1;
	int ret = 0;
	// char buffer[2];
	
	switch (*op_no)
	{
	// if file is not created than create one and sync it with data_source_1
	case 1:
		printf("Starting Create\n");
		fd = open(TEST_FILE, O_CREAT | O_RDWR);
		if(fd < 0) {
			ret = fd;
			goto errout;
		}
		ret = write(fd, data_source_1, SIZE_1);
		if (ret != SIZE_1) {
			goto errout;
		}
		printf("Ending Create\n");
		*op_no=2;
		break;
	// if file is already there in FS then overwrite it with data_source_2
	case 2:
		fd = open(TEST_FILE, O_RDWR);
		if(fd < 0) {
			ret = fd;
			goto errout;
		}
		printf("Starting Overwrite\n");
		ret = write(fd, data_source_2, SIZE_1);
		if (ret != SIZE_1) {
			goto errout;
		}
		printf("Ending Overwrite\n");
		*op_no = 3;
		break;
	//  Apped the file with data_source_3
	case 3:
		fd = open(TEST_FILE, O_APPEND | O_RDWR);
		if(fd < 0) {
			ret = fd;
			goto errout;
		}
		printf("Starting Append\n");
		ret = write(fd, data_source_3, SIZE_2);
		if (ret != SIZE_2) {
			goto errout;
		}
		printf("Ending Append\n");
		*op_no = 4;
		break;
	// delete partially created or appended file
	case 4:
		fd = open(TEST_FILE, O_RDWR);
		if(fd < 0) {
			ret = fd;
			goto errout;
		}
		ret = write(fd, "4", 1);
		if (ret != 1) {
			goto errout;
		}
		printf("Written character 4\n");
		close(fd);
		fd = -1;
		printf("Starting Delete\n");
		unlink(TEST_FILE);
		printf("Ending Delete\n");
		*op_no = 1;
		break;
	default:
		break;
	}
	errout:
		if(fd >= 0) {
			close(fd);
		}
		return ret;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int powercut_main(int argc, char *argv[])
#endif
{
	char* data_source_1 = NULL;
	char* data_source_2 = NULL;
	char* data_source_3 = NULL;
	int ret = 0;
	int op_no = 4;
	// initialize sources
	ret = initialize(&data_source_1, &data_source_2, &data_source_3);
	if (ret < 0) {
		printf("Error Allocating Sources, errno: %d\n", errno);
		goto errout;
	}
	// perform recovery
	ret = operation_recovery(data_source_1, data_source_2, &op_no);
	if (ret < 0) {
		printf("Error on Operation recovery, errno: %d\n", errno);
		goto errout;
	}
	// perform operation loop
	while (true) {
		ret = operation_loop(&op_no, data_source_1, data_source_2, data_source_3);
		if(ret < 0) {
			printf("Error on Operation Loop, errno: %d\n", errno);
			break;
		}
	}	
errout:
	if(data_source_1 != NULL) {
		free(data_source_1);
	}
	if(data_source_2 != NULL) {
		free(data_source_2);
	}
	if(data_source_3 != NULL) {
		free(data_source_3);
	}
	return ret;
}
