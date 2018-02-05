################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/drm_helper.c \
../src/filter.c \
../src/gpio_utils.c \
../src/imx274_cfg.c \
../src/log_events.c \
../src/m2m_sw_pipeline.c \
../src/mediactl_helper.c \
../src/ov13850_cfg.c \
../src/s2m_pipeline.c \
../src/v4l2_helper.c \
../src/vcap_csi.c \
../src/vcap_file.c \
../src/vcap_hdmi.c \
../src/vcap_tpg.c \
../src/vcap_uvc.c \
../src/vcap_vivid.c \
../src/video.c \
../src/video_src.c 

OBJS += \
./src/drm_helper.o \
./src/filter.o \
./src/gpio_utils.o \
./src/imx274_cfg.o \
./src/log_events.o \
./src/m2m_sw_pipeline.o \
./src/mediactl_helper.o \
./src/ov13850_cfg.o \
./src/s2m_pipeline.o \
./src/v4l2_helper.o \
./src/vcap_csi.o \
./src/vcap_file.o \
./src/vcap_hdmi.o \
./src/vcap_tpg.o \
./src/vcap_uvc.o \
./src/vcap_vivid.o \
./src/video.o \
./src/video_src.o 

C_DEPS += \
./src/drm_helper.d \
./src/filter.d \
./src/gpio_utils.d \
./src/imx274_cfg.d \
./src/log_events.d \
./src/m2m_sw_pipeline.d \
./src/mediactl_helper.d \
./src/ov13850_cfg.d \
./src/s2m_pipeline.d \
./src/v4l2_helper.d \
./src/vcap_csi.d \
./src/vcap_file.d \
./src/vcap_hdmi.d \
./src/vcap_tpg.d \
./src/vcap_uvc.d \
./src/vcap_vivid.d \
./src/video.d \
./src/video_src.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM A53 Linux gcc compiler'
	aarch64-linux-gnu-gcc -DPLATFORM_ZCU102 -DWITH_SDSOC -DENABLE_VCAP_UVC -DENABLE_VCAP_VIVID -Wall -O0 -g3 -I=/usr/include -I=/usr/include/libdrm -I=/usr/include/glib-2.0 -I=/usr/lib/glib-2.0/include -I"/home/lucas/Project/ZCU102/source/TRD_HOME/rdf0429-zcu102-es2-base-trd-2017-1/apu/video_app/ZCU102_YOLO/video_lib/include" -I"/home/lucas/Project/ZCU102/source/TRD_HOME/rdf0429-zcu102-es2-base-trd-2017-1/apu/video_app/ZCU102_YOLO/video_lib/src/include" -I"/home/lucas/Project/ZCU102/source/TRD_HOME/rdf0429-zcu102-es2-base-trd-2017-1/apu/video_app/ZCU102_YOLO/filter2d/src" -c -fmessage-length=0 -MT"$@" --sysroot=/home/lucas/Project/ZCU102/source/TRD_HOME/rdf0429-zcu102-es2-base-trd-2017-1/apu/petalinux_bsp/build/tmp/sysroots/plnx_aarch64 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


