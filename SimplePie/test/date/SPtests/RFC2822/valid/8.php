<?php

class SimplePie_Date_Test_RFC2822_8 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 07:15:30 CST';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>