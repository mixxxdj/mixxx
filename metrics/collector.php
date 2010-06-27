<?PHP
//$raw_post_data = "SYSINFO:0:CMETRICS:1:1.0 beta 2:2:LINUX:3:X86:4:Jan 17 2009:5:MIXXX:80000004:933C7639148513A656A40BFBCD397DBD:80000005:AD46A523718AFCB9402457882A158DD:80000000:0|2|B|9|A|756E6547|6C65746E|49656E69|106C2|1020800|40C39D|BFE9FBFF|4FBA5901|E3080C0|0|0|0|0|0|0|0|0|0|0|40|40|3|20220|1|2|1|0|0|0|0|0|0|0|0|0|0|0|0|0|7280203|0|0|2501|80000008|0|0|0|0|0|1|100000|20202020|20202020|746E4920|52286C65|74412029|54286D6F|4320294D|4E205550|20303732|20402020|30362E31|7A4847|0|0|0|0|0|0|2008040|0|0|0|0|0|2020|0|0|0:80000001:7DEA8000|54F97000|0|0|32C0000:80000002:/|EF53|DEF13|222E1|1000;/lib/init/rw|1021994|3EF54|3EF54|1000;/proc|9FA0|0|0|1000;/sys|62656572|0|0|1000;/var/run|1021994|3EF54|3EF39|1000;/var/lock|1021994|3EF54|3EF54|1000;/dev|1021994|3EF54|3ECA7|1000;/dev/shm|1021994|3EF54|3EF27|1000;/dev/pts|1CD1|0|0|1000;/home|EF53|14E26E|7CF58|1000;/sys/kernel/security|73636673|0|0|1000;/proc/sys/fs/binfmt_misc|42494E4D|0|0|1000;/media/disk|4D44|7A680|A834|8000;:80000003:Not Implemented";
$raw_post_data = file_get_contents("php://input");

if (trim($raw_post_data) == "") { die("no data submitted."); }
$result = mysql_db_query("cmetrics","INSERT INTO capture (capture_data) values ('$raw_post_data')");
if (!$result) {
   die("insert failed: ".mysql_error());
}

/*
$msg_definition = array( 
"SYSINFO" => array(
	0 => "LIBNAME", 
	1 => "LIBVERSION", 
	2 => "LIBOS",
	3 => "LIBARCH", 
	4 => "LIBBUILDDATE", 
	5 => "LIBCLIENT",
	80000000 => "CPUINFO", 
	80000001 => "MEMINFO",
	80000002 => "FSINFO",
	80000003 => "OSINFO",
	80000004 => "RELEASE_ID", 
	80000005 => "USER_ID"
	)
);

$msg = array();
list($msg_type, $raw_data) = explode(":", $raw_post_data,2);

do {
  list($key, $value, $raw_data) = explode(":", $raw_data, 3);
  $msg[$msg_definition[$msg_type][$key]] = $value;
} while (strpos($raw_data,":") !== false);

foreach ($msg as $key=>$value) {
// insert into metrics ("USER_ID", "RELEASE_ID", ) values ( $msg["USER_ID"], $msg["RELEASE_ID"] );
   echo $msg["USER_ID"].",".$msg["RELEASE_ID"]." key: $key => value: $value\n";
}
*/
?>

