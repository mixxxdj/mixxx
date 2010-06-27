<?php

class SimplePie_Absolutize_Test_Bug_274_Test_2 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/';
		$this->data['relative'] = '/b';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b';
	}
}

?>