<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_11 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g?y#s';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/g?y#s';
	}
}

?>