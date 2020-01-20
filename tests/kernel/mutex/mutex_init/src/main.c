/*
 * Copyright (c) 2012-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <tc_util.h>
#include <zephyr.h>
#include <ztest.h>
#include <sys/mutex.h>

#define STACKSIZE (512 + CONFIG_TEST_EXTRA_STACKSIZE)
#define TIMEOUT 1000

static void mutex_lock_unlock(struct sys_mutex *mutex)
{
	zassert_true(sys_mutex_lock(mutex, K_FOREVER) == 0,
		     "fail to lock K_FOREVER");
	sys_mutex_unlock(mutex);
	zassert_true(sys_mutex_lock(mutex, K_NO_WAIT) == 0,
		     "fail to lock K_NO_WAIT");
	sys_mutex_unlock(mutex);
	zassert_true(sys_mutex_lock(mutex, TIMEOUT) == 0,
		     "fail to lock TIMEOUT");
	sys_mutex_unlock(mutex);
}

/**
 * @brief Test mutex tests
 * @degroup kernel_mutext_tests Mutex Tests
 * @ingroup all_tests
 *
 * @{
 */

/**
 * @brief Test mutex initialization. A mutex can be defined and
 *        initialized at compile time and run time
 * @ingroup kernel_mutex_tests
 * @verify {@req{285}}
 */

ZTEST_BMEM SYS_MUTEX_DEFINE(compile_time_mutex);
void test_mutex_init_at_compile_time(void)
{
	TC_PRINT("Testing compile time mutex\n");
	mutex_lock_unlock(&compile_time_mutex);
	TC_PRINT("Testing compile time mutex successful\n");

}

void test_mutex_init_at_run_time(void)
{
	struct sys_mutex run_time_mutex;

	sys_mutex_init(&run_time_mutex);

	TC_PRINT("Testing run time mutex\n");
	mutex_lock_unlock(&run_time_mutex);
	TC_PRINT("Testing run time mutex successful\n");
}

/**
 * @brief Test mutex initialization. An application shall
 *        be able to define any number of mutexes
 * @ingroup kernel_mutex_tests
 * @verify {@req{287}}
 */

/* FIXME: there is no limit of mutex defined in kernel,
 * so number of mutexes an application can define is unlimited.
 */

/**
 * @}
 */

void test_main(void)
{

	ztest_test_suite(mutex_init,
			 ztest_1cpu_unit_test(test_mutex_init_at_compile_time),
			 ztest_1cpu_unit_test(test_mutex_init_at_run_time));

	ztest_run_test_suite(mutex_init);

}
