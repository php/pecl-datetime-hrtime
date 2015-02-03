--TEST--
HRTime\StopWatch long test microtime comparsion
--SKIPIF--
<?php if (!extension_loaded("hrtime")) print "skip"; ?>
<?php if (getenv("SKIP_SLOW_TESTS")) die("skip slow test"); ?>
<?php if (strtoupper(substr(PHP_OS, 0, 3)) == 'WIN') die("skip Test is not valid for Windows"); ?>
--INI--
presicion=48
--FILE--
<?php 
$c = new HRTime\StopWatch;
$i = 0;
$fails = 0;
$max_fail = 1;

while($i < 300000 && $fails <= $max_fail) {
	/*printf("Run %d\r", $i++);*/
	$i++;
	$c->start();
	$t = microtime(true);
	usleep(1000);
	$c->stop();
	$t = (microtime(true)-$t)*1000000;
	$e = $c->getLastElapsedTime(HRTime\Unit::MICROSECOND);

	$p = (int)(abs($t-$e)*100/$t);
	if ($p>3) {
		/*printf("\nEcart 1 : %.10f", $e);
		printf("\nEcart 2 : %.10f (%d)\n", $t, $p);*/
		$fails++;
	}
}

var_dump($fails <= $max_fail);

--EXPECTF--
bool(true)
