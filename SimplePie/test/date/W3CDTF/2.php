<?php

class SimplePie_Date_Test_W3CDTF_2 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = '1994-11-05T13:15:30Z';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>