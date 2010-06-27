<?php

class SimplePie_Date_Test_W3CDTF_1 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = '1994-11-05T08:15:30-05:00';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>