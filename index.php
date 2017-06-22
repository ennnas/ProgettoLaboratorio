<!DOCTYPE html>
<html >
<head>
  <meta charset="UTF-8">
  <title>Progetto Laboratorio</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/normalize/5.0.0/normalize.min.css">
  <link rel="stylesheet" href="css/style.css">
  <script type="text/javascript">
      function valida() {
        if(document.chiave.key.value=="")
          {
          alert("Chiave obbligatoria. Impossibile procedere.");
          return false;
          }
        
        return true;
      }
    </script>
  
</head>
<body>
  
<div class="container">
  <div class="row header">
    <h1>Twitter IoT &nbsp;</h1>
    <h3>#Hashtag popolare? Controlla qui!</h3>
  </div>
  <div class="row body">
    <form name='chiave' action="insertKey.php" method="POST" onSubmit="return valida();">
      <ul>
        
        <li>
          <p class="left">
            <label for="utente">Hashtag utente</label>
            <input type="text" name="utente" placeholder="#HashtagUtente" readonly="readonly" value="<?php
            $myfile = fopen("example.txt", "r") or die("Unable to open file!");
            echo fread($myfile,filesize("example.txt"));
            fclose($myfile);?>
            " />
          </p>
          <p class="pull-right">
            <label for="twitter">Trend Twitter</label>
            <input type="text" name="twitter" placeholder="#HashtagTwitter" readonly="readonly" value="<?php
            $myfile = fopen("trend.txt", "r") or die("Unable to open file!");
            echo fread($myfile,filesize("trend.txt"));
            fclose($myfile);?>
            " />      
          </p>
        </li>
        
        <li>
          <p>
            <label for="key">Inserisci qui <span class="req">*</span></label>
            <input type="text" name="key" placeholder="#YourHashtag" />
          </p>
        </li>        
        <li><div class="divider"></div></li>
      
        <li>
          <input class="btn btn-submit" type="submit" value="Submit" />
          <small>or press <strong>enter</strong></small>
        </li>
        
      </ul>
    </form>  
  </div>
</div>
  
  
</body>
</html>
