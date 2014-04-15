<?php # Script 16.5 - index.php
// This is the main page for the site.

// Include the configuration file:
require_once ('includes/config.inc.php'); 

// Set the page title and include the HTML header:
$page_title = 'Welcome to Wood Frog Matching!';
include ('includes/header.html');

// Welcome the user (by name if they are logged in):
echo '<h1>Welcome';
if (isset($_SESSION['first_name'])) {
	echo ", {$_SESSION['first_name']}!";
	
}
	
// If no first_name session variable exists, redirect the user:

if (!isset($_SESSION['first_name'])) {

	

	$url = BASE_URL . 'login.php'; // Define the URL.

	ob_end_clean(); // Delete the buffer.

	header("Location: $url");

	exit(); // Quit the script.
    
    }
	

echo '</h1>';
?>

<form action="input.php" method="post">
	<fieldset>
<p><table width="643" border="0" cellspacing="5" cellpadding="5">
  
  <tr>
    <th width="71" scope="row">File Path</th>
    <td width="537"><input name="date_due" type="text" size="15"/> &nbsp; &nbsp;</td>
  </tr>
  <tr>
    <th scope="row">Match Al</th>
    <td><textarea name="description" cols ="75" rows = "2" dir= "ltr"></textarea></td>
  </tr>
  <tr>
  	<th scope="row">Comments</th>
  	<td><textarea name="comments" cols="75" rows="2" dir="ltr"></textarea></td>
  </tr>
</table>
</p>

<div align="center"><input type="submit" name="submit" value="Submit" /></div>
	<input type="hidden" name="submitted" value="TRUE" />

</form>



<?php // Include the HTML footer file:
include ('includes/footer.html'); 
?>
