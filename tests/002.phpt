--TEST--
HRTime\StopWatch partial elapsed ticks sum == total elapsed ticks
--SKIPIF--
<?php if (!extension_loaded("hrtime")) print "skip"; ?>
--FILE--
<?php 

for ($j = 0; $j < 4; $j++) {
	$c = new HRTime\StopWatch; 

	$c->start();
	for ($i = 0; $i < 1024; $i++);
	$c->stop();
	$elapsed0 = $c->getLastElapsedTicks();

	usleep(1000);

	$c->start();
	for ($i = 0; $i < 1024; $i++);
	$c->stop();
	$elapsed1 = $c->getLastElapsedTicks();

	usleep(1000);

	$c->start();
	for ($i = 0; $i < 1024; $i++);
	$c->stop();
	$elapsed2 = $c->getLastElapsedTicks();

	$elapsed_total = $c->getElapsedTicks();

	echo "elapsed0=$elapsed0\n";
	echo "elapsed1=$elapsed1\n";
	echo "calculated elapsed total=" . ($elapsed0 + $elapsed1 + $elapsed2) . "\n";
	echo "elapsed total=$elapsed_total\n";
	var_dump(($elapsed0 + $elapsed1 + $elapsed2) == $elapsed_total);

	echo "\n";
}
--EXPECTF--
elapsed0=%d
elapsed1=%d
calculated elapsed total=%d
elapsed total=%d
bool(true)

elapsed0=%d
elapsed1=%d
calculated elapsed total=%d
elapsed total=%d
bool(true)

elapsed0=%d
elapsed1=%d
calculated elapsed total=%d
elapsed total=%d
bool(true)

elapsed0=%d
elapsed1=%d
calculated elapsed total=%d
elapsed total=%d
bool(true)

