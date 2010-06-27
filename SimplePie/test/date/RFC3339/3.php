<?php

class SimplePie_Date_Test_RFC3339_3 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = '1996-12-20T00:39:57Z';
	}
	
	function expected()
	{
		$this->expected = 851042397;
	}
}

?>