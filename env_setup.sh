#! /bin/sh
#
# env_setup.sh
# Copyright (C) 2018 lucas <lucas@lucas>
#
# Distributed under terms of the MIT license.
#


sh ~/Project/ZCU102/source/bsp/petalinux_2017-1/settings.sh
cd ~/Project/ZCU102/source/TRD_HOME/rdf0429-zcu102-es2-base-trd-2017-1/apu/video_app/ZCU102_YOLO/video_qt2
source qmake_set_env.sh
qmake video_qt2-dm10.pro -r -spec linux-oe-g++
cd ~/Project/ZCU102/source/TRD_HOME/rdf0429-zcu102-es2-base-trd-2017-1/apu/video_app/ZCU102_YOLO
source ~/Project/Software/SDSoc_2017-1/SDx/2017.1/settings64.sh
sdx --workspace .
