<?php

class who_knows_a_title_from_a_hole_in_the_ground_text_cdata extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">
<id>http://atomtests.philringnalda.com/tests/item/title/text-cdata.atom</id>
<title>Atom item title text in CDATA</title>
<updated>2005-12-18T00:13:00Z</updated>
<author>
  <name>Phil Ringnalda</name>
  <uri>http://weblog.philringnalda.com/</uri>
</author>
<link rel="self" href="http://atomtests.philringnalda.com/tests/item/title/text-cdata.atom"/>
<entry>
  
<id>http://atomtests.philringnalda.com/tests/item/title/text-cdata.atom/1</id>
  <title type="text"><![CDATA[<title>]]></title>
  <updated>2005-12-18T00:13:00Z</updated>
  <summary>An item with a type="text" title consisting of a less-than 
character, the word \'title\' and a greater-than character, where 
the less-than is escaped by being in a CDATA section.</summary>
  <link href="http://atomtests.philringnalda.com/alt/title-title.html"/>
  <category term="item title"/>
</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = '&lt;title&gt;';
	}
}

?>
