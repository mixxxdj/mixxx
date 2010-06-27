<?php

class SimplePie_Date_Test_RFC2822_10 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 06:15:30 MST';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>