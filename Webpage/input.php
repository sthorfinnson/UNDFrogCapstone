<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="refresh" content="2;URL=active_work_orders.php">
<meta name="keywords" content="automatic redirection">
<title>Submit</title>
</head>

<body>
<?php
//the example of inserting data with variable from HTML form
//input.php

mysql_connect('localhost','sthorfinnson','und_2014');//database connection

mysql_select_db("woodfrogs");

 $date_due = $_POST['file_path'];
 $description = $_POST['match_path'];
 

//inserting data order

echo("This is where the program runs using the file paths");



?>



</body>
</html>