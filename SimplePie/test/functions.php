<?php

require_once 'unit_test/unit_test2.php';

class SimplePie_Unit_Test2_Group extends Unit_Test2_Group
{
	function pre()
	{
		ob_start();
	}
	
	function post()
	{
		$output = ob_get_contents();
		
		if ($output !== false)
		{
			ob_end_clean();
		}
		
		$passed_percentage = floor($this->passes() / $this->total() * 100);
		$failed_percentage = ceil($this->fails() / $this->total() * 100);
		
		echo '<h2 class=' . (($passed_percentage == 100) ? 'pass' : 'fail') . '>' . htmlspecialchars($this->name(), ENT_COMPAT, 'UTF-8') . ': ' . $passed_percentage . "% passed!</h2>\n";
		echo "<p>\n$output\n</p>\n";
		echo '<p>We ran ' . $this->total() . ' tests in ' . round($this->time(), 3) . ' seconds of which ' . $this->passes() . ' passed, and ' . $this->fails() . ' failed.</p>';
		flush();
	}
}

class SimplePie_Unit_Test2 extends Unit_Test2
{
	function SimplePie_Unit_Test2()
	{
		parent::Unit_Test2();
		if (strpos($this->name, 'SimplePie') === 0)
		{
			$this->name = trim(substr_replace($this->name, '', 0, 9));
		}
	}
	
	function output($title, $class, $content)
	{
		printf("<span title='%s' class='%s'>%s</span>\n", $title, $class, $content);
	}
	
	function pass()
	{
		$this->output(htmlspecialchars($this->name(), ENT_COMPAT, 'UTF-8'), 'pass', '&#x2714;');
		parent::pass();
	}
	
	function fail()
	{
		$this->output(htmlspecialchars($this->name(), ENT_COMPAT, 'UTF-8'), 'fail', '&#x2718;');
		parent::fail();
	}
	
	function result()
	{
		if ($this->result === $this->expected)
		{
			$this->pass();
		}
		else
		{
			$this->fail();
		}
	}
}

class SimplePie_Feed_Test extends SimplePie_Unit_Test2
{
	function feed()
	{
		$feed = new SimplePie();
		$feed->set_raw_data($this->data);
		$feed->enable_cache(false);
		$feed->init();
		return $feed;
	}
}

class SimplePie_Feed_Author_Test extends SimplePie_Feed_Test
{
	function author()
	{
		$feed = $this->feed();
		if ($author = $item->get_author())
		{
			return $author;
		}
		else
		{
			return false;
		}
	}
}

class SimplePie_Feed_Category_Test extends SimplePie_Feed_Test
{
	function category()
	{
		$feed = $this->feed();
		if ($category = $feed->get_category())
		{
			return $category;
		}
		else
		{
			return false;
		}
	}
}

class SimplePie_First_Item_Test extends SimplePie_Feed_Test
{
	function first_item()
	{
		$feed = $this->feed();
		if ($item = $feed->get_item(0))
		{
			return $item;
		}
		else
		{
			return false;
		}
	}
}

class SimplePie_First_Item_Author_Test extends SimplePie_First_Item_Test
{
	function author()
	{
		if ($item = $this->first_item())
		{
			if ($author = $item->get_author())
			{
				return $author;
			}
		}
		return false;
	}
}

class SimplePie_First_Item_Category_Test extends SimplePie_First_Item_Test
{
	function category()
	{
		if ($item = $this->first_item())
		{
			if ($category = $item->get_category())
			{
				return $category;
			}
		}
		return false;
	}
}

class SimplePie_First_Item_Contributor_Test extends SimplePie_First_Item_Test
{
	function contributor()
	{
		if ($item = $this->first_item())
		{
			if ($contributor = $item->get_contributor())
			{
				return $contributor;
			}
		}
		return false;
	}
}

class SimplePie_Absolutize_Test extends SimplePie_Unit_Test2
{
	function test()
	{
		$this->result = SimplePie_Misc::absolutize_url($this->data['relative'], $this->data['base']);
	}
}

class SimplePie_Date_Test extends SimplePie_Unit_Test2
{
	function test()
	{
		$this->result = SimplePie_Misc::parse_date($this->data);
	}
}

class SimplePie_Feed_Category_Label_Test extends SimplePie_Feed_Category_Test
{
	function test()
	{
		if ($category = $this->category())
		{
			$this->result = $category->get_label();
		}
	}
}

class SimplePie_Feed_Copyright_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_copyright();
	}
}

class SimplePie_Feed_Description_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_description();
	}
}

class SimplePie_Feed_Image_Height_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_image_height();
	}
}

class SimplePie_Feed_Image_Link_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_image_link();
	}
}

class SimplePie_Feed_Image_Title_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_image_title();
	}
}

class SimplePie_Feed_Image_URL_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_image_url();
	}
}

class SimplePie_Feed_Image_Width_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_image_width();
	}
}

class SimplePie_Feed_Language_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_language();
	}
}

class SimplePie_Feed_Link_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_link();
	}
}

class SimplePie_Feed_Title_Test extends SimplePie_Feed_Test
{
	function test()
	{
		$feed = $this->feed();
		$this->result = $feed->get_title();
	}
}

class SimplePie_First_Item_Author_Name_Test extends SimplePie_First_Item_Author_Test
{
	function test()
	{
		if ($author = $this->author())
		{
			$this->result = $author->get_name();
		}
	}
}

class SimplePie_First_Item_Category_Label_Test extends SimplePie_First_Item_Category_Test
{
	function test()
	{
		if ($category = $this->category())
		{
			$this->result = $category->get_label();
		}
	}
}

class SimplePie_First_Item_Content_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_content();
		}
	}
}

class SimplePie_First_Item_Contributor_Name_Test extends SimplePie_First_Item_Contributor_Test
{
	function test()
	{
		if ($contributor = $this->contributor())
		{
			$this->result = $contributor->get_name();
		}
	}
}

class SimplePie_First_Item_Date_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_date('U');
		}
	}
}

class SimplePie_First_Item_Description_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_description();
		}
	}
}

class SimplePie_First_Item_ID_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_id();
		}
	}
}

class SimplePie_First_Item_Latitude_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_latitude();
		}
	}
}

class SimplePie_First_Item_Longitude_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_longitude();
		}
	}
}

class SimplePie_First_Item_Permalink_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_permalink();
		}
	}
}

class SimplePie_First_Item_Title_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			$this->result = $item->get_title();
		}
	}
}

class SimplePie_iTunesRSS_Channel_Block_Test extends SimplePie_First_Item_Test
{
	function test()
	{
		if ($item = $this->first_item())
		{
			if ($enclosure = $item->get_enclosure())
			{
				if ($restriction = $enclosure->get_restriction())
				{
					return $restriction->get_relationship();
				}
			}
		}
		return false;
	}
}

class diveintomark_Atom_Autodiscovery extends SimplePie_Unit_Test2
{
	var $data = array('url' => 'http://diveintomark.org/tests/client/autodiscovery/');
	
	function data()
	{
		$this->data['file'] =& new SimplePie_File($this->data['url'], 10, 5, null, SIMPLEPIE_USERAGENT);
		$this->name = $this->data['url'];
		$this->data['url'] = false;
	}
	
	function expected()
	{
		$this->expected = $this->data['file']->url;
	}
	
	function test()
	{
		$feed = new SimplePie();
		$feed->set_file($this->data['file']);
		$feed->enable_cache(false);
		$feed->init();
		$this->result = $feed->get_link();
	}
	
	function result()
	{
		if ($this->data['file']->url != 'http://diveintomark.org/tests/client/autodiscovery/')
		{
			parent::result();
		}
		static $done = array();
		$links = SimplePie_Misc::get_element('link', $this->data['file']->body);
		foreach ($links as $link)
		{
			if (!empty($link['attribs']['href']['data']) && !empty($link['attribs']['rel']['data']))
			{
				$rel = array_unique(SimplePie_Misc::space_seperated_tokens(strtolower($link['attribs']['rel']['data'])));
				$href = SimplePie_Misc::absolutize_url(trim($link['attribs']['href']['data']), $this->data['file']->url);
				if (!in_array($href, $done) && in_array('next', $rel))
				{
					$done[] = $this->data['url'] = $href;
					break;
				}
			}
		}
		if ($this->data['url'])
		{
			$this->run();
		}
	}
}

?>