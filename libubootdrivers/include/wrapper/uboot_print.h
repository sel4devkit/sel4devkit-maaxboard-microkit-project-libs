/* 
 * Copyright 2022, Capgemini Engineering
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 *
 */

/* Wrappers around U-Boot print string routines */

#include <stdio.h>
#include <log.h>


/* Map the U-Boot logging routines onto printf */

#define UBOOT_LOG_VERBOSE 9
#define UBOOT_LOG_DEBUG   8
#define UBOOT_LOG_INFO    6
#define UBOOT_LOG_WARN    4
#define UBOOT_LOG_ERROR   3
#define UBOOT_LOG_FATAL   0
		
#define UBOOT_LOG_ALLOW(lvl)    ((lvl) <= CONFIG_LOGLEVEL)
#define UBOOT_LOG_ALLOW_VERBOSE UBOOT_LOG_ALLOW(UBOOT_LOG_VERBOSE)
#define UBOOT_LOG_ALLOW_DEBUG   UBOOT_LOG_ALLOW(UBOOT_LOG_DEBUG)
#define UBOOT_LOG_ALLOW_INFO    UBOOT_LOG_ALLOW(UBOOT_LOG_INFO)
#define UBOOT_LOG_ALLOW_WARN    UBOOT_LOG_ALLOW(UBOOT_LOG_WARN)
#define UBOOT_LOG_ALLOW_ERROR   UBOOT_LOG_ALLOW(UBOOT_LOG_ERROR)
#define UBOOT_LOG_ALLOW_FATAL   UBOOT_LOG_ALLOW(UBOOT_LOG_FATAL)

#define UBOOT_LOG_PRINTF(...) ({ \
    const char* base_name = strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__; \
    printf("%s@%s:%u ", __func__, base_name, __LINE__); \
    printf(__VA_ARGS__); \
    printf("\n"); \
})


#define DUMMY_UNUSED ({})

#if UBOOT_LOG_ALLOW_VERBOSE
	#define UBOOT_LOGV(...) UBOOT_LOG_PRINTF(__VA_ARGS__)
#else
	#define UBOOT_LOGV(...) DUMMY_UNUSED
#endif

#if UBOOT_LOG_ALLOW_DEBUG
	#define UBOOT_LOGD(...) UBOOT_LOG_PRINTF(__VA_ARGS__)
#else
	#define UBOOT_LOGD(...) DUMMY_UNUSED
#endif

#if UBOOT_LOG_ALLOW_INFO
	#define UBOOT_LOGI(...) UBOOT_LOG_PRINTF(__VA_ARGS__)
#else
	#define UBOOT_LOGI(...) DUMMY_UNUSED
#endif

#if UBOOT_LOG_ALLOW_WARN
	#define UBOOT_LOGW(...) UBOOT_LOG_PRINTF(__VA_ARGS__)
#else
	#define UBOOT_LOGW(...) DUMMY_UNUSED
#endif

#if UBOOT_LOG_ALLOW_ERROR
	#define UBOOT_LOGE(...) UBOOT_LOG_PRINTF(__VA_ARGS__)
#else
	#define UBOOT_LOGE(...) DUMMY_UNUSED
#endif
		
#if UBOOT_LOG_ALLOW_FATAL
	#define UBOOT_LOGF(...) UBOOT_LOG_PRINTF(__VA_ARGS__)
#else
	#define UBOOT_LOGF(...) DUMMY_UNUSED
#endif

/* Define U-Boot debug_xxx macros */
#define debug(...)          UBOOT_LOGD(__VA_ARGS__)
#define debug_cond(A, ...)  UBOOT_LOGD(__VA_ARGS__)

/* Define U-Boot log_xxx macros */
#define log(_cat, _level, ...) ({ 							\
	if      (_level <= LOGL_EMERG)   UBOOT_LOGF(__VA_ARGS__);	\
	else if (_level <= LOGL_ERR)     UBOOT_LOGE(__VA_ARGS__);	\
	else if (_level <= LOGL_WARNING) UBOOT_LOGW(__VA_ARGS__);	\
	else if (_level <= LOGL_INFO)    UBOOT_LOGI(__VA_ARGS__);	\
	else if (_level <= LOGL_DEBUG)   UBOOT_LOGD(__VA_ARGS__);	\
	else							 UBOOT_LOGV(__VA_ARGS__);	\
	})

#define log_emer(_fmt...)		log(0, LOGL_EMERG, ##_fmt)
#define log_alert(_fmt...)		log(0, LOGL_ALERT, ##_fmt)
#define log_crit(_fmt...)		log(0, LOGL_CRIT, ##_fmt)
#define log_err(_fmt...)		log(0, LOGL_ERR, ##_fmt)
#define log_warning(_fmt...)	log(0, LOGL_WARNING, ##_fmt)
#define log_notice(_fmt...)		log(0, LOGL_NOTICE, ##_fmt)
#define log_info(_fmt...)		log(0, LOGL_INFO, ##_fmt)
#define log_debug(_fmt...)		log(0, LOGL_DEBUG, ##_fmt)
#define log_content(_fmt...)	log(0, LOGL_DEBUG_CONTENT, ##_fmt)
#define log_io(_fmt...)			log(0, LOGL_DEBUG_IO, ##_fmt)
#define log_cont(_fmt...)		log(0, LOGL_CONT, ##_fmt)

#define log_ret(_ret) ({ \
	int __ret = (_ret); \
	if (__ret < 0) \
		UBOOT_LOGE("returning err=%d\n", __ret); \
	__ret; \
	})

#define log_msg_ret(_msg, _ret) ({ \
	int __ret = (_ret); \
	if (__ret < 0) \
		UBOOT_LOGE("%s: returning err=%d\n", _msg, __ret); \
	__ret; \
	})

/* Define U-Boot printk macros */
#define no_printk(...)
#define printk(...)         	printf(__VA_ARGS__)
#define printk_once(fmt, ...)	printk(fmt, ##__VA_ARGS__)
#define pr_emerg(fmt, ...)		log_emerg(fmt, ##__VA_ARGS__)
#define pr_alert(fmt, ...)		log_alert(fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...)		log_crit(fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)		log_err(fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)		log_warning(fmt, ##__VA_ARGS__)
#define pr_notice(fmt, ...)		log_notice(fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)		log_info(fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...)		log_debug(fmt, ##__VA_ARGS__)
#define pr_devel(fmt, ...)		log_debug(fmt, ##__VA_ARGS__)
#define pr_cont(fmt, ...)		log_cont(fmt, ##__VA_ARGS__)

/* Define non spl logging routines */
#define warn_non_spl(fmt, ...)	log_warning(fmt, ##__VA_ARGS__)

#define assert_noisy(_x) \
	({ bool _val = (_x); \
	if (!_val) \
		panic("assert_noisy triggered"); \
	_val; \
	})
