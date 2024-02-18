############################################################################
# apps/examples/lvgltest/Makefile
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

PROGNAME = lvglcal


CSRCS += lv_tc.c lv_tc_screen.c esp_nvs_tc.c img/lv_tc_indicator_img.c

MAINSRC = lvglcal.c

CFLAGS += ${shell $(DEFINE) "$(CC)" LV_LVGL_H_INCLUDE_SIMPLE} -Wno-format
CXXFLAGS += ${shell $(DEFINE) "$(CC)" LV_LVGL_H_INCLUDE_SIMPLE} -Wno-format

include $(APPDIR)/Application.mk
