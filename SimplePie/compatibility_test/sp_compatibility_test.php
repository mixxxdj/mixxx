<?php
if (isset($_GET['logopng']))
{
	$data='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';
	header('Content-type: image/png');
	echo base64_decode($data);
	exit;
}
else if (isset($_GET['background']))
{
	$data='R0lGODlhMAEeAeYAAP///8ni6cTf5+72+PD3+c3k6+nz9ufy9ev099Pn7bnZ48bg6LfY4uHv8/r8/f3+/v7//7bX4cjh6fj7/Mvj6vz9/rva4+z19/X6+/f7/Pn8/fb6+7jZ4vv9/bra473b5Lzb5LXX4b7c5b/c5e31+NTo7tvs8dfq7/H3+bjY4tnq79Hm7c/l69nr8PL4+t7t8sDd5cLe5uPw9Nvr8Mri6fP4+tLn7er099Hm7O/2+dDm7OTw9OXx9Nbp7tbp7+Lv8+Du8szj6sXg57bY4d3s8djq7+jz9tzs8cPe58fh6M7k68Pf5+by9fz+/sDd5vT5+vT5+97t8bbY4trr8P7+/v7+/+Pw8+Xx9dXo7sHe5vH4+fP5+sHd5t/u8s/l7AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAAAAAAALAAAAAAwAR4BAAf/gCGCg4SFhoeIiYqLjI2Oj5CRkpOUlZaXmJmam5ydnp+goaKNEaWmp6ipqqusra6vsLGys7S1tre4ubq7vL2+v8DBwsPExbBDyMnKy8zNzs/Q0dLT1NXW19jZ2tvc3d7KUuHi4+Tl5ufo6err7O3u7/Dx8vP09fb34wz6+/z9/v8AAwocSLCgwYMIEypcyLChw4cQI0qcSLGixYsYM2osmKKjx48gQ4ocSbKkyZMoU6pcybKly5cwY8qc+ZGDzZs4c+rcybOnz59AgwodSrSo0aNIkypdyrSp06dQo0qdSrWq1aAKsmrdyrWr169gw4odS7as2bNo06pdy7at27dw/+PKnUu3rt27ePOS9cC3r9+/gAMLHky4sOHDiBMrXsy4sePHkCNLnky5suXLmDNr3sz5sIXPoEOLHk26tOnTqFOrXs26tevXsGPLnk27tu3buHPr3s27t+/fqkEIH068uPHjyJMrX868ufPn0KNLn069uvXr2LNr3869u/fv4MOLb/6hvPnz6NOrX8++vfv38OPLn0+/vv37+PPr38+/v///AAYo4IAEFgifCAgmqOCCDDbo4IMQRijhhBRWaOGFGGao4YYcdujhhyCGKOKIJJZo4okoTjjCiiy26OKLMMYo44w01mjjjTjmqOOOPPbo449ABinkkEQWaeSRSCap5P+SNsLg5JNQRinllFRWaeWVWGap5ZZcdunll2CGKeaYZELpxJlopqnmmmy26eabcMYp55x01mnnnXjmqeeefPaZJheABirooIQWauihiCaq6KKMNuroo5BGKumklFZqqaBZZKrpppx26umnoIYq6qiklmrqqaimquqqrLbq6qubxiDrrLTWauutuOaq66689urrr8AGK+ywxBZr7LHI0orEssw26+yz0EYr7bTUVmvttdhmq+223Hbr7bfghtvsEuSWa+656Kar7rrstuvuu/DGK++89NZr77345quvuQL06++/AAcs8MAEF2zwwQgnrPDCDDfs8MMQRyzxxBRXbPH/xRhnrPHGHHeMsBAghyzyyCSXbPLJKKes8sost+zyyzDHLPPMNNdss8gL5Kzzzjz37PPPQAct9NBEF2300UgnrfTSTDft9NNQRy311FRXbfXVWGdNdBJcd+3112CHLfbYZJdt9tlop6322my37fbbcMctt9cS1G333XjnrffefPft99+ABy744IQXbvjhiCeu+OKMN+7445BHLvnklFcOeACYZ6755px37vnnoIcu+uikl2766ainrvrqrLfu+uuwxy777LTXbvvtuI9Ow+689+7778AHL/zwxBdv/PHIJ6/88sw37/zz0EffOwXUV2/99dhnr/323Hfv/ffghy/+//jkl2/++einr/767Lfv/vvwxy///PR/H8T9+Oev//789+///wAMoAAHSMACGvCACEygAhfIwAbmrwAQjKAEJ0jBClrwghjMoAY3yMEOevCDIAyhCEdIwhKa8IQoTKEKV8jCFrrwhTDcoBJmSMMa2vCGOMyhDnfIwx768IdADKIQh0jEIhrxiEhMYg1ZwMQmOvGJUIyiFKdIxSpa8YpYzKIWt8jFLnrxi2AMoxid6IUymvGMaEyjGtfIxja68Y1wjKMc50jHOtrxjnjMox73eEYd+PGPgAykIAdJyEIa8pCITKQiF8nIRjrykZCMpCQnSUlA4uCSmMykJjfJyU568v+ToAylKEdJylKa8pSoTKUqV8nKVmZyBbCMpSxnScta2vKWuMylLnfJy1768pfADKYwh0nMYhpTljZIpjKXycxmOvOZ0IymNKdJzWpa85rYzKY2t8nNbnrzm8tMgDjHSc5ymvOc6EynOtfJzna6853wjKc850nPetrznvjMpz73yc9++vOfAA2oQNtZgoIa9KAITahCF8rQhjr0oRCNqEQnStGKWvSiGM2oRjd6UCx49KMgDalIR0rSkpr0pChNqUpXytKWuvSlMI2pTGdKU5D24KY4zalOd8rTnvr0p0ANqlCHStSiGvWoSE2qUpfK1Kbm1AdQjapUp0rVqlr1qlj/zapWt8rVrnr1q2ANq1jHStaymlWqJ0irWtfK1ra69a1wjatc50rXutr1rnjNq173yte++vWvay2CYAdL2MIa9rCITaxiF8vYxjr2sZCNrGQnS9nKWvaymCWsCjbL2c569rOgDa1oR0va0pr2tKhNrWpXy9rWuva1sI1tZ1tA29ra9ra4za1ud8vb3vr2t8ANrnCHS9ziGve4yE2ucm07heY697nQja50p0vd6lr3utjNrna3y93ueve74A2veMf73BmY97zoTa9618ve9rr3vfCNr3znS9/62ve++M2vfvfLX/Sa4L8ADrCAB0zgAhv4wAhOsIIXzOAGO/jBEI6w/4QnTOEKB/gIGM6whjfM4Q57+MMgDrGIR0ziEpv4xChOsYpXzOIWu1jDRIixjGdM4xrb+MY4zrGOd8zjHvv4x0AOspCHTOQiG/nIM46CkpfM5CY7+clQjrKUp0zlKlv5yljOspa3zOUue/nLYGbyC8ZM5jKb+cxoTrOa18zmNrv5zXCOs5znTOc62/nOeM5zmbvA5z77+c+ADrSgB03oQhv60IhOtKIXzehGO/rRkI60pP0MhEpb+tKYzrSmN83pTnv606AOtahHTepSm/rUqE61qld96Qa4+tWwjrWsZ03rWtv61rjOta53zete+/rXwA62sIdN7GIb+9jITrayl//N7GY7O9c/iLa0p03talv72tjOtra3ze1ue/vb4A63uMdN7nKb+9zTtoK6183udrv73fCOt7znTe962/ve+M63vvfN7377+98AZ7cMBk7wghv84AhPuMIXzvCGO/zhEI+4xCdO8Ypb/OIYz3jBd8Dxjnv84yAPuchHTvKSm/zkKE+5ylfO8pa7/OUwj7nMPc6Dmtv85jjPuc53zvOe+/znQA+60IdO9KIb/ehIT7rSl37zKzj96VCPutSnTvWqW/3qWM+61rfO9a57/etgD7vYx052qDPh7GhPu9rXzva2u/3tcI+73OdO97rb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';
	header('Content-type: image/gif');
	echo base64_decode($data);
	exit;
}

$php_ok = (function_exists('version_compare') && version_compare(phpversion(), '4.3.0', '>='));
$pcre_ok = extension_loaded('pcre');
$curl_ok = function_exists('curl_exec');
$zlib_ok = extension_loaded('zlib');
$mbstring_ok = extension_loaded('mbstring');
$iconv_ok = extension_loaded('iconv');
if (extension_loaded('xmlreader'))
{
	$xml_ok = true;
}
elseif (extension_loaded('xml'))
{
	$parser_check = xml_parser_create();
	xml_parse_into_struct($parser_check, '<foo>&amp;</foo>', $values);
	xml_parser_free($parser_check);
	$xml_ok = isset($values[0]['value']);
}
else
{
	$xml_ok = false;
}

header('Content-type: text/html; charset=UTF-8');

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">

<html lang="en">
<head>
<title>SimplePie: Server Compatibility Test 1.2</title>

<style type="text/css">
body {
	font:14px/1.4em "Lucida Grande", Verdana, Arial, Helvetica, Clean, Sans, sans-serif;
	letter-spacing:0px;
	color:#333;
	margin:0;
	padding:0;
	background:#fff url(<?php echo pathinfo(__FILE__, PATHINFO_BASENAME); ?>?background) repeat-x top left;
}

div#site {
	width:550px;
	margin:20px auto 0 auto;
}

a {
	color:#000;
	text-decoration:underline;
	padding:0 1px;
}

a:hover {
	color:#fff;
	background-color:#333;
	text-decoration:none;
	padding:0 1px;
}

p {
	margin:0;
	padding:5px 0;
}

em {
	font-style:normal;
	background-color:#ffc;
}

ul, ol {
	margin:10px 0 10px 20px;
	padding:0 0 0 15px;
}

ul li, ol li {
	margin:0 0 7px 0;
	padding:0 0 0 3px;
}

h2 {
	font-size:18px;
	padding:0;
	margin:30px 0 10px 0;
}

h3 {
	font-size:16px;
	padding:0;
	margin:20px 0 5px 0;
}

h4 {
	font-size:14px;
	padding:0;
	margin:15px 0 5px 0;
}

code {
	font-size:1.1em;
	background-color:#f3f3ff;
	color:#000;
}

em strong {
    text-transform: uppercase;
}

table#chart {
	border-collapse:collapse;
}

table#chart th {
	background-color:#eee;
	padding:2px 3px;
	border:1px solid #fff;
}

table#chart td {
	text-align:center;
	padding:2px 3px;
	border:1px solid #eee;
}

table#chart tr.enabled td {
	/* Leave this alone */
}

table#chart tr.disabled td, 
table#chart tr.disabled td a {
	color:#999;
	font-style:italic;
}

table#chart tr.disabled td a {
	text-decoration:underline;
}

div.chunk {
	margin:20px 0 0 0;
	padding:0 0 10px 0;
	border-bottom:1px solid #ccc;
}

.footnote,
.footnote a {
	font:10px/12px verdana, sans-serif;
	color:#aaa;
}

.footnote em {
	background-color:transparent;
	font-style:italic;
}
</style>

<script type="text/javascript">
// Sleight - Alpha transparency PNG's in Internet Explorer 5.5/6.0
// (c) 2001, Aaron Boodman; http://www.youngpup.net

if (navigator.platform == "Win32" && navigator.appName == "Microsoft Internet Explorer" && window.attachEvent) {
	document.writeln('<style type="text/css">img, input.image { visibility:hidden; } </style>');
	window.attachEvent("onload", fnLoadPngs);
}

function fnLoadPngs() {
	var rslt = navigator.appVersion.match(/MSIE (\d+\.\d+)/, '');
	var itsAllGood = (rslt != null && Number(rslt[1]) >= 5.5);

	for (var i = document.images.length - 1, img = null; (img = document.images[i]); i--) {
		if (itsAllGood && img.src.match(/\png$/i) != null) {
			var src = img.src;
			var div = document.createElement("DIV");
			div.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + src + "', sizing='scale')";
			div.style.width = img.width + "px";
			div.style.height = img.height + "px";
			img.replaceNode(div);
		}
		img.style.visibility = "visible";
	}
}
</script>

</head>

<body>

<div id="site">
	<div id="content">

		<div class="chunk">
			<h2 style="text-align:center;"><img src="<?php echo pathinfo(__FILE__, PATHINFO_BASENAME); ?>?logopng" alt="SimplePie Compatibility Test" title="SimplePie Compatibility Test" /></h2>
			<table cellpadding="0" cellspacing="0" border="0" width="100%" id="chart">
				<thead>
					<tr>
						<th>Test</th>
						<th>Should Be</th>
						<th>What You Have</th>
					</tr>
				</thead>
				<tbody>
					<tr class="<?php echo ($php_ok) ? 'enabled' : 'disabled'; ?>">
						<td>PHP&sup1;</td>
						<td>4.3.0 or higher</td>
						<td><?php echo phpversion(); ?></td>
					</tr>
					<tr class="<?php echo ($xml_ok) ? 'enabled, and sane' : 'disabled, or broken'; ?>">
						<td><a href="http://php.net/xml">XML</a></td>
						<td>Enabled</td>
						<td><?php echo ($xml_ok) ? 'Enabled, and sane' : 'Disabled, or broken'; ?></td>
					</tr>
					<tr class="<?php echo ($pcre_ok) ? 'enabled' : 'disabled'; ?>">
						<td><a href="http://php.net/pcre">PCRE</a>&sup2;</td>
						<td>Enabled</td>
						<td><?php echo ($pcre_ok) ? 'Enabled' : 'Disabled'; ?></td>
					</tr>
					<tr class="<?php echo ($curl_ok) ? 'enabled' : 'disabled'; ?>">
						<td><a href="http://php.net/curl">cURL</a></td>
						<td>Enabled</td>
						<td><?php echo (extension_loaded('curl')) ? 'Enabled' : 'Disabled'; ?></td>
					</tr>
					<tr class="<?php echo ($zlib_ok) ? 'enabled' : 'disabled'; ?>">
						<td><a href="http://php.net/zlib">Zlib</a></td>
						<td>Enabled</td>
						<td><?php echo ($zlib_ok) ? 'Enabled' : 'Disabled'; ?></td>
					</tr>
					<tr class="<?php echo ($mbstring_ok) ? 'enabled' : 'disabled'; ?>">
						<td><a href="http://php.net/mbstring">mbstring</a></td>
						<td>Enabled</td>
						<td><?php echo ($mbstring_ok) ? 'Enabled' : 'Disabled'; ?></td>
					</tr>
					<tr class="<?php echo ($iconv_ok) ? 'enabled' : 'disabled'; ?>">
						<td><a href="http://php.net/iconv">iconv</a></td>
						<td>Enabled</td>
						<td><?php echo ($iconv_ok) ? 'Enabled' : 'Disabled'; ?></td>
					</tr>
				</tbody>
			</table>
		</div>

		<div class="chunk">
			<h3>What does this mean?</h3>
			<ol>
				<?php if ($php_ok && $xml_ok && $pcre_ok && $mbstring_ok && $iconv_ok && $curl_ok && $zlib_ok): ?>
				<li><em>You have everything you need to run SimplePie properly!  Congratulations!</em></li>
				<?php else: ?>
					<?php if ($php_ok): ?>
						<li><strong>PHP:</strong> You are running a supported version of PHP.  <em>No problems here.</em></li>
						<?php if ($xml_ok): ?>
							<li><strong>XML:</strong> You have XMLReader support or a version of XML support that isn't broken installed.  <em>No problems here.</em></li>
							<?php if ($pcre_ok): ?>
								<li><strong>PCRE:</strong> You have PCRE support installed. <em>No problems here.</em></li>
								<?php if ($curl_ok): ?>
									<li><strong>cURL:</strong> You have <code>cURL</code> support installed.  <em>No problems here.</em></li>
								<?php else: ?>
									<li><strong>cURL:</strong> The <code>cURL</code> extension is not available.  SimplePie will use <code>fsockopen()</code> instead.</li>
								<?php endif; ?>
	
								<?php if ($zlib_ok): ?>
									<li><strong>Zlib:</strong> You have <code>Zlib</code> enabled.  This allows SimplePie to support GZIP-encoded feeds.  <em>No problems here.</em></li>
								<?php else: ?>
									<li><strong>Zlib:</strong> The <code>Zlib</code> extension is not available.  SimplePie will ignore any GZIP-encoding, and instead handle feeds as uncompressed text.</li>
								<?php endif; ?>
	
								<?php if ($mbstring_ok && $iconv_ok): ?>
									<li><strong>mbstring and iconv:</strong> You have both <code>mbstring</code> and <code>iconv</code> installed!  This will allow SimplePie to handle the greatest number of languages.  Check the <a href="http://simplepie.org/wiki/faq/supported_character_encodings">Supported Character Encodings</a> chart to see what's supported on your webhost.</li>
								<?php elseif ($mbstring_ok): ?>
									<li><strong>mbstring:</strong> <code>mbstring</code> is installed, but <code>iconv</code> is not.  Check the <a href="http://simplepie.org/wiki/faq/supported_character_encodings">Supported Character Encodings</a> chart to see what's supported on your webhost.</li>
								<?php elseif ($iconv_ok): ?>
									<li><strong>iconv:</strong> <code>iconv</code> is installed, but <code>mbstring</code> is not.  Check the <a href="http://simplepie.org/wiki/faq/supported_character_encodings">Supported Character Encodings</a> chart to see what's supported on your webhost.</li>
								<?php else: ?>
									<li><strong>mbstring and iconv:</strong> <em>You do not have either of the extensions installed.</em> This will significantly impair your ability to read non-English feeds, as well as even some English ones.  Check the <a href="http://simplepie.org/wiki/faq/supported_character_encodings">Supported Character Encodings</a> chart to see what's supported on your webhost.</li>
								<?php endif; ?>
							<?php else: ?>
								<li><strong>PCRE:</strong> Your PHP installation doesn't support Perl-Compatible Regular Expressions.  <em>SimplePie is a no-go at the moment.</em></li>
							<?php endif; ?>
						<?php else: ?>
							<li><strong>XML:</strong> Your PHP installation doesn't support XML parsing.  <em>SimplePie is a no-go at the moment.</em></li>
						<?php endif; ?>
					<?php else: ?>
						<li><strong>PHP:</strong> You are running an unsupported version of PHP.  <em>SimplePie is a no-go at the moment.</em></li>
					<?php endif; ?>
				<?php endif; ?>
			</ol>
		</div>

		<div class="chunk">
			<?php if ($php_ok && $xml_ok && $pcre_ok && $mbstring_ok && $iconv_ok) { ?>
				<h3>Bottom Line: Yes, you can!</h3>
				<p><em>Your webhost has its act together!</em></p>
				<p>You can download the latest version of SimplePie from <a href="http://simplepie.org/downloads/">SimplePie.org</a> and install it by <a href="http://simplepie.org/wiki/setup/start">following the instructions</a>.  You can find example uses with <a href="http://simplepie.org/ideas/">SimplePie Ideas</a>.</p>
				<p>Take the time to read <a href="http://simplepie.org/wiki/setup/start">Requirements and Getting Started</a> to make sure you're prepared to use SimplePie. No seriously, read them.</p>
				<p class="footnote"><em><strong>Note</strong></em>: Passing this test does not guarantee that SimplePie will run on your webhost &mdash; it only ensures that the basic requirements have been addressed.</p>
			<?php } else if ($php_ok && $xml_ok && $pcre_ok) { ?>
				<h3>Bottom Line: Yes, you can!</h3>
				<p><em>For most feeds, it'll run with no problems.</em>  There are <a href="http://simplepie.org/wiki/faq/supported_character_encodings">certain languages</a> that you might have a hard time with though.</p>
				<p>You can download the latest version of SimplePie from <a href="http://simplepie.org/downloads/">SimplePie.org</a> and install it by <a href="http://simplepie.org/wiki/setup/start">following the instructions</a>.  You can find example uses with <a href="http://simplepie.org/ideas/">SimplePie Ideas</a>.</p>
				<p>Take the time to read <a href="http://simplepie.org/wiki/setup/start">Requirements and Getting Started</a> to make sure you're prepared to use SimplePie. No seriously, read them.</p>
				<p class="footnote"><em><strong>Note</strong></em>: Passing this test does not guarantee that SimplePie will run on your webhost &mdash; it only ensures that the basic requirements have been addressed.</p>
			<?php } else { ?>
				<h3>Bottom Line: We're sorry…</h3>
				<p><em>Your webhost does not support the minimum requirements for SimplePie.</em>  It may be a good idea to contact your webhost, and ask them to install a more recent version of PHP as well as the <code>xmlreader</code>, <code>xml</code>, <code>mbstring</code>, <code>iconv</code>, <code>curl</code>, and <code>zlib</code> extensions.</p>
			<?php } ?>
		</div>

		<div class="chunk">
			<p class="footnote">&sup1; &mdash; SimplePie 2 will not support PHP 4.x. The core PHP team has discontinued PHP 4.x patches and support. <a href="http://simplepie.org/blog/2007/07/13/simplepie-is-going-php5-only/">Read the announcement.</a></p>
			<p class="footnote">&sup2; &mdash; Some recent versions of the PCRE (PERL-Compatible Regular Expression) engine compiled into PHP have been buggy, and are the source of PHP segmentation faults (e.g. crashes) which cause random things like blank, white screens. Check the <a href="http://simplepie.org/support/">Support Forums</a> for the latest information on patches and ongoing fixes.</p>
		</div>

	</div>

</div>

</body>
</html>