/******************************************************************************
 * (c) Copyright 2012-2016 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *******************************************************************************/

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "filter.h"
#include "helper.h"
#include "video_int.h"

#define FILTER_PR_BIN_SIZE 753848

/**
 * filter_type_register - register a filter with vlib
 * @ft:	Pointer to filter table
 * @fs: Pointer to filter struct to register
 *
 * Return: 0 on success, error code otherwise.
 */
int filter_type_register(struct filter_tbl *ft, struct filter_s *fs)
{
	if (!ft || !fs)
		return VLIB_ERROR_INVALID_PARAM;

	if (!ft->filter_types) {
		ft->filter_types = g_ptr_array_new();
		if (!ft->filter_types) {
			return VLIB_ERROR_OTHER;
		}
	}

	g_ptr_array_add(ft->filter_types, fs);

	vlib_info("Filter %s registered successfully!\n",
		  filter_type_get_display_text(fs));

	ft->size = ft->filter_types->len;

	return 0;
}

int filter_type_unregister(struct filter_tbl *ft, struct filter_s *fs)
{
	if (!ft || !fs)
		return -1;

	g_ptr_array_remove_fast(ft->filter_types, fs);
	ft->size = ft->filter_types->len;

	if (!ft->size) {
		g_ptr_array_free(ft->filter_types, TRUE);
	}

	vlib_info("Filter %s unregistered successfully!\n", filter_type_get_display_text(fs));

	return ft->size;
}

struct filter_s *filter_type_get_obj(struct filter_tbl *ft, unsigned int i)
{
	if (ft && i < ft->size)
		return g_ptr_array_index(ft->filter_types, i);

	return NULL;
}

int filter_type_match(struct filter_s *fs, const char *str)
{
	if (fs && !strcmp(filter_type_get_display_text(fs), str))
		return 1;
	else
		return 0;
}

void filter_type_set_mode(struct filter_s *fs, filter_mode mode)
{
	ASSERT2(fs, " %s :: argument NULL\n", __func__);
	fs->mode = mode;
}

const char *filter_type_get_display_text(const struct filter_s *fs)
{
	ASSERT2(fs, " %s :: argument NULL\n", __func__);
	return fs->display_text;
}

int filter_type_prefetch_bin(struct filter_s *fs)
{
	char file_name[128];
	char *buf;
	int fd;
	int ret;

	if (fs && fs->pr_file_name[0] != '\0') {
		// compose file name
		sprintf(file_name, "/media/card/partial/%s", fs->pr_file_name);

		// open partial bitfile
		fd = open(file_name, O_RDONLY);
		if (fd < 0) {
			vlib_err("failed to open partial bitfile %s\n", file_name);
			return -1;
		}

		// allocate buffer and read partial bitfile into buffer
		buf = (char *) malloc(FILTER_PR_BIN_SIZE);
		ret = read(fd, buf, FILTER_PR_BIN_SIZE);
		if (ret < 0) {
			vlib_err("failed to read partial bitfile %s\n", file_name);
			close(fd);
			return -1;
		}

		// store buffer address and close file handle
		fs->pr_buf = buf;
		close(fd);
	}

	return 0;
}

int filter_type_free_bin(struct filter_s *fs)
{
	/* In case arg to free is NULL, no action occurs */
	if (fs)
		free(fs->pr_buf);

	return 0;
}

int filter_type_config_bin(struct filter_s *fs)
{
	if (!(fs && fs->pr_buf)) {
		return 0;
	}

	// Set is_partial_bitfile device attribute
	int fd = open("/sys/devices/soc0/amba/f8007000.devcfg/is_partial_bitstream", O_RDWR);
	if (fd < 0) {
		vlib_err("failed to set xdevcfg attribute 'is_partial_bitstream'\n");
		return VLIB_ERROR_FILE_IO;
	}

	int ret = write(fd, "1", 2);
	if (ret != 2) {
		vlib_err("failed to set xdevcfg attribute 'is_partial_bitstream'\n");
		ret = VLIB_ERROR_FILE_IO;
		goto err_close;
	}
	close(fd);

	// Write partial bitfile to xdevcfg device
	fd = open("/dev/xdevcfg", O_RDWR);
	if (fd < 0) {
		vlib_err("failed to open xdevcfg device\n");
		return VLIB_ERROR_FILE_IO;
	}
	ret = write(fd, fs->pr_buf, FILTER_PR_BIN_SIZE);
	if (ret != FILTER_PR_BIN_SIZE) {
		vlib_err("failed to open xdevcfg device\n");
		ret = VLIB_ERROR_FILE_IO;
		goto err_close;
	}

err_close:
	close(fd);

	return ret;
}

const char *filter_mode2str(filter_mode m)
{
	switch (m) {
	case FILTER_MODE_OFF:
		return "off";
	case FILTER_MODE_SW:
		return "software";
	case FILTER_MODE_HW:
		return "hardware";
	default:
		return "invalid";
	}
}
