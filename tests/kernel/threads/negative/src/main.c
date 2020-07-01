/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <kernel_structs.h>
#include <kernel.h>
#include <kernel_internal.h>
#include <string.h>

#define STACK_SIZE (512 + CONFIG_TEST_EXTRA_STACKSIZE)
static K_THREAD_STACK_DEFINE(user_stack, STACK_SIZE);
static K_THREAD_STACK_DEFINE(noentry_stack, STACK_SIZE);
static K_THREAD_STACK_DEFINE(zero_stack, 0);
static struct k_thread noentry_thread;
static struct k_thread user_thread;
static struct k_thread zero_thread;

void k_sys_fatal_error_handler(unsigned int reason,
			       const z_arch_esf_t *esf)
{
	ARG_UNUSED(esf);
	ARG_UNUSED(reason);
}

void assert_post_action(const char *file, unsigned int line)
{
	ARG_UNUSED(file);
	ARG_UNUSED(line);

#ifdef CONFIG_USERSPACE
	if (_is_user_context()) {
		k_oops();
	}
#endif
	if (k_is_in_isr())
		ztest_test_pass();
	else
		k_panic();
}

/** @brief Iterate over all the threads in the system
 *
 * @details Iterates over all the threads int the system and call
 *         the user_cb function, if user_cb is NULL, system will panic.
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_foreach()
 *
 * @return \b should be PASS
 */
void test_k_thread_foreach_no_cb(void)
{
#if defined(CONFIG_THREAD_MONITOR)
	k_thread_foreach(NULL, "test");
	zassert_unreachable("NULL user_cb should be a fatal error");
#else
	ztest_test_skip();
#endif
}

/** @brief Iterate over all the threads in the system
 *
 * @details Iterates over all the threads int the system and call
 *         the user_cb function, if user_cb is NULL, system will panic.
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_foreach_unlocked()
 *
 * @return \b should be PASS
 */
void test_k_thread_foreach_unlocked_no_cb(void)
{
#if defined(CONFIG_THREAD_MONITOR)
	k_thread_foreach_unlocked(NULL, "test");
	zassert_unreachable("NULL user_cb should be a fatal error");
#else
	ztest_test_skip();
#endif
}

static void thread_entry(void *p1, void *p2, void *p3)
{
	/* Nothing to do */
}

static void isr_entry(void *arg)
{
	k_thread_create(&user_thread, user_stack, STACK_SIZE,
			thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);
}

void test_k_thread_create_in_isr(void)
{
	irq_offload(isr_entry, NULL);
}
/** @brief Create a thread without a k_thread structure
 *
 * @details k_thread_create need a pointer to uninitialized k_thread
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_null_thread(void)
{
	k_thread_create(NULL, user_stack, STACK_SIZE,
			thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	zassert_unreachable("NULL thread pointer should be a fatal error");
}

/** @brief Create a thread with a invalid stack
 *
 * @details k_thread_create need a pointer to a valid stack space
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_null_stack(void)
{

	k_thread_create(&user_thread, NULL, STACK_SIZE,
			thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	zassert_unreachable("NULL stack pointer should be a fatal error");
}

/** @brief Create a thread with a invalid stack size
 *
 * @details k_thread_create need a valid stack size
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_zero_stack_size(void)
{

	k_thread_create(&zero_thread, zero_stack, 0,
			thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	zassert_unreachable("Zero stack size should be a fatal error");
}

/** @brief Create a thread with a invalid stack size
 *
 * @details k_thread_create need a valid stack size
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_overflow_stack_size(void)
{

	k_thread_create(&zero_thread, zero_stack, (size_t)-1,
			thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	zassert_unreachable("Zero stack size should be a fatal error");
}

/** @brief Create a thread without entry
 *
 * @details k_thread_create should check the entry parameter but it doesn't,
 *          so the thread is created successfully and system will crash when
 *          that thread is scheduled to run.
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_no_entry(void)
{
	k_thread_create(&noentry_thread, noentry_stack, STACK_SIZE,
		        NULL, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);
	/* schedule new thread to run */
	k_sleep(K_MSEC(100));
	zassert_unreachable("Null entry should be a fatal error");
}

/** @brief Create a thread with a invalid priority
 *
 * @details The new thread's priority should not to be higher than the creator
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_priority(void)
{
	k_thread_create(&user_thread, user_stack, STACK_SIZE,
		        thread_entry, NULL, NULL, NULL,
			k_thread_priority_get(k_current_get()) - 1,
			K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	zassert_unreachable("Priority of user thread too high");
}

/** @brief Create a thread with a invalid option
 *
 * @details The user thread should be created with K_USER flag
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_no_user_flag(void)
{
	k_thread_create(&user_thread, user_stack, STACK_SIZE,
				      thread_entry, NULL, NULL, NULL,
				      K_PRIO_PREEMPT(1), 0, K_NO_WAIT);

	zassert_unreachable("Invalid user option");
}

/** @brief Create a thread with a invalid k_thread structure
 *
 * @details k_thread_create need a pointer to uninitialized k_thread,
 *          create a thread with an initialized k_thread will cause crash.
 *
 * @ingroup kernel.thread.negative
 *
 * @see k_thread_create()
 *
 * @return Should be PASS
 */
void test_k_thread_create_user_inited(void)
{
	k_thread_create(&user_thread, user_stack, STACK_SIZE,
		        thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	k_thread_create(&user_thread, user_stack, STACK_SIZE,
		        thread_entry, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), K_USER | K_INHERIT_PERMS, K_NO_WAIT);

	zassert_unreachable("A thread should never be inited before create");
}

/**
 * @brief Negative cases for thread test
 * @defgroup kernel.thread.negative
 * @ingroup all_tests
 */
void test_main(void)
{
	k_thread_access_grant(k_current_get(),
			      &user_thread, user_stack,
			      &noentry_thread, noentry_stack,
			      &zero_thread, zero_stack);

	ztest_test_suite(threads_negative,
			 ztest_unit_test(test_k_thread_create_in_isr),
			 ztest_unit_test(test_k_thread_foreach_no_cb),
			 ztest_unit_test(test_k_thread_foreach_unlocked_no_cb),
			 ztest_user_unit_test(test_k_thread_create_user_null_thread),
			 ztest_user_unit_test(test_k_thread_create_user_null_stack),
			 ztest_user_unit_test(test_k_thread_create_user_no_entry),
			 ztest_user_unit_test(test_k_thread_create_user_priority),
			 ztest_user_unit_test(test_k_thread_create_user_no_user_flag),
			 ztest_user_unit_test(test_k_thread_create_user_inited),
			 ztest_user_unit_test(test_k_thread_create_user_overflow_stack_size),
			 ztest_user_unit_test(test_k_thread_create_user_zero_stack_size)
			 );
	ztest_run_test_suite(threads_negative);
}
