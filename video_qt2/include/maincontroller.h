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

/*
 * This file defines GUI helper functions.
 */

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QQuickItem>
#include <QTimer>
#include <pthread.h>
#include <qglobal.h>
#include <QtCharts/QAbstractSeries>
#include "CPUStat.h"
#include "video.h"
#include "vcap_tpg.h"
#if defined (SAMPLE_FILTER2D)
#include "filter2d.h"
#endif
#if defined (SAMPLE_OPTICAL_FLOW)
#include "optical_flow.h"
#endif
#if defined (SAMPLE_SIMPLE_POSTERIZE)
#include "simple_posterize.h"
#endif

QT_BEGIN_NAMESPACE
class QQuickView;
QT_END_NAMESPACE

#define HISTORY_LENGTH 60
QT_CHARTS_USE_NAMESPACE

class maincontroller : public QObject
{
Q_OBJECT
	video_ctrl videoCtrl;
	vlib_config config;
	QQuickItem *rootObject;
	pthread_t autoloop;
	QTimer * demo_timer;
	QTimer * fps_timer;
	QTimer * demo_timer_filter;
	vlib_config demo_config;
	struct filter_tbl *ft;
	int demo_filter_loop_count;
	int demo_src_loop_count;
	int demo_perset_loop_count;
	// TODO move array initialization to contructor/init function
	int demo_sequence[9][3];
	int GLOBAL;
	int demoTimerInterval;
	int demoSequenceLength;

	enum CpuData
	{
		Cpu1,
		Cpu2,
		Cpu3,
		Cpu4,
		NCpuData
	};
	CPUStat *cpuStat1;
	CPUStat *cpuStat2;
	CPUStat *cpuStat3;
	CPUStat *cpuStat4;
	QVector<qreal> cpu1List;
	QVector<qreal> cpu2List;
	QVector<qreal> cpu3List;
	QVector<qreal> cpu4List;

	enum MemData
	{
		videoSrc,
		filter,
		videoPort,
		NMemData
	};
	QVector<qreal> videoSrcList;
	QVector<qreal> filterList;
	QVector<qreal> videoPortList;

	enum demoSequenceParams{
		SOURCE_TYPE,
		FILTER_TYPE,
		FILTER_MODE
	};

public slots:
	// TODO use contructor instead of function
	void init(QQuickItem *,vlib_config,struct filter_tbl *);
	void setVideo(int);
	void setFilterMode(int);
	void setFilterType(int);
	void setInput(int);
	void closeall();
//	void setInvert(int);
//	void setSensitivity(int,int);
	void setTPGPattern(int);
	void setPreset(int);
	void demoSequence(QVariantList seqList);
	void updateDemoTimer(int timerval);
	void demoMode(bool);
	void updateFilterOptions(int);
	void updateinputOptions(int);
	void updateFilterTypeOptions(int);
	void videoSrcLoop();
	void getFps();
	void setFgOverlay(int);
	void setBoxSize(int);
	void setBoxColor(int);
	void setBoxSpeed(int);
	void setCrossRows(int);
	void setCrossColumns(int);
	void setZoneH(int);
	void setZoneHDelta(int);
	void setZoneV(int);
	void setZoneVDelta(int);
	void filterCoeff(int,int,int,int,int,int,int,int,int);
	void errorPopup(int);
	void updatecpu(QAbstractSeries *cpu1, QAbstractSeries *cpu2, QAbstractSeries *cpu3, QAbstractSeries *cpu4);
	void updateThroughput(QAbstractSeries *videoSrc, QAbstractSeries *accelerator, QAbstractSeries *displayPort);
};

#endif // MAINCONTROLLER_H
