<?php

class SimplePie_Date_Test_RFC3339_2 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = '1996-12-19T16:39:57-08:00';
	}
	
	function expected()
	{
		$this->expected = 851042397;
	}
}

?>