<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="refresh" content="2;URL=completed_work_orders.php">
<meta name="keywords" content="automatic redirection">
<title>Untitled Document</title>
</head>

<body>
<?php
$host="localhost"; // Host name 
$username="root"; // Mysql username 
$password="root"; // Mysql password 
$db_name="CS297"; // Database name 
$tbl_name="work_orders"; // Table name


$wo_num = $_POST['button'];




// Connect to server and select database.
mysql_connect("$host", "$username", "$password")or die("cannot connect"); 
mysql_select_db("$db_name")or die("cannot select DB");



// update data in mysql database 
$sql="UPDATE $tbl_name SET completed = 1 WHERE wo_num='$wo_num'";
$result=mysql_query($sql);

// if successfully updated. 
if($result){
echo "Update Successful";
echo "<BR>";
}

else {
echo "ERROR";
}

?>
</body>
</html>