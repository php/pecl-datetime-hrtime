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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hrtime.h"

/*ZEND_DECLARE_MODULE_GLOBALS(hrtime);*/

/* True global resources - no need for thread safety here */
static int le_hrtime;

zend_class_entry *PerformanceCounter_ce;
zend_class_entry *StopWatch_ce;
zend_class_entry *Unit_ce;

/* {{{ hrtime_functions[]
 *
 * Every user visible function must have an entry in hrtime_functions[].
 */
const zend_function_entry hrtime_functions[] = {
	PHP_FE_END
};
/* }}} */

const zend_function_entry PerformanceCounter_methods[] = {/*{{{*/
	PHP_ME(PerformanceCounter, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(PerformanceCounter, stop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(PerformanceCounter, getElapsedTicks, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(PerformanceCounter, getLastElapsedTicks, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(PerformanceCounter, getFrequency, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(PerformanceCounter, isRunning, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};/*}}}*/

const zend_function_entry StopWatch_methods[] = {/*{{{*/
	PHP_ME(StopWatch, getElapsedTime, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(StopWatch, getLastElapsedTime, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};/*}}}*/

/* {{{ hrtime_module_entry
 */
zend_module_entry hrtime_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"hrtime",
	hrtime_functions,
	PHP_MINIT(hrtime),
	PHP_MSHUTDOWN(hrtime),
	PHP_RINIT(hrtime),
	PHP_RSHUTDOWN(hrtime),
	PHP_MINFO(hrtime),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_HRTIME_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_HRTIME
ZEND_GET_MODULE(hrtime)
#endif

void
php_performance_counter_obj_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_performance_counter_obj *zvco = (struct ze_performance_counter_obj *)obj;

	zend_object_std_dtor(&zvco->zo TSRMLS_CC);

	/*hrtime_counter_destroy(&zvco->htc);*/

	efree(zvco);
}/*}}}*/

zend_object_value
php_performance_counter_obj_init(zend_class_entry *ze TSRMLS_DC)
{/*{{{*/
	zend_object_value ret;
	struct ze_performance_counter_obj *zvco;
#if PHP_VERSION_ID < 50399
	zval *tmp;
#endif

	zvco = (struct ze_performance_counter_obj *)emalloc(sizeof(struct ze_performance_counter_obj));
	memset(&zvco->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zvco->zo, ze TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(zvco->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));
#else
	object_properties_init(&zvco->zo, ze);
#endif

	zvco->is_running = 0;
	zvco->start = 0;
	zvco->elapsed = 0;
	zvco->elapsed_ref = 0;

	ret.handle = zend_objects_store_put(zvco, NULL,
										(zend_objects_free_object_storage_t) php_performance_counter_obj_destroy,
										NULL TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	ret.handlers = zend_get_std_object_handlers();
	ret.handlers->clone_obj = NULL;
#else
	ret.handlers = &default_hrtime_handlers;
#endif

	return ret;
}/*}}}*/

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hrtime.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_hrtime_globals, hrtime_globals)
    STD_PHP_INI_ENTRY("hrtime.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_hrtime_globals, hrtime_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_hrtime_counter_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_hrtime_init_globals(zend_hrtime_globals *hrtime_globals)
{
	hrtime_globals->global_value = 0;
	hrtime_globals->global_string = NULL;
}
*/
/* }}} */

#define HRTIME_SECOND 0
#define HRTIME_MILLISECOND 1
#define HRTIME_MICROSECOND 2
#define HRTIME_NANOSECOND 3

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(hrtime)
{
	zend_class_entry ce;

	if (timer_lib_initialize()) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to initialize internal timer");
		return FAILURE;
	}

#if PHP_VERSION_ID >= 50399
	memcpy(&default_hrtime_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	default_hrtime_handlers.clone_obj = NULL;
#endif

	/* Init internal classes */
	INIT_CLASS_ENTRY(ce, "HRTime\\PerformanceCounter", PerformanceCounter_methods);
	ce.create_object = php_performance_counter_obj_init;
	PerformanceCounter_ce = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "HRTime\\StopWatch", StopWatch_methods);
	ce.create_object = php_performance_counter_obj_init;
	StopWatch_ce = zend_register_internal_class_ex(&ce, PerformanceCounter_ce, "HRTime\\PerformanceCounter" TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "HRTime\\Unit", NULL);
	Unit_ce = zend_register_internal_class(&ce TSRMLS_CC);
	zend_declare_class_constant_long(Unit_ce, "SECOND", sizeof("SECOND")-1, HRTIME_SECOND TSRMLS_CC);
	zend_declare_class_constant_long(Unit_ce, "MILLISECOND", sizeof("MILLISECOND")-1, HRTIME_MILLISECOND TSRMLS_CC);
	zend_declare_class_constant_long(Unit_ce, "MICROSECOND", sizeof("MICROSECOND")-1, HRTIME_MICROSECOND TSRMLS_CC);
	zend_declare_class_constant_long(Unit_ce, "NANOSECOND", sizeof("NANOSECOND")-1, HRTIME_NANOSECOND TSRMLS_CC);

	/* 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(hrtime)
{
	timer_lib_shutdown();

	/*
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(hrtime)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(hrtime)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(hrtime)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "hrtime support", "enabled");
	php_info_print_table_header(2, "Version", PHP_HRTIME_VERSION);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

PHP_METHOD(PerformanceCounter, start)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zvco->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The counter is already running");
		return;
	}

	zvco->start = timer_current();
	zvco->is_running = 1;
}/*}}}*/

PHP_METHOD(PerformanceCounter, stop)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (!zvco->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The counter is not running");
		return;
	}

	zvco->elapsed_ref = timer_elapsed_ticks(zvco->start);
	zvco->elapsed += zvco->elapsed_ref;
	zvco->is_running = 0;
}/*}}}*/

PHP_METHOD(PerformanceCounter, getElapsedTicks)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zvco->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Counter is still running");
	}

	RETURN_LONG(zvco->elapsed);
}/*}}}*/

PHP_METHOD(PerformanceCounter, getLastElapsedTicks)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zvco->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Counter is still running");
		RETURN_LONG(0);
	}

	RETURN_LONG(zvco->elapsed_ref);
}/*}}}*/

PHP_METHOD(PerformanceCounter, getFrequency)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RETURN_LONG(timer_ticks_per_second());
}/*}}}*/

PHP_METHOD(PerformanceCounter, isRunning)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RETURN_BOOL(zvco->is_running);
}/*}}}*/

#define RET_TIME_BY_UNIT(t, unit) do { \
	double factor; \
	switch (unit) { \
		default: \
		case HRTIME_SECOND: \
			factor = 1.0; \
		break; \
		case HRTIME_MILLISECOND: \
			factor = 1000.0; \
		break; \
		case HRTIME_MICROSECOND: \
			factor = 1000000.0; \
		break; \
		case HRTIME_NANOSECOND: \
			factor = 1000000000.0; \
		break; \
	} \
	RETURN_DOUBLE(((double)t/(double)timer_ticks_per_second())*factor); \
	/*RETURN_DOUBLE((double)timer_elapsed(t)/factor);*/ \
} while (0);

PHP_METHOD(StopWatch, getElapsedTime)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;
	long unit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &unit) == FAILURE) {
		RETURN_NULL();
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RET_TIME_BY_UNIT(zvco->elapsed, unit);
}/*}}}*/

PHP_METHOD(StopWatch, getLastElapsedTime)
{/*{{{*/
	struct ze_performance_counter_obj *zvco;
	long unit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &unit) == FAILURE) {
		RETURN_NULL();
	}

	zvco = (struct ze_performance_counter_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RET_TIME_BY_UNIT(zvco->elapsed_ref, unit);
}/*}}}*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
