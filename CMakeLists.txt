# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules/tas2563")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(goodEgg-RT685)

target_sources(app PRIVATE app/src/tas2563_driver_test.c)

