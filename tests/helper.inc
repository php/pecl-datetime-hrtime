<?php

function s_to($secs, $unit)
{
	$ret = .0;

	switch ($unit) {
	case HRTime\Unit::SECOND:
		$ret = $secs;
		break;
	case HRTime\Unit::MILLISECOND:
		$ret = $secs*1000.0;
		break;
	case HRTime\Unit::MICROSECOND:
		$ret = $secs*1000000.0;
		break;
	case HRTime\Unit::NANOSECOND:
		$ret = $secs*1000000000.0;
		break;
	}

	return $ret;
}

function uncertainty_abs($a, $b)
{
	return $a - $b;
}

function uncertainty_rel($a, $b)
{
	return uncertainty_abs($a, $b)/$b;
}

function is_uncertainty_acceptable($a, $b, $unit)
{
	$ret = false;

	$rel = uncertainty_rel($a, $b);
	$treshold = .05;

	switch ($unit) {
	case HRTime\Unit::SECOND:
		$ret = abs($rel) < $treshold;
		break;
	case HRTime\Unit::MILLISECOND:
		$ret = abs($rel) < $treshold;
		break;
	case HRTime\Unit::MICROSECOND:
		$ret = abs($rel) < $treshold;
		break;
	case HRTime\Unit::NANOSECOND:
		$ret = abs($rel) < $treshold;
		break;
	}

	if (!$ret) {
		printf("a=$a b=$b relative uncertainty %d%%\n", (int)($rel*100.0));
	}

	return $ret;
}
