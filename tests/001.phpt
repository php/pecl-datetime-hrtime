--TEST--
HRTime\StopWatch lasts not more that microtime()
--SKIPIF--
<?php if (!extension_loaded("hrtime")) print "skip"; ?>
--FILE--
<?php 

include 'helper.inc';

for ($j = 0; $j < 4; $j++) {
	$c = new HRTime\StopWatch; 

	$t0 = microtime(true);
	$c->start();
	for ($i = 0; $i < (1024*1024); $i++);
	$t1 = microtime(true);
	$c->stop();
	$elapsed0 = $t1 - $t0;

	$e0 = $elapsed0;
	$e1 = $c->getLastElapsedTime(HRTime\Unit::SECOND);
	echo "Microtime  s: ", $e0, "s", "\n";
	echo "Stopwatch  s: ", $e1, "s", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::SECOND));

	$e0 = s_to($elapsed0, HRTime\Unit::MILLISECOND);
	$e1 = $c->getLastElapsedTime(HRTime\Unit::MILLISECOND);
	echo "Microtime ms: ", $e0, "ms", "\n";
	echo "Stopwatch ms: ", $e1, "ms", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::MILLISECOND));

	$e0 = s_to($elapsed0, HRTime\Unit::MICROSECOND);
	$e1 = $c->getLastElapsedTime(HRTime\Unit::MICROSECOND);
	echo "Microtime us: ", $e0, "us", "\n";
	echo "Stopwatch us: ", $e1, "us", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::MICROSECOND));

	$e0 = s_to($elapsed0, HRTime\Unit::NANOSECOND);
	$e1 = $c->getLastElapsedTime(HRTime\Unit::NANOSECOND);
	echo "Microtime ns: ", $e0, "ns", "\n";
	echo "Stopwatch ns: ", $e1, "ns", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::NANOSECOND));

	echo "\n";
}
--EXPECTF--
Microtime  s: %fs
Stopwatch  s: %fs
bool(true)
Microtime ms: %fms
Stopwatch ms: %fms
bool(true)
Microtime us: %fus
Stopwatch us: %fus
bool(true)
Microtime ns: %fns
Stopwatch ns: %fns
bool(true)

Microtime  s: %fs
Stopwatch  s: %fs
bool(true)
Microtime ms: %fms
Stopwatch ms: %fms
bool(true)
Microtime us: %fus
Stopwatch us: %fus
bool(true)
Microtime ns: %fns
Stopwatch ns: %fns
bool(true)

Microtime  s: %fs
Stopwatch  s: %fs
bool(true)
Microtime ms: %fms
Stopwatch ms: %fms
bool(true)
Microtime us: %fus
Stopwatch us: %fus
bool(true)
Microtime ns: %fns
Stopwatch ns: %fns
bool(true)

Microtime  s: %fs
Stopwatch  s: %fs
bool(true)
Microtime ms: %fms
Stopwatch ms: %fms
bool(true)
Microtime us: %fus
Stopwatch us: %fus
bool(true)
Microtime ns: %fns
Stopwatch ns: %fns
bool(true)

