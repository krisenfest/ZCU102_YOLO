#-------------------------------------------------
#
# Project created by QtCreator 2011-11-29T15:51:32
#
#-------------------------------------------------

SDSOC_ACCEL = filter2d

INCLUDEPATH += \
	../$$SDSOC_ACCEL/src

QMAKE_LIBDIR_FLAGS += \
	-L../$$SDSOC_ACCEL/Release \
	-L../$$SDSOC_ACCEL/Debug

LIBS += \
	-l$$SDSOC_ACCEL

DEFINES += \
	SAMPLE_FILTER2D

include(video_qt2-common.pro)
