<?php

class SimplePie_Absolutize_Test_Bug_1091_Test_0_1 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://example.com';
		$this->data['relative'] = '//example.net';
	}
	
	function expected()
	{
		$this->expected = 'http://example.net';
	}
}

?>