# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mutex_api)

if(NOT NEGATIVE_CASE)
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

else(NEGATIVE_CASE)
	FILE(GLOB negative_src src/negative/*.c)
	target_sources(app PRIVATE ${negative_src})
endif(NEGATIVE_CASE)
