<?php

class SimplePie_Date_Test_RFC2822_20 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 13:15:30 G';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>