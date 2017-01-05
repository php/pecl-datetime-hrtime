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
	PHP_ME(PerformanceCounter, getFrequency, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PerformanceCounter, getTicks, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PerformanceCounter, getTicksSince, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};/*}}}*/

const zend_function_entry StopWatch_methods[] = {/*{{{*/
	PHP_ME(StopWatch, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(StopWatch, stop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(StopWatch, getElapsedTicks, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(StopWatch, getLastElapsedTicks, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(StopWatch, isRunning, NULL, ZEND_ACC_PUBLIC)
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

#if PHP_MAJOR_VERSION >= 7
void
php_stop_watch_obj_destroy(zend_object *obj)
{/*{{{*/
	struct ze_stop_watch_obj *zswo = php_stop_watch_fetch_obj(obj);

	zend_object_std_dtor(&zswo->zo); /* ??? */
}/*}}}*/
#else
void
php_stop_watch_obj_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_stop_watch_obj *zswo = (struct ze_stop_watch_obj *)obj;

	zend_object_std_dtor(&zswo->zo TSRMLS_CC);

	/*hrtime_counter_destroy(&zswo->htc);*/

	efree(zswo);
}/*}}}*/
#endif

#if PHP_MAJOR_VERSION >= 7
zend_object *
php_stop_watch_obj_init(zend_class_entry *ze)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;

	zswo = (struct ze_stop_watch_obj *)ecalloc(1, sizeof(struct ze_stop_watch_obj));
	
	zend_object_std_init(&zswo->zo, ze);
	zswo->zo.handlers = &default_hrtime_handlers;

	zswo->is_running = 0;
	zswo->start = 0;
	zswo->elapsed = 0;
	zswo->elapsed_ref = 0;


	return &zswo->zo;
}/*}}}*/
#else
zend_object_value
php_stop_watch_obj_init(zend_class_entry *ze TSRMLS_DC)
{/*{{{*/
	zend_object_value ret;
	struct ze_stop_watch_obj *zswo;
#if PHP_VERSION_ID < 50399
	zval *tmp;
#endif

	zswo = (struct ze_stop_watch_obj *)emalloc(sizeof(struct ze_stop_watch_obj));
	memset(&zswo->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zswo->zo, ze TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(zswo->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));
#else
	object_properties_init(&zswo->zo, ze);
#endif

	zswo->is_running = 0;
	zswo->start = 0;
	zswo->elapsed = 0;
	zswo->elapsed_ref = 0;

	ret.handle = zend_objects_store_put(zswo, NULL,
										(zend_objects_free_object_storage_t) php_stop_watch_obj_destroy,
										NULL TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	ret.handlers = zend_get_std_object_handlers();
	ret.handlers->clone_obj = NULL;
#else
	ret.handlers = &default_hrtime_handlers;
#endif

	return ret;
}/*}}}*/
#endif /* php_stop_watch_obj_init */

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
#if PHP_MAJOR_VERSION >= 7
	default_hrtime_handlers.offset = XtOffsetOf(struct ze_stop_watch_obj, zo);
	default_hrtime_handlers.free_obj = php_stop_watch_obj_destroy;
#endif
#endif

	/* Init internal classes */
	INIT_CLASS_ENTRY(ce, "HRTime\\PerformanceCounter", PerformanceCounter_methods);
	ce.create_object = php_stop_watch_obj_init;
	PerformanceCounter_ce = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "HRTime\\StopWatch", StopWatch_methods);
	ce.create_object = php_stop_watch_obj_init;
#if PHP_MAJOR_VERSION >= 7
	StopWatch_ce = zend_register_internal_class_ex(&ce, PerformanceCounter_ce);
#else
	StopWatch_ce = zend_register_internal_class_ex(&ce, PerformanceCounter_ce, "HRTime\\PerformanceCounter" TSRMLS_CC);
#endif

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

PHP_METHOD(StopWatch, start)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (zswo->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The counter is already running");
		return;
	}

	zswo->start = timer_current();
	zswo->is_running = 1;
}/*}}}*/

PHP_METHOD(StopWatch, stop)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!zswo->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The counter is not running");
		return;
	}

	zswo->elapsed_ref = timer_elapsed_ticks(zswo->start);
	zswo->elapsed += zswo->elapsed_ref;
	zswo->is_running = 0;
}/*}}}*/

PHP_METHOD(StopWatch, getElapsedTicks)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (zswo->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Counter is still running");
	}

	RETURN_LONG(zswo->elapsed);
}/*}}}*/

PHP_METHOD(StopWatch, getLastElapsedTicks)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (zswo->is_running) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Counter is still running");
		RETURN_LONG(0);
	}

	RETURN_LONG(zswo->elapsed_ref);
}/*}}}*/

PHP_METHOD(PerformanceCounter, getFrequency)
{/*{{{*/
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(timer_ticks_per_second());
}/*}}}*/

PHP_METHOD(StopWatch, isRunning)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	RETURN_BOOL(zswo->is_running);
}/*}}}*/

PHP_METHOD(PerformanceCounter, getTicks)
{/*{{{*/
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_LONG(timer_current());
}/*}}}*/

PHP_METHOD(PerformanceCounter, getTicksSince)
{/*{{{*/
	zend_long base;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &base) == FAILURE) {
		return;
	}

	RETURN_LONG(timer_elapsed_ticks(base));
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
	struct ze_stop_watch_obj *zswo;
	zend_long unit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &unit) == FAILURE) {
		RETURN_NULL();
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	RET_TIME_BY_UNIT(zswo->elapsed, unit);
}/*}}}*/

PHP_METHOD(StopWatch, getLastElapsedTime)
{/*{{{*/
	struct ze_stop_watch_obj *zswo;
	zend_long unit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &unit) == FAILURE) {
		RETURN_NULL();
	}

#if PHP_MAJOR_VERSION >= 7
	zswo = php_stop_watch_fetch_obj(Z_OBJ_P(getThis()));
#else
	zswo = (struct ze_stop_watch_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	RET_TIME_BY_UNIT(zswo->elapsed_ref, unit);
}/*}}}*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
