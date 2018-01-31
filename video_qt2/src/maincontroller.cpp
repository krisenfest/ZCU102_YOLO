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
 * This file implements GUI helper functions.
 */

#include "maincontroller.h"
#include <unistd.h>
#include "video_cfg.h"
#include <QtCharts/QXYSeries>
#if defined (PERFAPM_CLIENT)
#include <perfapm-client.h>
#endif

/* Time to switch among video_sources in demo-mode */
#define SECONDS_TO_MILLISEC 1000

static bool demo_src = true;

void maincontroller :: init(QQuickItem* item, vlib_config cfg, struct filter_tbl *ft){
	rootObject = item;

	/* Start-up config/mode */
	videoCtrl = VIDEO_CTRL_OFF;
	config = cfg;
	this->ft = ft;

	int perfInitStatus = EXIT_FAILURE;
#if defined (PERFAPM_CLIENT)
	perfInitStatus = perfoacl_init_all(PERF_LINUX_OS, PERF_SAMPLE_INTERVAL_COUNTER);
#endif

	if(perfInitStatus == EXIT_SUCCESS){
		rootObject->setProperty("showMemoryThroughput", QVariant(1));
	}else{
		rootObject->setProperty("showMemoryThroughput", QVariant(0));
	}

	qRegisterMetaType<QAbstractSeries*>();
	qRegisterMetaType<QAbstractAxis*>();

	cpuStat1 = new CPUStat("cpu0");
	cpuStat2 = new CPUStat("cpu1");
	cpuStat3 = new CPUStat("cpu2");
	cpuStat4 = new CPUStat("cpu3");

	/* Demo timer */
	demo_timer = new QTimer(this);
	connect(demo_timer, SIGNAL(timeout()), this, SLOT(videoSrcLoop()));

	/* FPS timer */
	fps_timer = new QTimer(this);
	connect(fps_timer, SIGNAL(timeout()), this, SLOT(getFps()));
	fps_timer->start(1000); // 1 second
	demo_src = true;
}

void maincontroller :: setVideo(int play){
	if(play == 0){
		videoCtrl = VIDEO_CTRL_OFF;
		int err = vlib_pipeline_stop();
		errorPopup(err);
	}else{
		videoCtrl = VIDEO_CTRL_ON;
		int err = vlib_change_mode(&config);
		errorPopup(err);
	}
}

void maincontroller :: getFps(){
	// updating FPS
	if(demo_timer->isActive()){
		rootObject->setProperty("fpsValueText",QVariant("N/A"));
	}else{
		int fpsValue = vlib_get_event_cnt(DISPLAY);
		QString str = (QString::compare(QString::number(fpsValue),
				"nan",
				Qt::CaseInsensitive) == 0 ? "0": QString::number(fpsValue));
		str = str + QString::fromStdString(" fps");
		QVariant fpsval = QVariant(str);
		rootObject->setProperty("fpsValueText",QVariant(fpsval));
	}
}

void maincontroller :: setFilterMode(int filter){
	videoCtrl = VIDEO_CTRL_OFF;
	if(filter == 0){ //off
		config.mode = FILTER_MODE_OFF;
	}else if(filter == 1){ //sw
		 config.mode = FILTER_MODE_SW;
	}else{ //hw
		config.mode = FILTER_MODE_HW;
	}
	// set new state
	videoCtrl = VIDEO_CTRL_ON;

	// configure and start video pipeline
	int err = vlib_change_mode(&config);
	errorPopup(err);
}

void maincontroller :: setFilterType(int filter){
	videoCtrl = VIDEO_CTRL_OFF;
	config.type = filter;
	videoCtrl = VIDEO_CTRL_ON;

	// configure and start video pipeline
	int err = vlib_change_mode(&config);
	errorPopup(err);
}

void maincontroller :: setInput(int input){
	videoCtrl = VIDEO_CTRL_OFF;
	size_t prevVsrc = config.vsrc;
	config.vsrc = vlib_video_src_get_index(vsrc_get_vd((video_src)input));
	// set new state
	videoCtrl = VIDEO_CTRL_ON;

	int ret = vlib_change_mode(&config);
	QVariant msg;
	QVariant returnedValue;
	msg = true;
	errorPopup(ret);
	if (ret != VLIB_SUCCESS) {
		config.vsrc = prevVsrc;
		vlib_change_mode(&config);
		msg = false;
	}
	QMetaObject::invokeMethod(rootObject, "updateInput",
			Q_RETURN_ARG(QVariant, returnedValue),
			Q_ARG(QVariant, msg));
}

/*
void maincontroller :: setInvert(int input){
	 sobel_invert(fs_sobel, input);
}
void maincontroller :: setSensitivity(int max, int min){
	sobel_thresh(fs_sobel, max, min);
}
*/
void maincontroller:: setTPGPattern(int input){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_bg_pattern(vd, input+1); // Test pattern 0 is neglected.
}
void maincontroller:: setPreset(int input){
	UNUSED(input);
#if defined (SAMPLE_FILTER2D)
	if(input < FILTER2D_PRESET_CNT){
		filter2d_set_preset_coeff(filter_type_get_obj(ft, config.type), (filter2d_preset) input);
	}
	// Get api here for getcoeff
	coeff_t *coeff1 = filter2d_get_coeff((filter_type_get_obj(ft, config.type)));
	rootObject->setProperty("filter00Coeff",QVariant( ((*coeff1)[0][0]) + 9));
	rootObject->setProperty("filter01Coeff",QVariant( ((*coeff1)[0][1]) + 9));
	rootObject->setProperty("filter02Coeff",QVariant( ((*coeff1)[0][2]) + 9));
	rootObject->setProperty("filter10Coeff",QVariant( ((*coeff1)[1][0]) + 9));
	rootObject->setProperty("filter11Coeff",QVariant( ((*coeff1)[1][1]) + 9));
	rootObject->setProperty("filter12Coeff",QVariant( ((*coeff1)[1][2]) + 9));
	rootObject->setProperty("filter20Coeff",QVariant( ((*coeff1)[2][0]) + 9));
	rootObject->setProperty("filter21Coeff",QVariant( ((*coeff1)[2][1]) + 9));
	rootObject->setProperty("filter22Coeff",QVariant( ((*coeff1)[2][2]) + 9));
#endif
}

void maincontroller:: setFgOverlay(int overlay){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_fg_pattern(vd, overlay);
}
void maincontroller:: setBoxSize(int size){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_box_size(vd, size);
}
void maincontroller:: setBoxColor(int color){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_box_color(vd, color);
}
void maincontroller:: setBoxSpeed(int speed){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_box_speed(vd, speed);
}
void maincontroller:: setCrossRows(int rows){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_cross_hair_num_rows(vd, rows);
}
void maincontroller:: setCrossColumns(int columns){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_cross_hair_num_columns(vd, columns);
}
void maincontroller:: setZoneH(int h){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_zplate_hor_cntl_start(vd, h);
}
void maincontroller:: setZoneHDelta(int h){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_zplate_hor_cntl_delta(vd, h);
}
void maincontroller:: setZoneV(int v){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_zplate_ver_cntl_start(vd, v);
}
void maincontroller:: setZoneVDelta(int v){
	const struct vlib_vdev *vd = vsrc_get_vd(VLIB_VCLASS_TPG);

	tpg_set_zplate_hor_cntl_delta(vd, v);
}
void maincontroller:: filterCoeff(int c00,int c01,int c02,int c10,int c11,int c12,int c20,int c21,int c22){
	int coeff[3][3];
	coeff[0][0] = c00;
	coeff[0][1] = c01;
	coeff[0][2] = c02;
	coeff[1][0] = c10;
	coeff[1][1] = c11;
	coeff[1][2] = c12;
	coeff[2][0] = c20;
	coeff[2][1] = c21;
	coeff[2][2] = c22;
	UNUSED(coeff);
#if defined (SAMPLE_FILTER2D)
	filter2d_set_coeff(filter_type_get_obj(ft, config.type), coeff);
#endif
}
void maincontroller:: errorPopup(int errorno){
	if(!demo_src){
		demo_src = true;
		rootObject->setProperty("errorNameText",QVariant("Demo Error"));
		rootObject->setProperty("errorMessageText",QVariant("<b>SW</b> mode for <b>Optical Flow</b> is not supported"));
		QVariant msg;
		QVariant returnedValue;
		msg = true;
		QMetaObject::invokeMethod(rootObject, "abortDemoMode",
			Q_RETURN_ARG(QVariant, returnedValue),
			Q_ARG(QVariant, msg));
		demo_timer->stop();
		autoloop = 0;
		// stop video pipeline
		vlib_pipeline_stop();
		return;
	}

	if(errorno == VLIB_SUCCESS){
		return;
	}

	QVariant errorName = QString::fromUtf8(vlib_error_name((vlib_error) errorno));
	rootObject->setProperty("errorNameText",QVariant(errorName));

	QVariant errorMsg = QString::fromUtf8(vlib_strerror());
	rootObject->setProperty("errorMessageText",QVariant(errorMsg));
}

void maincontroller::closeall() {
#if defined (PERFAPM_CLIENT)
	perfoacl_deinit_all(PERF_LINUX_OS, PERF_SAMPLE_INTERVAL_COUNTER);
#endif
	demo_timer->stop();
	fps_timer->stop();
	vlib_pipeline_stop();
	vlib_uninit();
}
void maincontroller :: videoSrcLoop(){
	int count = 0;
	int arySize = demoSequenceLength;
	vlib_config d_config;
	while(1){
		if(arySize <= demo_src_loop_count){
			demo_src_loop_count = 0;
		}
		const struct vlib_vdev *vsrc = vsrc_get_vd((video_src)demo_sequence[demo_src_loop_count][SOURCE_TYPE]);
		bool isSwOptical = false;
		if(demo_sequence[demo_src_loop_count][FILTER_TYPE] != -1){
			isSwOptical = (demo_sequence[demo_src_loop_count][FILTER_MODE] == 1) && ( demo_sequence[demo_src_loop_count][FILTER_TYPE] == 1); // hard coded 1 for filter_type for considering 2nd filter is Optical flow
		}
		if (!isSwOptical && vsrc) { // checking for enabled sources.
			demo_src = true;
			demo_timer->start(demoTimerInterval);
			d_config.vsrc = vlib_video_src_get_index(vsrc);
			if(demo_sequence[demo_src_loop_count][FILTER_TYPE] != -1){
				d_config.mode = (filter_mode)demo_sequence[demo_src_loop_count][FILTER_MODE];
				d_config.type = demo_sequence[demo_src_loop_count][FILTER_TYPE];
			}
			int ret = vlib_change_mode(&d_config);
			if(ret != VLIB_SUCCESS){  // check for error while setting mode.
				demo_src_loop_count++;
				count++;
				if(count >= arySize){
					demo_src = false;
					demoMode(0);
					demo_timer->stop();
					rootObject->setProperty("auto_start",QVariant(false));
					break;
				}
				continue;
			}
			count = 0;
			config.vsrc = d_config.vsrc;
			updateinputOptions(vlib_video_src_get_class(vsrc));
			if(demo_sequence[demo_src_loop_count][FILTER_TYPE] != -1){
				config.mode = d_config.mode;
				updateFilterOptions(config.mode);
				config.type = d_config.type;
				updateFilterTypeOptions(config.type);
			}
		}else{
			demo_src_loop_count++;
			count ++;
			if(arySize <= count){
				demo_src_loop_count = 0;
				if(!demo_src){
					demoMode(0);
					demo_timer->stop();
					rootObject->setProperty("auto_start",QVariant(false));
					count = 0;
				}
				break;
			}else{
				continue;
			}
		}
		demo_src_loop_count ++;
		break;
	}
}
void maincontroller :: updateFilterOptions(int filter){
	rootObject->setProperty("filterOptSel",QVariant(filter));
	switch(filter){
	case 0:
		rootObject->setProperty("numericfilterlabelText",QVariant("OFF"));
		break;
	case 1:
		rootObject->setProperty("numericfilterlabelText",QVariant("SW"));
		break;
	case 2:
		rootObject->setProperty("numericfilterlabelText",QVariant("HW"));
		break;
	default:
		rootObject->setProperty("numericfilterlabelText",QVariant("NA"));
		break;
	}

}
void maincontroller :: updateinputOptions(int input){
	rootObject->setProperty("videoInput",QVariant(input));
	rootObject->setProperty("numericinputlabelText",QVariant(vsrc_get_short_name((video_src)input)));
}
void maincontroller :: updateFilterTypeOptions(int filter){
	rootObject->setProperty("filterTypeInput",QVariant(filter));
	rootObject->setProperty("numericFilterTypeLabelText",QVariant(vfilter_get_short_name(filter_type_get_display_text(filter_type_get_obj(this->ft,filter)))));
}
void maincontroller:: demoSequence(QVariantList seqList){
	if (demo_timer->isActive()){
		demo_src = false;
	}
	int i = 0;
	for(; i < seqList.count(); i++){
		QVariantMap row = seqList.at(i).toMap();
		demo_sequence[i][SOURCE_TYPE] = row["source"].toInt();
		demo_sequence[i][FILTER_TYPE] = row["filterType"].toInt();
		demo_sequence[i][FILTER_MODE] = row["filterMode"].toInt();
	}
	demoSequenceLength = i;
}
void maincontroller :: updateDemoTimer(int timerval){
	demoTimerInterval = timerval * SECONDS_TO_MILLISEC;
}

void maincontroller:: demoMode(bool start){
	if (!start) {
		demo_timer->stop();
		autoloop = 0;
		// set new state
		videoCtrl = VIDEO_CTRL_OFF;

		// stop video pipeline
		int err = vlib_pipeline_stop();
		errorPopup(err);
		demo_src = true;

		// setting actual filter and inputs
//		updateFilterOptions(config.mode);
//		updateinputOptions(config.src);

	}else{
		demo_src_loop_count = 0;
		demo_filter_loop_count = 0;
		GLOBAL = 0;
		demo_src = false;
		videoSrcLoop();//call immidiately to run the demo.
	}
};
void maincontroller :: updatecpu(QAbstractSeries *cpu1, QAbstractSeries *cpu2, QAbstractSeries *cpu3, QAbstractSeries *cpu4){

	double data[NCpuData];
	cpuStat1->statistic(data[Cpu1]);
	cpuStat2->statistic(data[Cpu2]);
	cpuStat3->statistic(data[Cpu3]);
	cpuStat4->statistic(data[Cpu4]);

	QString cpus1s;
	cpus1s.sprintf("%.2f%%",data[Cpu1]);
	QString cpus2s;
	cpus2s.sprintf("%.2f%%",data[Cpu2]);
	QString cpus3s;
	cpus3s.sprintf("%.2f%%",data[Cpu3]);
	QString cpus4s;
	cpus4s.sprintf("%.2f%%",data[Cpu4]);

	QString cpu1s;
	cpu1s.sprintf("CPU 1 (%.0f)",data[Cpu1]);
	QString cpu2s;
	cpu2s.sprintf("CPU 2 (%.0f)",data[Cpu2]);
	QString cpu3s;
	cpu3s.sprintf("CPU 3 (%.0f)",data[Cpu3]);
	QString cpu4s;
	cpu4s.sprintf("CPU 4 (%.0f)",data[Cpu4]);

	QXYSeries *cpu1Series = static_cast<QXYSeries *>(cpu1);
	QXYSeries *cpu2Series = static_cast<QXYSeries *>(cpu2);
	QXYSeries *cpu3Series = static_cast<QXYSeries *>(cpu3);
	QXYSeries *cpu4Series = static_cast<QXYSeries *>(cpu4);

	rootObject->setProperty("cp1Lbl",QVariant(cpus1s));
	rootObject->setProperty("cp2Lbl",QVariant(cpus2s));
	rootObject->setProperty("cp3Lbl",QVariant(cpus3s));
	rootObject->setProperty("cp4Lbl",QVariant(cpus4s));

	cpu1Series->setName(cpu1s);
	cpu2Series->setName(cpu2s);
	cpu3Series->setName(cpu3s);
	cpu4Series->setName(cpu4s);

	if(cpu1List.length()>60){
		cpu1List.removeFirst();
		cpu2List.removeFirst();
		cpu3List.removeFirst();
		cpu4List.removeFirst();
	}

	cpu1List.append(data[Cpu1]);
	cpu2List.append(data[Cpu2]);
	cpu3List.append(data[Cpu3]);
	cpu4List.append(data[Cpu4]);

	QVector<QPointF> cpu1points;
	QVector<QPointF> cpu2points;
	QVector<QPointF> cpu3points;
	QVector<QPointF> cpu4points;

	for(int p = 0; p < cpu1List.length(); p++){
		cpu1points.append(QPointF(p,cpu1List.at(p)));
		cpu2points.append(QPointF(p,cpu2List.at(p)));
		cpu3points.append(QPointF(p,cpu3List.at(p)));
		cpu4points.append(QPointF(p,cpu4List.at(p)));
	}
	cpu1Series->replace(cpu1points);
	cpu2Series->replace(cpu2points);
	cpu3Series->replace(cpu3points);
	cpu4Series->replace(cpu4points);
}
void maincontroller :: updateThroughput(QAbstractSeries *videoSrcAS, QAbstractSeries *acceleratorAS, QAbstractSeries *videoPortAS){
	double data[NMemData];
	QXYSeries *videoSrcSeries = static_cast<QXYSeries *>(videoSrcAS);
	QXYSeries *acceleratorSeries = static_cast<QXYSeries *>(acceleratorAS);
	QXYSeries *vpSeries = static_cast<QXYSeries *>(videoPortAS);

#if defined (PERFAPM_CLIENT)
	Perf_OA_Payload *openamp_payload = NULL;
	openamp_payload = perfoacl_get_rd_wr_cnt(MAX_PERF_SLOTS, PERF_SAMPLE_INTERVAL_COUNTER);
	data[videoSrc] = (float) ((openamp_payload->readcnt[6] + openamp_payload->readcnt[7]) * 8 / 1000000000.0);
	data[filter] = (float) ((openamp_payload->readcnt[8] + openamp_payload->readcnt[9]) * 8 / 1000000000.0);
	data[videoPort] = (float) ((openamp_payload->readcnt[4] + openamp_payload->readcnt[5]) * 8 / 1000000000.0);
#endif

	if(videoSrcList.length() > HISTORY_LENGTH){
		videoSrcList.removeFirst();
		filterList.removeFirst();
		videoPortList.removeFirst();
	}
	videoSrcList.append(data[videoSrc]);
	filterList.append(data[filter]);
	videoPortList.append(data[videoPort]);

	QString str1;
	str1.sprintf("Video Source (%2.0f)", data[videoSrc]);
	QString str2;
	str2.sprintf("Accelerator (%2.0f)", data[filter]);
	QString str3;
	str3.sprintf("Display (%2.0f)", data[videoPort]);

	videoSrcSeries->setName(str1);
	acceleratorSeries->setName(str2);
	vpSeries->setName(str3);

	QVector < QPointF > videoSrcpoints;
	QVector < QPointF > accelpoints;
	QVector < QPointF > vppoints;

	for(int p = 0; p < videoSrcList.length(); p++){
		videoSrcpoints.append(QPointF(p, videoSrcList.at(p)));
		accelpoints.append(QPointF(p, filterList.at(p)));
		vppoints.append(QPointF(p, videoPortList.at(p)));
	}
	videoSrcSeries->replace(videoSrcpoints);
	acceleratorSeries->replace(accelpoints);
	vpSeries->replace(vppoints);

	// setting value
	QString strTruVSPut;
	QString strTruFilterPut;
	QString strTruDPPut;
	strTruVSPut.sprintf("%2.1f Gbps", data[videoSrc]);
	strTruFilterPut.sprintf("%2.1f Gbps", data[filter]);
	strTruDPPut.sprintf("%2.1f Gbps", data[videoPort]);
	rootObject->setProperty("truPutVideoSrcLbl", strTruVSPut);
	rootObject->setProperty("truPutFilterLbl", strTruFilterPut);
	rootObject->setProperty("truPutDisplayPortLbl", strTruDPPut);
}
