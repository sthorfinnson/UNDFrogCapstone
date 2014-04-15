<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>Untitled Document</title>
</head>

<body>
<?php
$host="mysql.aero.und.edu"; // Host name
$username="sthorfinnson"; // Mysql username
$password="Buddy123"; // Mysql password
$db_name="sthorfinnson"; // Database name


// Connecting to server
mysql_connect("$host", "$username", "$password")or die("cannot connect to database at this time");
mysql_select_db("$db_name")or die("cannot select database at this time");

if (isset($_POST['submitted'])) { // Handle the form.

	require_once (MYSQL);
	
	// Trim all the incoming data:
	$trimmed = array_map('trim', $_POST);
	
	// Assume invalid values:
	$username = $password = FALSE;
	
	// Check for a username:
	if (preg_match ('/^[A-Z \'.-]{2,20}$/i', $trimmed['username'])) {
		$username = mysqli_real_escape_string ($dbc, $trimmed['username']);
	} else {
		echo '<p class="error">Please enter your username!</p>';
	}
	
	

	// Check for a password and match against the confirmed password:
	if (preg_match ('/^\w{4,20}$/', $trimmed['password1']) ) {
		if ($trimmed['password1'] == $trimmed['password2']) {
			$password = mysqli_real_escape_string ($dbc, $trimmed['password1']);
		} else {
			echo '<p class="error">Your password did not match the confirmed password!</p>';
		}
	} else {
		echo '<p class="error">Please enter a valid password!</p>';
	}
	
	if ($username && $password) { // If everything's OK...

		// Make sure the email address is available:
		$query = "SELECT user_id FROM users WHERE username='$username'";
		$r = mysqli_query ($dbc, $q) or trigger_error("Query: $q\n<br />MySQL Error: " . mysqli_error($dbc));
		
		if (mysqli_num_rows($r) == 0) { // Available.
		
			// Create the activation code:
			$a = md5(uniqid(rand(), true));
		
			// Add the user to the database:
			$query = "INSERT INTO users (username, password,) VALUES ('$username', SHA1('$password'))";
			$results = mysqli_query ($dbc, $q) or trigger_error("Query: $q\n<br />MySQL Error: " . mysqli_error($dbc));

			if (mysqli_affected_rows($dbc) == 1) { // If it ran OK.
			
				
				
				// Finish the page:
				echo '<h3>Thank you for registering! </h3>';
				include ('footer.html'); // Include the HTML footer.
				exit(); // Stop the page.
				
			} else { // If it did not run OK.
				echo '<p class="error">You could not be registered due to a system error. We apologize for any inconvenience.</p>';
			}
			
		} else { // The email address is not available.
			echo '<p class="error">That username has already been registered.</p>';
		}
		
	} else { // If one of the data tests failed.
		echo '<p class="error">Please re-enter your passwords and try again.</p>';
	}

	mysqli_close($dbc);

} // End of the main Submit conditional.
?>
	
<h1>Register</h1>
<form action="register.php" method="post">
	<fieldset>
	
	<p><b>Username:</b> <input type="text" name="username" size="20" maxlength="20" value="<?php if (isset($trimmed['username'])) echo $trimmed['username']; ?>" /></p>
	
    
		
    <p><b>Password:</b> 
      <input type="password" name="password1" size="20" maxlength="20" /> 
	  <small>Use only letters, numbers, and the underscore. Must be between 4 and 20 characters long.</small></p>
	<p><b>Confirm Password:</b> <input type="password" name="password2" size="20" maxlength="20" /></p>
	</fieldset>
	
	<div align="center"><input type="submit" name="submit" value="Register" /></div>
	<input type="hidden" name="submitted" value="TRUE" />

</form>


<?php // Include the HTML footer.
include ('footer.html'); ?>




?>