QT += core gui widgets quick qml charts

TARGET = video_qt2
TEMPLATE = app

SOURCES += \
	src/maincontroller.cpp \
	src/main.cpp \
	src/CPUStat.cpp \
	src/video_cfg.cpp

HEADERS += \
	include/maincontroller.h \
	include/CPUStat.h \
	include/video_cfg.h

MOC_DIR = moc

OBJECTS_DIR = obj

mkdirs.commands = $(MKDIR) $$MOC_DIR $$OBJECTS_DIR

QMAKE_EXTRA_TARGETS += mkdirs

INCLUDEPATH += \
	include \
	../video_lib/include \
	=/usr/include/glib-2.0 \
	=/usr/lib/glib-2.0/include \
	=/usr/include

QMAKE_LIBDIR_FLAGS += \
	-L../video_lib/Release \
	-L../video_lib/Debug \
	-L=/usr/lib \
	-L=/lib

LIBS += \
	-lopencv_core \
	-lopencv_imgproc \
	-lvideo \
	-lQt5PrintSupport \
	-lQt5OpenGL \
	-lQt5Svg \
	-lgobject-2.0 \
	-lglib-2.0 \
	-lpcre \
	-lffi \
	-lX11 \
	-lXfixes \
	-lXext \
	-lxcb \
	-lXau \
	-lXdmcp \
	-lXdamage \
	-lz \
	-lpng16 \
	-lrt \
	-ldl \
	-lssl \
	-lcrypto \
	-ldrm \
	-lv4l2subdev \
	-lmediactl

CONFIG += "release"

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3 --sysroot=${SYSROOT}
QMAKE_CXXFLAGS_RELEASE += -O3 --sysroot=${SYSROOT}
QMAKE_LFLAGS_RELEASE += --sysroot=${SYSROOT}
QMAKE_LFLAGS_RELEASE += -Wl,-rpath-link=${SYSROOT}/lib,-rpath-link=${SYSROOT}/usr/lib

RESOURCES += \
	resourcefile.qrc
