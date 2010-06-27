<?php

class SimplePie_Date_Test_RFC2822_12 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 05:15:30 PST';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>