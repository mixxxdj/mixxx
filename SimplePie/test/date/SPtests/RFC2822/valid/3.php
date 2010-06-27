<?php

class SimplePie_Date_Test_RFC2822_3 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 5 Nov 94 13:15:30 GMT';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>