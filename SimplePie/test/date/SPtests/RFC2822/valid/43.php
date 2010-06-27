<?php

class SimplePie_Date_Test_RFC2822_43 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri(day), 05 Nov(ember) 94 13:15:30 GMT';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>