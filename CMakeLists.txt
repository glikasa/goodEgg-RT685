# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
#set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules/tas2563")
# list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules/tas2563")
#list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules/adpd188")
list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules/esp_hosted")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(goodEgg-RT685)

target_sources(app PRIVATE app/src/esp_hosted_test.c)

