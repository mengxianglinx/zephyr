/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <ztest.h>
#include <wait_q.h>

#define TIMEOUT 500
#define STACK_SIZE (512 + CONFIG_TEST_EXTRA_STACKSIZE)

/**TESTPOINT: init via K_MUTEX_DEFINE*/
K_MUTEX_DEFINE(kmutex);
static struct k_mutex mutex;

static K_THREAD_STACK_DEFINE(tstack, STACK_SIZE);
static struct k_thread tdata;

static void tmutex_test_lock(struct k_mutex *pmutex,
			     void (*entry_fn)(void *, void *, void *))
{
	k_mutex_init(pmutex);
	k_thread_create(&tdata, tstack, STACK_SIZE,
			entry_fn, pmutex, NULL, NULL,
			K_PRIO_PREEMPT(0),
			K_USER | K_INHERIT_PERMS, K_NO_WAIT);
	k_mutex_lock(pmutex, K_FOREVER);
	TC_PRINT("access resource from main thread\n");

	/* wait for spawn thread to take action */
	k_msleep(TIMEOUT);
}

void k_sys_fatal_error_handler(unsigned int reason,
				      const z_arch_esf_t *esf)
{
	ARG_UNUSED(esf);
	ARG_UNUSED(reason);
}

/**@brief Null pointer fault in k_mutex_init()
 *
 * @ingroup kenrel_mutex_negative
 *
 * @details
 *   - k_mutex_init() should handle null pointer, return an error
 *
 * @return \b should be PASS
 */
void test_mutex_init_fault(void)
{
	int ret = k_mutex_init(NULL);
	zassert_false(ret == 0, "Null pointer should be checked");
}

static void tThread_entry_lock_forever_modify(void *p1, void *p2, void *p3)
{
	/* should not get lock,
	 * modify mutex structure directly to get lock
	 * succeed on several platforms.
	 */

	((struct k_mutex *)p1)->lock_count = 0;
	zassert_false(k_mutex_lock((struct k_mutex *)p1, K_FOREVER) == 0,
		      "access locked resource from spawn thread");
}

static void tThread_entry_lock_forever_reinit(void *p1, void *p2, void *p3)
{
	/* should not get lock,
	 * reinitialize the mutex to get lock
	 * succeed on every platform
	 */
	k_mutex_init((struct k_mutex *)p1);
	zassert_false(k_mutex_lock((struct k_mutex *)p1, K_FOREVER) == 0,
		      "access locked resource from spawn thread");
}

/**@brief init an initialized mutex
 *
 * @ingroup kernel_mutex_negative
 *
 * @details
 *  - Mutex module should make sure that an initialized mutex not to be
 *    initialized again
 *
 * @return \b should be PASS
 */
void test_mutex_reent_init_fault(void)
{
	/*test k_mutex_init mutex*/
	k_mutex_init(&mutex);
	tmutex_test_lock(&mutex, tThread_entry_lock_forever_reinit);
	k_thread_abort(&tdata);
}

/**@brief modify mutex directly
 *
 * @ingroup kernel_mutex_negative
 *
 * @details
 *  - User should access mutex through interfaces of mutex module. Access
 *    mutex structure directly add a way for user to manipulate mutex value,
 *    hance increase the risk to crash lock and unlock mechanism.
 *
 * @return \b should be PASS
 */
void test_mutex_reent_modify_fault(void)
{
	/*test K_MUTEX_DEFINE mutex*/
	tmutex_test_lock(&kmutex, tThread_entry_lock_forever_modify);
	k_thread_abort(&tdata);
}

/*test case main entry*/
void test_main(void)
{
	k_thread_access_grant(k_current_get(), &tdata, &tstack, &kmutex,
			      &mutex);

	ztest_test_suite(mutex_api,
			 ztest_1cpu_user_unit_test(test_mutex_reent_init_fault),
			 ztest_1cpu_user_unit_test(test_mutex_reent_modify_fault),
			 ztest_1cpu_user_unit_test(test_mutex_init_fault),
			 ztest_1cpu_unit_test(test_mutex_reent_modify_fault),
			 ztest_1cpu_unit_test(test_mutex_init_fault)
			 );
	ztest_run_test_suite(mutex_api);
}
