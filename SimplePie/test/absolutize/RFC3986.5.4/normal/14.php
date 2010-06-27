<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_14 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g;x?y#s';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/g;x?y#s';
	}
}

?>