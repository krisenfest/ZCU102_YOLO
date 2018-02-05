/******************************************************************************
 * (c) Copyright 2017 Xilinx, Inc. All rights reserved.
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

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev-user.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "helper.h"
#include "ov13850_cfg.h"
#include "video_int.h"

#define I2C_MUX_DEV_STR		"/dev/i2c-1"
#define I2C_MUX_ADDR  		0x75
#define I2C_MUX_REG		0x0
#define I2C_MUX_VAL		0x1
#define I2C_IOEXP_ADDR		0x3c
#define I2C_IOEXP_REG_1		0x1
#define I2C_IOEXP_VAL_1		0xff
#define I2C_IOEXP_REG_2		0x3
#define I2C_IOEXP_VAL_2		0xcd

#define I2C_SENSOR_DEV_STR	"/dev/i2c-2"
#define OV13850_SENSOR_ADDR	0x10

/* I2C sensor data structure */
struct i2c_regval {
	uint16_t addr;
	uint8_t data;
};

static const struct i2c_regval ov13850_4l_2160p30_regs[] = {
	{ 0x0103, 0x01 },
	{ 0x0300, 0x00 },
	{ 0x0301, 0x00 },
	{ 0x0302, 0x32 },
	{ 0x0303, 0x00 },
	{ 0x030a, 0x00 },
	{ 0x300f, 0x10 },
	{ 0x3010, 0x01 },
	{ 0x3011, 0x76 },
	{ 0x3012, 0x41 },
	{ 0x3013, 0x12 },
	{ 0x3014, 0x11 },
	{ 0x301f, 0x03 },
	{ 0x3106, 0x00 },
	{ 0x3210, 0x47 },
	{ 0x3500, 0x00 },
	{ 0x3501, 0xc0 },
	{ 0x3502, 0x00 },
	{ 0x3506, 0x00 },
	{ 0x3507, 0x02 },
	{ 0x3508, 0x00 },
	{ 0x350a, 0x00 },
	{ 0x350b, 0xf8 },
	{ 0x350e, 0x00 },
	{ 0x350f, 0x10 },
	{ 0x351a, 0x00 },
	{ 0x351b, 0x10 },
	{ 0x351c, 0x00 },
	{ 0x351d, 0x20 },
	{ 0x351e, 0x00 },
	{ 0x351f, 0x40 },
	{ 0x3520, 0x00 },
	{ 0x3521, 0x80 },
	{ 0x3600, 0xc0 },
	{ 0x3601, 0xfc },
	{ 0x3602, 0x02 },
	{ 0x3603, 0x78 },
	{ 0x3604, 0xb1 },
	{ 0x3605, 0x95 },
	{ 0x3606, 0x73 },
	{ 0x3607, 0x07 },
	{ 0x3609, 0x40 },
	{ 0x360a, 0x30 },
	{ 0x360b, 0x91 },
	{ 0x360C, 0x09 },
	{ 0x360f, 0x02 },
	{ 0x3611, 0x10 },
	{ 0x3612, 0x08 },
	{ 0x3613, 0x33 },
	{ 0x3614, 0x2a },
	{ 0x3615, 0x0c },
	{ 0x3616, 0x0e },
	{ 0x3641, 0x02 },
	{ 0x3660, 0x82 },
	{ 0x3667, 0xa0 },
	{ 0x3668, 0x54 },
	{ 0x3669, 0x00 },
	{ 0x366a, 0x3f },
	{ 0x3702, 0x40 },
	{ 0x3703, 0x44 },
	{ 0x3704, 0x2c },
	{ 0x3705, 0x01 },
	{ 0x3706, 0x15 },
	{ 0x3707, 0x44 },
	{ 0x3708, 0x3c },
	{ 0x3709, 0x1f },
	{ 0x370a, 0x24 },
	{ 0x370b, 0x3c },
	{ 0x3710, 0x28 },
	{ 0x3716, 0x03 },
	{ 0x3718, 0x10 },
	{ 0x3719, 0x0c },
	{ 0x371a, 0x08 },
	{ 0x371b, 0x01 },
	{ 0x371c, 0xfc },
	{ 0x3720, 0x55 },
	{ 0x3722, 0x84 },
	{ 0x3728, 0x40 },
	{ 0x372a, 0x05 },
	{ 0x372b, 0x02 },
	{ 0x372e, 0x22 },
	{ 0x372f, 0xa0 },
	{ 0x3730, 0x02 },
	{ 0x3731, 0x5c },
	{ 0x3732, 0x02 },
	{ 0x3733, 0x70 },
	{ 0x3738, 0x02 },
	{ 0x3739, 0x72 },
	{ 0x373a, 0x02 },
	{ 0x373b, 0x74 },
	{ 0x3740, 0x01 },
	{ 0x3741, 0xd0 },
	{ 0x3742, 0x00 },
	{ 0x3743, 0x01 },
	{ 0x3748, 0x21 },
	{ 0x3749, 0x22 },
	{ 0x374a, 0x28 },
	{ 0x3760, 0x13 },
	{ 0x3761, 0x33 },
	{ 0x3762, 0x86 },
	{ 0x3763, 0x16 },
	{ 0x3767, 0x24 },
	{ 0x3768, 0x06 },
	{ 0x3769, 0x45 },
	{ 0x376c, 0x23 },
	{ 0x376f, 0x80 },
	{ 0x3773, 0x06 },
	{ 0x3780, 0x90 },
	{ 0x3781, 0x00 },
	{ 0x3782, 0x01 },
	{ 0x3800, 0x00 },
	{ 0x3801, 0x0C },
	{ 0x3802, 0x00 },
	{ 0x3803, 0x04 },
	{ 0x3804, 0x10 },
	{ 0x3805, 0x93 },
	{ 0x3806, 0x0c },
	{ 0x3807, 0x4B },
	{ 0x3808, 0x0F },
	{ 0x3809, 0x00 },
	{ 0x380a, 0x08 },
	{ 0x380b, 0x70 },
	{ 0x380c, 0x1e },
	{ 0x380d, 0x78 },
	{ 0x380e, 0x0d },
	{ 0x380f, 0x00 },
	{ 0x3810, 0x00 },
	{ 0x3811, 0xc4 },
	{ 0x3812, 0x01 },
	{ 0x3813, 0xec },
	{ 0x3814, 0x11 },
	{ 0x3815, 0x11 },
	{ 0x3820, 0x04 },
	{ 0x3821, 0x04 },
	{ 0x3823, 0x00 },
	{ 0x3826, 0x00 },
	{ 0x3827, 0x02 },
	{ 0x3834, 0x00 },
	{ 0x3835, 0x1c },
	{ 0x3836, 0x04 },
	{ 0x3837, 0x01 },
	{ 0x3d84, 0x00 },
	{ 0x3d85, 0x17 },
	{ 0x3d8c, 0x73 },
	{ 0x3d8d, 0xbf },
	{ 0x4000, 0xf1 },
	{ 0x4001, 0x00 },
	{ 0x400b, 0x0c },
	{ 0x4011, 0x00 },
	{ 0x401a, 0x00 },
	{ 0x401b, 0x00 },
	{ 0x401c, 0x00 },
	{ 0x401d, 0x00 },
	{ 0x4020, 0x03 },
	{ 0x4021, 0x6C },
	{ 0x4022, 0x0D },
	{ 0x4023, 0x17 },
	{ 0x4024, 0x0D },
	{ 0x4025, 0xFC },
	{ 0x4026, 0x0D },
	{ 0x4027, 0xFF },
	{ 0x4028, 0x00 },
	{ 0x4029, 0x02 },
	{ 0x402a, 0x04 },
	{ 0x402b, 0x08 },
	{ 0x402c, 0x02 },
	{ 0x402d, 0x02 },
	{ 0x402e, 0x0c },
	{ 0x402f, 0x08 },
	{ 0x403d, 0x2c },
	{ 0x403f, 0x7F },
	{ 0x4041, 0x07 },
	{ 0x4500, 0x82 },
	{ 0x4501, 0x38 },
	{ 0x458b, 0x00 },
	{ 0x459c, 0x00 },
	{ 0x459d, 0x00 },
	{ 0x459e, 0x00 },
	{ 0x4601, 0x04 },
	{ 0x4602, 0x22 },
	{ 0x4603, 0x00 },
	{ 0x4837, 0x0D },
	{ 0x4d00, 0x04 },
	{ 0x4d01, 0x42 },
	{ 0x4d02, 0xd1 },
	{ 0x4d03, 0x90 },
	{ 0x4d04, 0x66 },
	{ 0x4d05, 0x65 },
	{ 0x4d0b, 0x00 },
	{ 0x5000, 0x0e },
	{ 0x5001, 0x01 },
	{ 0x5002, 0x07 },
	{ 0x5003, 0x4f },
	{ 0x5013, 0x40 },
	{ 0x501c, 0x00 },
	{ 0x501d, 0x10 },
	{ 0x5100, 0x30 },
	{ 0x5101, 0x02 },
	{ 0x5102, 0x01 },
	{ 0x5103, 0x01 },
	{ 0x5104, 0x02 },
	{ 0x5105, 0x01 },
	{ 0x5106, 0x01 },
	{ 0x5107, 0x00 },
	{ 0x5108, 0x00 },
	{ 0x5109, 0x00 },
	{ 0x510f, 0xfc },
	{ 0x5110, 0xf0 },
	{ 0x5111, 0x10 },
	{ 0x536d, 0x02 },
	{ 0x536e, 0x67 },
	{ 0x536f, 0x01 },
	{ 0x5370, 0x4c },
	{ 0x5400, 0x00 },
	{ 0x5401, 0x71 },
	{ 0x5402, 0x00 },
	{ 0x5403, 0x00 },
	{ 0x5404, 0x00 },
	{ 0x5405, 0x80 },
	{ 0x540c, 0x05 },
	{ 0x5501, 0x00 },
	{ 0x5b00, 0x00 },
	{ 0x5b01, 0x00 },
	{ 0x5b02, 0x01 },
	{ 0x5b03, 0xff },
	{ 0x5b04, 0x02 },
	{ 0x5b05, 0x6c },
	{ 0x5b09, 0x02 },
	{ 0x5e00, 0x00 },
	{ 0x5e10, 0x1c },
};

static const struct i2c_regval ov13850_4l_1080p60_regs[] = {
	{ 0x0103, 0x01 },
	{ 0x0100, 0x00 },
	{ 0x0300, 0x00 },
	{ 0x0301, 0x00 },
	{ 0x0302, 0x30 },
	{ 0x0303, 0x00 },
	{ 0x030a, 0x00 },
	{ 0x300f, 0x10 },
	{ 0x3010, 0x01 },
	{ 0x3011, 0x76 },
	{ 0x3012, 0x41 },
	{ 0x3013, 0x12 },
	{ 0x3014, 0x11 },
	{ 0x301f, 0x03 },
	{ 0x3106, 0x00 },
	{ 0x3210, 0x47 },
	{ 0x3500, 0x00 },
	{ 0x3501, 0x67 },
	{ 0x3502, 0x80 },
	{ 0x3506, 0x00 },
	{ 0x3507, 0x02 },
	{ 0x3508, 0x00 },
	{ 0x3509, 0x10 },
	{ 0x350a, 0x00 },
	{ 0x350b, 0xf8 },
	{ 0x350e, 0x00 },
	{ 0x350f, 0x10 },
	{ 0x3600, 0x40 },
	{ 0x3601, 0xFC },
	{ 0x3602, 0x02 },
	{ 0x3603, 0x48 },
	{ 0x3604, 0xA5 },
	{ 0x3605, 0x9F },
	{ 0x3607, 0x00 },
	{ 0x360a, 0x40 },
	{ 0x360b, 0x91 },
	{ 0x360c, 0x49 },
	{ 0x360f, 0x8A },
	{ 0x3611, 0x13 },
	{ 0x3612, 0x27 },
	{ 0x3613, 0x33 },
	{ 0x3614, 0x60 },
	{ 0x3615, 0x0C },
	{ 0x3641, 0x02 },
	{ 0x3660, 0x82 },
	{ 0x3667, 0xA0 },
	{ 0x3668, 0x54 },
	{ 0x3669, 0x40 },
	{ 0x3702, 0x40 },
	{ 0x3703, 0x44 },
	{ 0x3704, 0x2C },
	{ 0x3705, 0x24 },
	{ 0x3706, 0x50 },
	{ 0x3707, 0x44 },
	{ 0x3708, 0x3C },
	{ 0x3709, 0x1F },
	{ 0x370a, 0x26 },
	{ 0x370b, 0x3C },
	{ 0x3710, 0x28 },
	{ 0x3716, 0x03 },
	{ 0x3718, 0x10 },
	{ 0x3719, 0x08 },
	{ 0x371c, 0xFC },
	{ 0x3720, 0x66 },
	{ 0x3722, 0x84 },
	{ 0x3728, 0x40 },
	{ 0x372a, 0x00 },
	{ 0x372f, 0x90 },
	{ 0x3760, 0x13 },
	{ 0x3761, 0x34 },
	{ 0x3767, 0x24 },
	{ 0x3768, 0x06 },
	{ 0x3769, 0x45 },
	{ 0x376c, 0x23 },
	{ 0x3800, 0x00 },
	{ 0x3801, 0x08 },
	{ 0x3802, 0x01 },
	{ 0x3803, 0xEC },
	{ 0x3804, 0x10 },
	{ 0x3805, 0x97 },
	{ 0x3806, 0x0A },
	{ 0x3807, 0x63 },
	{ 0x3808, 0x07 },
	{ 0x3809, 0x80 },
	{ 0x380a, 0x04 },
	{ 0x380b, 0x38 },
	{ 0x380c, 0x09 },
	{ 0x380d, 0x60 },
	{ 0x380e, 0x04 },
	{ 0x380f, 0x9D },
	{ 0x3810, 0x00 },
	{ 0x3811, 0x64 },
	{ 0x3812, 0x00 },
	{ 0x3813, 0x02 },
	{ 0x3814, 0x31 },
	{ 0x3815, 0x31 },
	{ 0x3820, 0x06 },
	{ 0x3821, 0x06 },
	{ 0x3834, 0x00 },
	{ 0x3835, 0x1C },
	{ 0x3836, 0x08 },
	{ 0x3837, 0x02 },
	{ 0x3d84, 0x00 },
	{ 0x3d85, 0x17 },
	{ 0x3d8c, 0x73 },
	{ 0x3d8d, 0xBF },
	{ 0x4000, 0xF1 },
	{ 0x4001, 0x00 },
	{ 0x400b, 0x0C },
	{ 0x4011, 0x00 },
	{ 0x401a, 0x00 },
	{ 0x401b, 0x00 },
	{ 0x401c, 0x00 },
	{ 0x401d, 0x00 },
	{ 0x4020, 0x02 },
	{ 0x4021, 0xAC },
	{ 0x4022, 0x04 },
	{ 0x4023, 0xD7 },
	{ 0x4024, 0x05 },
	{ 0x4025, 0xBC },
	{ 0x4026, 0x05 },
	{ 0x4027, 0xBF },
	{ 0x4028, 0x00 },
	{ 0x4029, 0x02 },
	{ 0x402a, 0x04 },
	{ 0x402b, 0x08 },
	{ 0x402c, 0x02 },
	{ 0x402d, 0x02 },
	{ 0x402e, 0x0C },
	{ 0x402f, 0x08 },
	{ 0x403d, 0x2C },
	{ 0x403f, 0x7F },
	{ 0x4500, 0x82 },
	{ 0x4501, 0x38 },
	{ 0x4601, 0x77 },
	{ 0x4602, 0x22 },
	{ 0x4603, 0x01 },
	{ 0x4603, 0x01 },
	{ 0x4802, 0x42 },
	{ 0x481a, 0x00 },
	{ 0x481b, 0x1C },
	{ 0x4826, 0x12 },
	{ 0x4837, 0x0D },
	{ 0x4d00, 0x04 },
	{ 0x4d01, 0x42 },
	{ 0x4d02, 0xD1 },
	{ 0x4d03, 0x90 },
	{ 0x4d04, 0x66 },
	{ 0x4d05, 0x65 },
	{ 0x5000, 0x0E },
	{ 0x5001, 0x03 },
	{ 0x5002, 0x07 },
	{ 0x5013, 0x40 },
	{ 0x501c, 0x00 },
	{ 0x501d, 0x10 },
	{ 0x5242, 0x00 },
	{ 0x5243, 0xB8 },
	{ 0x5244, 0x00 },
	{ 0x5245, 0xF9 },
	{ 0x5246, 0x00 },
	{ 0x5247, 0xF6 },
	{ 0x5248, 0x00 },
	{ 0x5249, 0xA6 },
	{ 0x5300, 0xFC },
	{ 0x5301, 0xDF },
	{ 0x5302, 0x3F },
	{ 0x5303, 0x08 },
	{ 0x5304, 0x0C },
	{ 0x5305, 0x10 },
	{ 0x5306, 0x20 },
	{ 0x5307, 0x40 },
	{ 0x5308, 0x08 },
	{ 0x5309, 0x08 },
	{ 0x530a, 0x02 },
	{ 0x530b, 0x01 },
	{ 0x530c, 0x01 },
	{ 0x530d, 0x0C },
	{ 0x530e, 0x02 },
	{ 0x530f, 0x01 },
	{ 0x5310, 0x01 },
	{ 0x5400, 0x00 },
	{ 0x5401, 0x61 },
	{ 0x5401, 0x61 },
	{ 0x5402, 0x00 },
	{ 0x5403, 0x00 },
	{ 0x5404, 0x00 },
	{ 0x5405, 0x40 },
	{ 0x5405, 0x40 },
	{ 0x540c, 0x05 },
	{ 0x5b00, 0x00 },
	{ 0x5b01, 0x00 },
	{ 0x5b02, 0x01 },
	{ 0x5b03, 0xFF },
	{ 0x5b04, 0x02 },
	{ 0x5b05, 0x6C },
	{ 0x5b09, 0x02 },
	{ 0x5e00, 0x00 },
	{ 0x5e10, 0x1C },
};

static const struct i2c_regval ov13850_4l_720p60_regs[] = {
	{ 0x0103, 0x01 },
	{ 0x0300, 0x00 },
	{ 0x0301, 0x00 },
	{ 0x0302, 0x30 },
	{ 0x0303, 0x00 },
	{ 0x030a, 0x00 },
	{ 0x300f, 0x10 },
	{ 0x3010, 0x01 },
	{ 0x3011, 0x76 },
	{ 0x3012, 0x41 },
	{ 0x3013, 0x12 },
	{ 0x3014, 0x11 },
	{ 0x301f, 0x03 },
	{ 0x3106, 0x00 },
	{ 0x3210, 0x47 },
	{ 0x3500, 0x00 },
	{ 0x3501, 0x67 },
	{ 0x3502, 0x80 },
	{ 0x3506, 0x00 },
	{ 0x3507, 0x02 },
	{ 0x3508, 0x00 },
	{ 0x3509, 0x10 },
	{ 0x350a, 0x00 },
	{ 0x350b, 0xF8 },
	{ 0x350e, 0x00 },
	{ 0x350f, 0x10 },
	{ 0x3600, 0x40 },
	{ 0x3601, 0xFC },
	{ 0x3602, 0x02 },
	{ 0x3603, 0x48 },
	{ 0x3604, 0xA5 },
	{ 0x3605, 0x9F },
	{ 0x3607, 0x00 },
	{ 0x360a, 0x40 },
	{ 0x360b, 0x91 },
	{ 0x360c, 0x49 },
	{ 0x360f, 0x8A },
	{ 0x3611, 0x13 },
	{ 0x3612, 0x27 },
	{ 0x3613, 0x33 },
	{ 0x3614, 0x64 },
	{ 0x3615, 0x0C },
	{ 0x3641, 0x02 },
	{ 0x3660, 0x82 },
	{ 0x3667, 0xA0 },
	{ 0x3668, 0x54 },
	{ 0x3669, 0x40 },
	{ 0x3702, 0x40 },
	{ 0x3703, 0x44 },
	{ 0x3704, 0x2C },
	{ 0x3705, 0x24 },
	{ 0x3706, 0x50 },
	{ 0x3707, 0x44 },
	{ 0x3708, 0x3C },
	{ 0x3709, 0x1F },
	{ 0x370a, 0x26 },
	{ 0x370b, 0x3C },
	{ 0x3710, 0x28 },
	{ 0x3716, 0x03 },
	{ 0x3718, 0x10 },
	{ 0x3719, 0x08 },
	{ 0x371c, 0xFC },
	{ 0x3720, 0x66 },
	{ 0x3722, 0x84 },
	{ 0x3728, 0x40 },
	{ 0x372a, 0x00 },
	{ 0x372f, 0x90 },
	{ 0x3760, 0x13 },
	{ 0x3761, 0x34 },
	{ 0x3767, 0x24 },
	{ 0x3768, 0x06 },
	{ 0x3769, 0x45 },
	{ 0x376c, 0x23 },
	{ 0x3800, 0x02 },
	{ 0x3801, 0x88 },
	{ 0x3802, 0x03 },
	{ 0x3803, 0x54 },
	{ 0x3804, 0x0E },
	{ 0x3805, 0x17 },
	{ 0x3806, 0x08 },
	{ 0x3807, 0xFB },
	{ 0x3808, 0x05 },
	{ 0x3809, 0x00 },
	{ 0x380a, 0x02 },
	{ 0x380b, 0xD0 },
	{ 0x380c, 0x09 },
	{ 0x380d, 0x60 },
	{ 0x380e, 0x03 },
	{ 0x380f, 0x39 },
	{ 0x3810, 0x00 },
	{ 0x3811, 0x64 },
	{ 0x3812, 0x00 },
	{ 0x3813, 0x02 },
	{ 0x3814, 0x31 },
	{ 0x3815, 0x31 },
	{ 0x3820, 0x06 },
	{ 0x3821, 0x06 },
	{ 0x3834, 0x00 },
	{ 0x3835, 0x1C },
	{ 0x3836, 0x08 },
	{ 0x3837, 0x02 },
	{ 0x3d84, 0x00 },
	{ 0x3d85, 0x17 },
	{ 0x3d8c, 0x73 },
	{ 0x3d8d, 0xBF },
	{ 0x4000, 0xF1 },
	{ 0x4001, 0x00 },
	{ 0x400b, 0x0C },
	{ 0x4011, 0x00 },
	{ 0x401a, 0x00 },
	{ 0x401b, 0x00 },
	{ 0x401c, 0x00 },
	{ 0x401d, 0x00 },
	{ 0x4020, 0x00 },
	{ 0x4021, 0x2C },
	{ 0x4022, 0x04 },
	{ 0x4023, 0xD7 },
	{ 0x4024, 0x05 },
	{ 0x4025, 0xBC },
	{ 0x4026, 0x05 },
	{ 0x4027, 0xBF },
	{ 0x4028, 0x00 },
	{ 0x4029, 0x02 },
	{ 0x402a, 0x04 },
	{ 0x402b, 0x08 },
	{ 0x402c, 0x02 },
	{ 0x402d, 0x02 },
	{ 0x402e, 0x0C },
	{ 0x402f, 0x08 },
	{ 0x403d, 0x2C },
	{ 0x403f, 0x7F },
	{ 0x4500, 0x82 },
	{ 0x4501, 0x38 },
	{ 0x4601, 0x4F },
	{ 0x4602, 0x22 },
	{ 0x4603, 0x01 },
	{ 0x4603, 0x01 },
	{ 0x4802, 0x42 },
	{ 0x481a, 0x00 },
	{ 0x481b, 0x1C },
	{ 0x4826, 0x12 },
	{ 0x4837, 0x0D },
	{ 0x4d00, 0x04 },
	{ 0x4d01, 0x42 },
	{ 0x4d02, 0xD1 },
	{ 0x4d03, 0x90 },
	{ 0x4d04, 0x66 },
	{ 0x4d05, 0x65 },
	{ 0x5000, 0x0E },
	{ 0x5001, 0x03 },
	{ 0x5002, 0x07 },
	{ 0x5013, 0x40 },
	{ 0x501c, 0x00 },
	{ 0x501d, 0x10 },
	{ 0x5242, 0x00 },
	{ 0x5243, 0xB8 },
	{ 0x5244, 0x00 },
	{ 0x5245, 0xF9 },
	{ 0x5246, 0x00 },
	{ 0x5247, 0xF6 },
	{ 0x5248, 0x00 },
	{ 0x5249, 0xA6 },
	{ 0x5300, 0xFC },
	{ 0x5301, 0xDF },
	{ 0x5302, 0x3F },
	{ 0x5303, 0x08 },
	{ 0x5304, 0x0C },
	{ 0x5305, 0x10 },
	{ 0x5306, 0x20 },
	{ 0x5307, 0x40 },
	{ 0x5308, 0x08 },
	{ 0x5309, 0x08 },
	{ 0x530a, 0x02 },
	{ 0x530b, 0x01 },
	{ 0x530c, 0x01 },
	{ 0x530d, 0x0C },
	{ 0x530e, 0x02 },
	{ 0x530f, 0x01 },
	{ 0x5310, 0x01 },
	{ 0x5400, 0x00 },
	{ 0x5401, 0x61 },
	{ 0x5401, 0x61 },
	{ 0x5402, 0x00 },
	{ 0x5403, 0x00 },
	{ 0x5404, 0x00 },
	{ 0x5405, 0x40 },
	{ 0x5405, 0x40 },
	{ 0x540c, 0x05 },
	{ 0x5b00, 0x00 },
	{ 0x5b01, 0x00 },
	{ 0x5b02, 0x01 },
	{ 0x5b03, 0xFF },
	{ 0x5b04, 0x02 },
	{ 0x5b05, 0x6C },
	{ 0x5b09, 0x02 },
	{ 0x5e00, 0x00 },
	{ 0x5e10, 0x1C },
};

static int i2c_init(void)
{
	int fd, ret;

	fd = open(I2C_MUX_DEV_STR, O_RDWR);
	if (fd < 0) {
		vlib_err("Error opening file: %s\n", strerror(errno));
		return VLIB_ERROR_FILE_IO;
	}

	/* Initialize Mux */
	ret = ioctl(fd, I2C_SLAVE_FORCE, I2C_MUX_ADDR);
	if (ret < 0) {
		vlib_err("Invalid i2c mux addr 0x%x\n", I2C_MUX_ADDR);
		goto err;
	}

	ret = i2c_smbus_write_byte_data(fd, I2C_MUX_REG, I2C_MUX_VAL);
	if (ret < 0) {
		vlib_err("I2c write to i2c mux addr 0x%x failed\n", I2C_MUX_ADDR);
		goto err;
	}

	/* Initialize IO Expander */
	ret = ioctl(fd, I2C_SLAVE_FORCE, I2C_IOEXP_ADDR);
	if (ret < 0) {
		vlib_err("Invalid i2c io expander addr 0x%x\n", I2C_IOEXP_ADDR);
		goto err;
	}

	ret = i2c_smbus_write_byte_data(fd, I2C_IOEXP_REG_1, I2C_IOEXP_VAL_1);
	if (ret < 0) {
		vlib_err("Invalid i2c io expander addr 0x%x write to 0x%x\n",
		I2C_IOEXP_ADDR, I2C_IOEXP_REG_1);
		goto err;
	}

	ret = i2c_smbus_write_byte_data(fd, I2C_IOEXP_REG_2, I2C_IOEXP_VAL_2);
	if (ret < 0) {
		vlib_err("Invalid i2c io expander addr 0x%x write to 0x%x\n",
		I2C_IOEXP_ADDR, I2C_IOEXP_REG_2);
		goto err;
	}

	return 0;

err:
	close(fd);
	return ret;
}

static int i2c_write(int fd, const struct i2c_regval *reginfo)
{
	uint8_t data[4];

	data[0] = reginfo->addr & 0xff;
	data[1] = reginfo->data;
	return i2c_smbus_write_i2c_block_data(fd, reginfo->addr >> 8, 2, data);
}

int ov13850_setup(unsigned int width, unsigned int height)
{
	int cnt, fd, ret;
	const struct i2c_regval *cfg;

	ret = i2c_init();
	if (ret < 0) {
		vlib_err("I2c initialization failed\n");
		return VLIB_ERROR_OTHER;
	}

	if ((width == 3840) && (height == 2160)) {
		cnt = ARRAY_SIZE(ov13850_4l_2160p30_regs);
		cfg = ov13850_4l_2160p30_regs;
	} else if ((width == 1920) && (height == 1080)) {
		cnt = ARRAY_SIZE(ov13850_4l_1080p60_regs);
		cfg = ov13850_4l_1080p60_regs;
	} else if ((width == 1280) && (height == 720)) {
		cnt = ARRAY_SIZE(ov13850_4l_720p60_regs);
		cfg = ov13850_4l_720p60_regs;
	} else {
		sprintf(vlib_errstr,
			"OV13850 does not support requested resolution '%dx%d'",
			width, height);
		vlib_info("Continue with previous mode\n");
		return VLIB_ERROR_NOT_SUPPORTED;
	}

	fd = open(I2C_SENSOR_DEV_STR, O_RDWR);
	if (fd < 0) {
		vlib_err("Error opening file: %s\n", strerror(errno));
		return VLIB_ERROR_FILE_IO;
	}

	ret = ioctl(fd, I2C_SLAVE, OV13850_SENSOR_ADDR);
	if (ret < 0) {
		vlib_err("Invalid i2c sensor addr 0x%x\n", OV13850_SENSOR_ADDR);
		goto err;
	}

	for (int i = 0; i < cnt; i++) {
		ret = i2c_write(fd, &cfg[i]);
		if (ret) {
			vlib_err("Write to sensor failed\n");
			goto err;
		}
	}

	return 0;

err:
	close(fd);
	return ret;
}

static int ov13850_start_stop(int cmd)
{
	int fd, ret;
	uint8_t data[4];
	struct i2c_regval startcmd;

	startcmd.addr = 0x0100;
	startcmd.data = cmd;

	fd = open(I2C_SENSOR_DEV_STR, O_RDWR);
	if (fd < 0) {
		vlib_err("Error opening file: %s\n", strerror(errno));
		return VLIB_ERROR_FILE_IO;
	}

	ret = ioctl(fd, I2C_SLAVE, OV13850_SENSOR_ADDR);
	if (ret < 0) {
		vlib_err("Invalid i2c sensor addr 0x%x\n", OV13850_SENSOR_ADDR);
		goto err;
	}

	data[0] = startcmd.addr & 0xff;
	data[1] = startcmd.data;
	ret = i2c_smbus_write_i2c_block_data(fd, startcmd.addr >> 8, 2, data);
	if (ret < 0) {
		vlib_err("I2c write to sensor failed\n");
		goto err;
	}

	return 0;

err:
	close(fd);
	return ret;
}

int ov13850_start(void)
{
	return ov13850_start_stop(1);
}

int ov13850_stop(void)
{
	return ov13850_start_stop(0);
}
