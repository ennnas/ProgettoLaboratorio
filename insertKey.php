
<?php
$myfile = fopen("example.txt", "w") or die("Unable to open file!");
$txt = $_POST['key'];
fwrite($myfile, $txt);
fclose($myfile);
echo '<meta http-equiv="refresh" content="3;url=index.php">Scrittura su file avvenuta con successo. Reindirizzamento...';
?> 
