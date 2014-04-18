--TEST--
HRTime\StopWatch elapsed tick count vs. elapsed time
--SKIPIF--
<?php if (!extension_loaded("hrtime")) print "skip"; ?>
--FILE--
<?php 

include 'helper.inc';

$c = new HRTime\StopWatch; 
$freq = $c->getFrequency();

for ($j = 0; $j < 4; $j++) {

	$c->start();
	for ($i = 0; $i < 1024; $i++);
	$c->stop();
	$elapsed0 = $c->getLastElapsedTicks();

	$e0 = $elapsed0/$freq;
	$e1 = $c->getLastElapsedTime(HRTime\Unit::SECOND);
	echo "calc from ticks  s: ", $e0, "s", "\n";
	echo "time method  s: ", $e1, "s", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::SECOND));

	$e0 = ($elapsed0/$freq)*1000.0;
	$e1 = $c->getLastElapsedTime(HRTime\Unit::MILLISECOND);
	echo "calc from ticks  ms: ", $e0, "ms", "\n";
	echo "time method  ms: ", $e1, "ms", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::MILLISECOND));

	$e0 = ($elapsed0/$freq)*1000000.0;
	$e1 = $c->getLastElapsedTime(HRTime\Unit::MICROSECOND);
	echo "calc from ticks  us: ", $e0, "us", "\n";
	echo "time method  us: ", $e1, "us", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::MICROSECOND));

	$e0 = ($elapsed0/$freq)*1000000000.0;
	$e1 = $c->getLastElapsedTime(HRTime\Unit::NANOSECOND);
	echo "calc from ticks  ns: ", $e0, "ns", "\n";
	echo "time method  ns: ", $e1, "ns", "\n";
	var_dump(is_uncertainty_acceptable($e0, $e1, HRTime\Unit::NANOSECOND));

	echo "\n";
}
--EXPECTF--
calc from ticks  s: %fs
time method  s: %fs
bool(true)
calc from ticks  ms: %fms
time method  ms: %fms
bool(true)
calc from ticks  us: %fus
time method  us: %fus
bool(true)
calc from ticks  ns: %fns
time method  ns: %fns
bool(true)

calc from ticks  s: %fs
time method  s: %fs
bool(true)
calc from ticks  ms: %fms
time method  ms: %fms
bool(true)
calc from ticks  us: %fus
time method  us: %fus
bool(true)
calc from ticks  ns: %fns
time method  ns: %fns
bool(true)

calc from ticks  s: %fs
time method  s: %fs
bool(true)
calc from ticks  ms: %fms
time method  ms: %fms
bool(true)
calc from ticks  us: %fus
time method  us: %fus
bool(true)
calc from ticks  ns: %fns
time method  ns: %fns
bool(true)

calc from ticks  s: %fs
time method  s: %fs
bool(true)
calc from ticks  ms: %fms
time method  ms: %fms
bool(true)
calc from ticks  us: %fus
time method  us: %fus
bool(true)
calc from ticks  ns: %fns
time method  ns: %fns
bool(true)

