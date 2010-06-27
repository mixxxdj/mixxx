<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_10 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g#s';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/g#s';
	}
}

?>