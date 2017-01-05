/*
 * Copyright (c) 2014 Anatol Belski
 * All rights reserved.
 *
 * Author: Anatol Belski <ab@php.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#ifndef PHP_HRTIME_H
#define PHP_HRTIME_H

#define PHP_HRTIME_VERSION "0.6.0"

extern zend_module_entry hrtime_module_entry;
#define phpext_hrtime_ptr &hrtime_module_entry

#ifdef PHP_WIN32
#	define PHP_HRTIME_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_HRTIME_API __attribute__ ((visibility("default")))
#else
#	define PHP_HRTIME_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "timer.h"

PHP_MINIT_FUNCTION(hrtime);
PHP_MSHUTDOWN_FUNCTION(hrtime);
PHP_RINIT_FUNCTION(hrtime);
PHP_RSHUTDOWN_FUNCTION(hrtime);
PHP_MINFO_FUNCTION(hrtime);

PHP_METHOD(PerformanceCounter, start);
PHP_METHOD(PerformanceCounter, stop);
PHP_METHOD(PerformanceCounter, getElapsedTicks);
PHP_METHOD(PerformanceCounter, getLastElapsedTicks);
PHP_METHOD(PerformanceCounter, getFrequency);
PHP_METHOD(PerformanceCounter, isRunning);
PHP_METHOD(PerformanceCounter, getTicks);
PHP_METHOD(PerformanceCounter, getTicksSince);

PHP_METHOD(StopWatch, getElapsedTime);
PHP_METHOD(StopWatch, getLastElapsedTime);


/*ZEND_BEGIN_MODULE_GLOBALS(hrtime)
ZEND_END_MODULE_GLOBALS(hrtime)*/

#ifdef ZTS
#define HRTIME_G(v) TSRMG(hrtime_globals_id, zend_hrtime_globals *, v)
#else
#define HRTIME_G(v) (hrtime_globals.v)
#endif

#if PHP_MAJOR_VERSION < 7
typedef long zend_long;
#endif

#if PHP_MAJOR_VERSION >= 7
struct ze_performance_counter_obj {
	tick_t start;
	tick_t elapsed;
	tick_t elapsed_ref;
	zend_bool is_running;
	zend_object zo;
};

static zend_always_inline struct ze_performance_counter_obj *
php_performance_counter_fetch_obj(zend_object *obj)
{/*{{{*/
	return (struct ze_performance_counter_obj *)((char *)obj - XtOffsetOf(struct ze_performance_counter_obj, zo));
}/*}}}*/
#else
struct ze_performance_counter_obj {
	zend_object zo;
	tick_t start;
	tick_t elapsed;
	tick_t elapsed_ref;
	zend_bool is_running;
};
#endif

#if PHP_VERSION_ID >= 50399
zend_object_handlers default_hrtime_handlers;
#endif

#endif	/* PHP_HRTIME_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
