<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_9 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = '#s';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/d;p?q#s';
	}
}

?>