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

ZTEST_BMEM SYS_MUTEX_DEFINE(test_mutex);

K_THREAD_STACK_DEFINE(thread_stack_area, STACKSIZE);
struct k_thread thread_data;
static int thread_ret = 0xFF;

/**
 *
 * @brief thread_waiter - thread that participates in recursive locking tests
 *
 * @return  N/A
 */

void thread_waiter(void)
{
	int ret;

	/* Wait for test_mutex to be released */
	/* yes, wait forever */

	ret = sys_mutex_lock(&test_mutex, K_FOREVER);
	if (ret != 0) {
		thread_ret = TC_FAIL;
		TC_ERROR("Failed to get the test_mutex\n");
		return;
	}

	/* keep the next waiter waitting for a while */

	thread_ret = TC_PASS;
	k_sleep(K_MSEC(500));
	sys_mutex_unlock(&test_mutex);

}

/**
 * @brief Test mutex tests
 * @degroup kernel_mutext_tests Mutex Tests
 * @ingroup all_tests
 *
 * @{
 */

/**
 * @brief Test recursive mutex. The kernel shall support recursive mutexes,
 *        A lock of a mutex already locked will succeed, and waiters will
 *        be unblocked only when the number of locks reaches zero.
 * @ingroup kernel_mutex_tests
 * @verify {@req{283}}
 */
void test_mutex_recursive(void)
{
	int ret;
	int thread_flags = 0;


	TC_PRINT("Testing recursive locking\n");

	ret = sys_mutex_lock(&test_mutex, K_NO_WAIT);
	zassert_equal(ret, 0, "Failed to lock test_mutex");

	ret = sys_mutex_lock(&test_mutex, K_NO_WAIT);
	zassert_equal(ret, 0, "Failed to recursively lock test_mutex");

	/* Start waiter thread */
	k_thread_create(&thread_data, thread_stack_area, STACKSIZE,
			(k_thread_entry_t)thread_waiter, NULL, NULL, NULL,
			K_PRIO_PREEMPT(12), thread_flags, K_NO_WAIT);

	zassert_equal(thread_ret, 0xFF,"waiter thread should block on the recursively locked mutex");
	sys_mutex_unlock(&test_mutex);

	zassert_equal(thread_ret, 0xFF,"waiter thread should block on the locked mutex");
	sys_mutex_unlock(&test_mutex);

	k_sleep(K_MSEC(1));     /* Give thread_waiter a chance to get the mutex */
	zassert_equal(thread_ret, TC_PASS, "waiter thread can't take the mutex");

	ret = sys_mutex_lock(&test_mutex, K_NO_WAIT);
	zassert_equal(ret, -EBUSY, "Unexpectedly got lock on test_mutex");

	ret = sys_mutex_lock(&test_mutex, K_SECONDS(1));
	zassert_equal(ret, 0, "Failed to re-obtain lock on test_mutex");

	sys_mutex_unlock(&test_mutex);

	TC_PRINT("Recursive locking tests successful\n");

}

/**
 * @}
 */

void test_main(void)
{

	ztest_test_suite(mutex_recursive,
			 ztest_1cpu_unit_test(test_mutex_recursive));

	ztest_run_test_suite(mutex_recursive);

}
