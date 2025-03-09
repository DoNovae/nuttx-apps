############################################################################
# apps/doais/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

MODULE_DIR = $(APPDIR)/doais

# -D CONFIG_LV_PORT_USE_TOUCHPAD=y
CFLAGS += -Iinc -Iincm -Iinc -Iinc/gps 
CXXFLAGS += -Iinc -Iincm -Iinc -Iinc/gps

#-I$(TOPDIR)/fs/littlefs/littlefs
#CSRCS    += $(wildcard src/*.c)
CSRCS    += src/img/img_target.c src/img/ui_target.c src/img/img_vessels.c src/img/ui_vessels.c src/img/img_settings.c src/img/ui_settings.c
CSRCS    += src/img/lv_font_doais_speed.c src/img/lv_font_doais_status.c src/img/lv_font_doais_12.c src/img/lv_font_doais_24.c
#CSRCS    += src/img/ui_wind.c src/img/ui_autopilot.c


CXXSRCS  += src/configuration_store.cxx
CXXSRCS  += src/doais_serial.cxx src/ais_utils.cxx src/doais_db_update.cxx src/doais_gps.cxx
CXXSRCS  += src/ais_messages.cxx src/ais_monitoring.cxx src/ais_utils.cxx src/utilities.cxx src/ais_test.cxx src/rx_packet.cxx
CXXSRCS  += src/gps/gmath.cxx src/tx_packet.cxx src/gps/nmea.cxx src/gps/unicore.cxx src/gps/rtcm.cxx src/gps/crc.cxx src/gps/gps_helper.cxx

MAINSRC += src/doais_main.cxx
PROGNAME  += "doais"

MAINSRC  += src/doais_mng.cxx
PROGNAME += "doais_mng"

#PRIORITY  = $(CONFIG_DOAIS_PRIORITY)
#STACKSIZE = $(CONFIG_DOAIS_STACKSIZE)
#MODULE    = $(CONFIG_DOAIS)

PRIORITY  = 100
STACKSIZE = 20480*16
MODULE    = $(CONFIG_DOAIS)

include $(APPDIR)/Application.mk
