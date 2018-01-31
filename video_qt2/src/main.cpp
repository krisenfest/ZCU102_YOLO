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
 * This file controls application entry and GUI initialization.
 */

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <video.h>
#include "maincontroller.h"
#include "video_cfg.h"

#include <QtGui/QGuiApplication>
#include <QQuickItem>
#include <QQuickView>
#include <QFileInfo>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QProcess>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSplashScreen>
#include <QThread>
#include <QQmlContext>
#include <QPixmap>
#include <QCursor>

#define MAX_MODES 3

class MyQuickView : public QQuickView
{
private:
	int maxX = 3840;
	int maxY = 2160;

public:
	QQuickItem *item;
	void setMaxX(int max){
		maxX = max;
	}
	int getMaxX(){
		return maxX;
	}
	void setMaxY(int max){
		maxY = max;
	}
	int getMaxY(){
		return maxY;
	}
	void mouseMoveEvent(QMouseEvent * e){
		QVariant returnedValue;
		QVariant msg = 0;
		QMetaObject::invokeMethod(item, "resetTimer",
				Q_RETURN_ARG(QVariant, returnedValue),
				Q_ARG(QVariant, msg));
		bool inVisibleArea = true;
		QPoint p;
		p = QCursor::pos();
		if(p.x() > maxX){
			inVisibleArea = false;
			p.setX(maxX);
		}
		if(p.y() > maxY){
			inVisibleArea = false;
			p.setY(maxY);
		}
		if(!inVisibleArea){
			QCursor::setPos(p);
		}
		QQuickView::mouseMoveEvent(e);
	}
	void mouseReleaseEvent(QMouseEvent *e){
		QVariant returnedValue;
		QVariant msg = 0;
		QMetaObject::invokeMethod(item, "resetTimer",
				Q_RETURN_ARG(QVariant, returnedValue),
				Q_ARG(QVariant, msg));
		QQuickView::mouseReleaseEvent(e);
	}
};

void signalhandler(int sig)
{
	QCoreApplication::exit(sig);
}

static void usage(const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("-d, --drm-module name   DRM module: 'xilinx' or 'xylon' (default: xylon)\n");
	printf("-h, --help              Show this help screen\n");
	printf("-p, --partial-reconfig  Enable partial reconfiguration of image filter\n");
	printf("-r, --resolution WxH    Width'x'Height (default: 3840x2160)\n");
}

static struct option opts[] = {
	{ "drm-module", required_argument, NULL, 'd' },
	{ "help", no_argument, NULL, 'h' },
	{ "partial-reconfig", no_argument, NULL, 'p' },
	{ "resolution", required_argument, NULL, 'r' },
	{ NULL, 0, NULL, 0 }
};

static void filter_init(struct filter_tbl *ft)
{
	UNUSED(ft);

#if defined (SAMPLE_FILTER2D)
	struct filter_s *fs = filter2d_create();
	if (filter_type_register(ft, fs)) {
		printf("Failed to register filter %s\n", filter_type_get_display_text(fs));
	}
#endif
#if defined (SAMPLE_OPTICAL_FLOW)
        struct filter_s *fof = optical_flow_create();
        if (filter_type_register(ft, fof)) {
		printf("Failed to register filter %s\n", filter_type_get_display_text(fof));
        }
#endif
#if defined (SAMPLE_SIMPLE_POSTERIZE)
        struct filter_s *fsp = simple_posterize_create();
        if (filter_type_register(ft, fsp)) {
		printf("Failed to register filter %s\n", filter_type_get_display_text(fsp));
        }
#endif
}


int main(int argc, char *argv[])
{
	int ret, i, c;
	int best_mode = 1;
	float resolutionFraction;
	const int width[MAX_MODES] = {3840, 1920, 1280};
	const int height[MAX_MODES] = {2160, 1080, 720};
	struct filter_tbl ft = {};
	struct vlib_config_data cfg;
	struct vlib_config config;

	filter_init(&ft);

	memset(&cfg, 0, sizeof(cfg));
	cfg.width_out = width[0];
	cfg.height_out = height[0];
	cfg.ft = &ft;

	/* Parse command line arguments */
	while ((c = getopt_long(argc, argv, "d:hpr:", opts, NULL)) != -1) {
		switch (c) {
		case 'd':
			sscanf(optarg, "%u", &cfg.dri_card_id);
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		case 'p':
			cfg.flags |= VLIB_CFG_FLAG_PR_ENABLE;
			break;
		case 'r':
			ret = sscanf(optarg, "%ux%u", &cfg.width_out, &cfg.height_out);
			if (ret != 2) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			best_mode = 0;
			break;
		default:
			printf("Invalid option -%c\n", c);
			printf("Run %s -h for help\n", argv[0]);
			return 1;
		}
	}

	/* Find and set best supported mode */
	for (i = 0; i < MAX_MODES; i++) {
		if (best_mode) {
			ret = vlib_drm_try_mode(cfg.dri_card_id, width[i], height[i]);
			if (ret == VLIB_SUCCESS) {
				cfg.width_out = width[i];
				cfg.height_out = height[i];
				break;
			}
		} else {
			if (cfg.width_out == width[i] && cfg.height_out == height[i])
				break;
		}
	}
	if (i == MAX_MODES) {
		printf("Only supported resolutions are: 720p, 1080p, and 2160p\n");
		return 1;
	} else {
		/* Set multiplier based on 1080p reference resolution height[1] */
		resolutionFraction = (float) cfg.height_out / height[1];
	}

	/* Set input resolution equal to output resolution */
	if (!cfg.width_in) {
		cfg.width_in = cfg.width_out;
		cfg.height_in = cfg.height_out;
	}

	/* Initialize video library */
	vlib_init(&cfg);

	/* Set start-up defaults */
	vlib_set_event_log(DISPLAY);

	/* Set startup config */
	config.type = 0;
	config.mode = FILTER_MODE_OFF;
	for (config.vsrc = 0; config.vsrc < vlib_video_src_cnt_get(); config.vsrc++) {
		const struct vlib_vdev *vsrc = vlib_video_src_get(config.vsrc);

		if (vlib_video_src_get_class(vsrc) == VLIB_VCLASS_TPG) {
			break;
		}
	}

	/* use first available video source if no TPG present */
	if (config.vsrc == vlib_video_src_cnt_get()) {
		config.vsrc = 0;
	}

	QApplication app(argc, argv);

	struct sigaction sa;
	sa.sa_handler = signalhandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);

	MyQuickView viewer;
	viewer.rootContext()->setContextProperty("resoluteFrac",resolutionFraction);
	viewer.rootContext()->setContextProperty("imageResolution",2160);
	viewer.rootContext()->setContextProperty("imageResolutionHeight", cfg.height_out);
	viewer.rootContext()->setContextProperty("imageResolutionWidth", cfg.width_out);
	viewer.rootContext()->setContextProperty("displayDeviceType", (cfg.dri_card_id == 0) ? "DP" : "HDMI");
	/* Video Sources */
	QVariantList sourceList;
	for (unsigned int i = 0; i < vlib_video_src_cnt_get(); i++) {
		const struct vlib_vdev *vd = vlib_video_src_get(i);
		video_src vclass = vlib_video_src_get_class(vd);

		/* we assume a single instance per vsrc class */
		if (vsrc_get_vd(vclass)) {
			continue;
		}

		QVariantMap map;
		map.insert("displayName", QString::fromUtf8(vlib_video_src_get_display_text(vd)));
		map.insert("sourceIndex", vclass);
		map.insert("hasPanel", vsrc_get_has_panel(vclass));
		map.insert("shortName", vsrc_get_short_name(vclass));
		map.insert("settingsPanelName", vsrc_get_short_name(vclass));

		/* make TPG appear first in sources list to make it default */
		if (vclass == VLIB_VCLASS_TPG) {
			sourceList.prepend(map);
		} else {
			sourceList.append(map);
		}

		vsrc_set_vd(vclass, vd);
	}
	if(sourceList.count() == 0){
		// exiting GUI as no video sources available.
		exit(0);
	}

	viewer.rootContext()->setContextProperty("videoSourcesCount",sourceList.count());
	viewer.rootContext()->setContextProperty("videoSourceList",QVariant::fromValue(sourceList));
	/*Filter Types*/
	QVariantList filterTypeList;
	for(unsigned int i = 0; i < cfg.ft->size; i++){
		QVariantMap map;
		map.insert("displayName", QString::fromUtf8(filter_type_get_display_text(filter_type_get_obj(&ft,i))));
		map.insert("sourceIndex", i);
		map.insert("hasPanel", vfilter_get_has_panel(filter_type_get_display_text(filter_type_get_obj(&ft,i))));
		map.insert("shortName", vfilter_get_short_name(filter_type_get_display_text(filter_type_get_obj(&ft,i))));
		map.insert("settingsPanelName", vfilter_get_short_name(filter_type_get_display_text(filter_type_get_obj(&ft,i))));
		filterTypeList.append(map);
	}
	viewer.rootContext()->setContextProperty("filterTypeSourcesCount",filterTypeList.count());
	viewer.rootContext()->setContextProperty("filterTypeSourceList",QVariant::fromValue(filterTypeList));

	/* TPG PATTERNS */
	QStringList qstr;
	for(int i = 1; i < TPG_BG_PATTERN_CNT; i++)
		qstr.append(QString::fromUtf8(tpg_get_pattern_menu_name(i)));
	viewer.rootContext()->setContextProperty("tpgPatterns",qstr);

	QStringList qstrPreset;
	bool filter2DEnabled = false;
#if defined (SAMPLE_FILTER2D)
	/* Filter2d presets */
	filter2DEnabled = true;
	for(int i = 0; i < FILTER2D_PRESET_CNT; i++)
		qstrPreset.append(QString::fromUtf8(filter2d_get_preset_name((filter2d_preset)i)));
	/* Custom for preset*/
	qstrPreset.append("Custom");
#endif
	viewer.rootContext()->setContextProperty("presetList",qstrPreset);
	viewer.rootContext()->setContextProperty("filter2DEnabled",filter2DEnabled);
	maincontroller myClass;
	viewer.rootContext()->setContextProperty("dataSource", &myClass);
	viewer.setSource(QUrl("qrc:qml/main.qml"));

	QSurfaceFormat surfaceFormat;
	surfaceFormat.setAlphaBufferSize(8);
	viewer.setFormat(surfaceFormat);
	viewer.setClearBeforeRendering(true);
	viewer.setColor(QColor(Qt::transparent));

	QQuickItem *item = viewer.rootObject();
	viewer.item = item;
	viewer.setMaxX(cfg.width_out);
	viewer.setMaxY(cfg.height_out);
	item->setWidth(cfg.width_out);
	item->setHeight(cfg.height_out);
	item->setProperty("resolution", cfg.height_out);
	item->update();

	myClass.init(item, config, &ft);

	QObject :: connect( item, SIGNAL(playvideo(int)),&myClass,SLOT(setVideo(int)));
	QObject :: connect( item, SIGNAL(setFilter(int)),&myClass,SLOT(setFilterMode(int)));
	QObject :: connect( item, SIGNAL(setFilterType(int)),&myClass,SLOT(setFilterType(int)));
	QObject :: connect( item, SIGNAL(setInputType(int)),&myClass,SLOT(setInput(int)));
//	QObject :: connect( item, SIGNAL(setInvert(int)),&myClass,SLOT(setInvert(int)));
	QObject :: connect( item, SIGNAL(setTPGPattern(int)),&myClass,SLOT(setTPGPattern(int)));
	QObject :: connect( item, SIGNAL(setpreset(int)),&myClass,SLOT(setPreset(int)));
//	QObject :: connect( item, SIGNAL(setSensitivity(int,int)),&myClass,SLOT(setSensitivity(int,int)));
	QObject :: connect( item, SIGNAL(demoMode(bool)),&myClass,SLOT(demoMode(bool)));
	QObject :: connect( item, SIGNAL(setFgOverlay(int)),&myClass,SLOT(setFgOverlay(int)));
	QObject :: connect( item, SIGNAL(setBoxSize(int)),&myClass,SLOT(setBoxSize(int)));
	QObject :: connect( item, SIGNAL(setBoxColor(int)),&myClass,SLOT(setBoxColor(int)));
	QObject :: connect( item, SIGNAL(setBoxSpeed(int)),&myClass,SLOT(setBoxSpeed(int)));
	QObject :: connect( item, SIGNAL(setCrossRows(int)),&myClass,SLOT(setCrossRows(int)));
	QObject :: connect( item, SIGNAL(setCrossColumns(int)),&myClass,SLOT(setCrossColumns(int)));
	QObject :: connect( item, SIGNAL(setZoneH(int)),&myClass,SLOT(setZoneH(int)));
	QObject :: connect( item, SIGNAL(setZoneHDelta(int)),&myClass,SLOT(setZoneHDelta(int)));
	QObject :: connect( item, SIGNAL(setZoneV(int)),&myClass,SLOT(setZoneV(int)));
	QObject :: connect( item, SIGNAL(setZoneVDelta(int)),&myClass,SLOT(setZoneVDelta(int)));
	QObject :: connect( item, SIGNAL(filterCoeff(int,int,int,int,int,int,int,int,int)),&myClass,SLOT(filterCoeff(int,int,int,int,int,int,int,int,int)));
	QObject :: connect((QObject*)viewer.engine(), SIGNAL(quit()), qApp, SLOT(quit()) );
	QObject :: connect( qApp, SIGNAL(aboutToQuit()),&myClass,SLOT(closeall()));

	viewer.show();

	return app.exec();
}
