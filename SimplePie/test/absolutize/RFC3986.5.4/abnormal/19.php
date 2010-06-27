<?php

class SimplePie_Absolutize_Test_RFC3986_Abnormal_19 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'http:g';
	}
	
	function expected()
	{
		$this->expected = 'http:g';
	}
}

?>