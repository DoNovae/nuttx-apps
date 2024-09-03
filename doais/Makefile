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

CFLAGS += -Iinc0 -Iincm -Iinc -Iinc/gps
CXXFLAGS += -Iinc0 -Iincm -Iinc -Iinc/gps

#CSRCS    += $(wildcard src/*.c)
CSRCS    += src0/ui_clock.c src0/img_hand.c
CXXSRCS  += srcm/configuration_store.cxx

#CXXSRCS    += src/ais_monitoring.cxx src/ais_messages.cxx

MAINSRC += src0/doais_main.cxx
PROGNAME  += "doais"

MAINSRC  += src0/doais_mng.c
PROGNAME += doais_mng

#PRIORITY  = $(CONFIG_DOAIS_PRIORITY)
#STACKSIZE = $(CONFIG_DOAIS_STACKSIZE)
#MODULE    = $(CONFIG_DOAIS)

PRIORITY  = 100
STACKSIZE = 20480
MODULE    = $(CONFIG_DOAIS)

include $(APPDIR)/Application.mk
