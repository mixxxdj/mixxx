<?php

class SimplePie_Date_Test_Bug_259_Test_0 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = '1994-11-05T08:15:30-0500';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>