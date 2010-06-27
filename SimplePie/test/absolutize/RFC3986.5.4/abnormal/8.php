<?php

class SimplePie_Absolutize_Test_RFC3986_Abnormal_8 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = '..g';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/..g';
	}
}

?>