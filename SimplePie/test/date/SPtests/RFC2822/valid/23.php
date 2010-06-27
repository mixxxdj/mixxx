<?php

class SimplePie_Date_Test_RFC2822_23 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 13:15:30 K';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>