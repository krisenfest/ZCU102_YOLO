#-------------------------------------------------
#
# Project created by QtCreator 2011-11-29T15:51:32
#
#-------------------------------------------------

SDSOC_ACCEL = filter2d_optflow

INCLUDEPATH += \
	../$$SDSOC_ACCEL/src/src_of \
	../$$SDSOC_ACCEL/src/src_f2d

QMAKE_LIBDIR_FLAGS += \
	-L../$$SDSOC_ACCEL/Release \
	-L../$$SDSOC_ACCEL/Debug

LIBS += \
	-l$$SDSOC_ACCEL

DEFINES += \
	SAMPLE_FILTER2D \
	SAMPLE_OPTICAL_FLOW

include(video_qt2-common.pro)

