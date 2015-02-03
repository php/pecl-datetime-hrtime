--TEST--
HRTime\StopWatch start right after stop with nanosecond resolution > 0
--SKIPIF--
<?php if (!extension_loaded("hrtime")) print "skip"; ?>
--INI--
presicion=48
--FILE--
<?php 

for ($j = 0; $j < 4; $j++) {
	$c = new HRTime\StopWatch; 

	$c->start();
	$c->stop();

	$e1 = $c->getLastElapsedTime(HRTime\Unit::NANOSECOND);
	var_dump($e1, $e1 >= .0);

	echo "\n";

	/* Need this because the accuracy won't be as good as 1ns even on the best machines.
	Well, at least now it's sience fiction.*/
	usleep(1);
}
--EXPECTF--
float(%f)
bool(true)

float(%f)
bool(true)

float(%f)
bool(true)

float(%f)
bool(true)

