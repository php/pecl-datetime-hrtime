# High resolution timer for PHP #

This PHP extension brings the possibility to use a high resolution timer from the user land PHP scripts. The best available APIs are used on differend platforms to achieve this goal. 

## Synopsis ##

```php

	$c = new HRTime\StopWatch;

	$c->start();
	/* do some interesting stuff*/
	$c->stop();
	$elapsed0 = $c->getLastElapsedTime(HRTime\Unit::NANOSECOND);

	/* do something else */

	$c->start();
	/* do some interesting stuff again */
	$c->stop();
	$elapsed1 = $c->getLastElapsedTime(HRTime\Unit::NANOSECOND);

	$elapsed_total = $c->getElapsedTime(HRTime\Unit::NANOSECOND);
```