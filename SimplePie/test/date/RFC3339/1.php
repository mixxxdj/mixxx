<?php

class SimplePie_Date_Test_RFC3339_1 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = '1985-04-12T23:20:50.52Z';
	}
	
	function expected()
	{
		$this->expected = 482196051;
	}
}

?>