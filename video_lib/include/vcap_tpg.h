/******************************************************************************
 * (c) Copyright 2016-2017 Xilinx, Inc. All rights reserved.
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

#ifndef VCAP_TPG_H
#define VCAP_TPG_H

#ifdef __cplusplus
extern "C"
{
#endif

#define TPG_BG_PATTERN_CNT 17

struct vlib_vdev;

/* tpg helper functions */
const char *tpg_get_pattern_menu_name(unsigned int idx);
void tpg_set_blanking(const struct vlib_vdev *vd, unsigned int vblank,
		      unsigned int hblank, unsigned int ppc);
void tpg_set_bg_pattern(const struct vlib_vdev *vd, unsigned int bg);
void tpg_set_fg_pattern(const struct vlib_vdev *vd, unsigned int fg);
void tpg_set_box_size(const struct vlib_vdev *vd, unsigned int size);
void tpg_set_box_color(const struct vlib_vdev *vd, unsigned int color);
void tpg_set_box_speed(const struct vlib_vdev *vd, unsigned int speed);
void tpg_set_cross_hair_num_rows(const struct vlib_vdev *vd, unsigned int row);
void tpg_set_cross_hair_num_columns(const struct vlib_vdev *vd, unsigned int column);
void tpg_set_zplate_hor_cntl_start(const struct vlib_vdev *vd, unsigned int hstart);
void tpg_set_zplate_hor_cntl_delta(const struct vlib_vdev *vd, unsigned int hspeed);
void tpg_set_zplate_ver_cntl_start(const struct vlib_vdev *vd, unsigned int vstart);
void tpg_set_zplate_ver_cntl_delta(const struct vlib_vdev *vd, unsigned int vspeed);
#ifdef __cplusplus
}
#endif

#endif /* VCAP_TPG_H */
