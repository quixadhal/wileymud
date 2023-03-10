<?php
  // This command is useful with this file.
  // find . -type d -print0 | xargs -0 -I{} ln -s ../index.php {}/index.php

  $title_array = explode("/", getcwd());
  //$title = ucfirst($title_array[count($title_array)-1]);
  $title = $title_array[count($title_array)-1];

  $no_desc = "No description available.";

  # light mode
  #$file_colors = array("#99FF99","#DDFFDD");
  #$dir_colors = array("#9999FF","#DDDDFF");
  #$parent_color = "#FF9999";
  #$bg_color = "#FFFFFF";
  #$text_color = "#000000";
  #$link_color = "#000099";
  #$visited_link_color = "#0000FF";

  # dark mode
  $file_colors = array("#005500","#002200");
  $dir_colors = array("#000055","#000022");
  $parent_color = "#550000";
  $bg_color = "#000000";
  $text_color = "#d0d0d0";
  $link_color = "#ffffbf";
  $visited_link_color = "#ffa040";

  $files = get_data();

function natSortKey(&$arrIn)
{
  $key_array = array();
  $arrOut = array();
  
  if(is_array($arrIn)) {
      if(count($arrIn) > 0) {
          foreach ( $arrIn as $key=>$value ) {
              $key_array[] = $key;
          }
          natsort( $key_array);
          foreach ( $key_array as $key=>$value ) {
              $arrOut[$value] = $arrIn[$value];
          }
      }
  }
  $arrIn = $arrOut;
}

function human_size($size, $format = NULL) {
  $sizes = array('B ', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB');
  if ($format === NULL)
    $format = '%01.2f %s';
  $lastsizestring = end($sizes);
  foreach ($sizes as $sizestring) {
    if($size < 1024)
      break;
    if($sizestring != $lastsizestring)
      $size /= 1024;
  }
  if($sizestring == $sizes[0])
    $format = '%01d %s'; // Bytes aren't normally fractional
  return sprintf($format, $size, $sizestring);
}

function get_dirs()
{
  global $no_desc;
  $dirs = array();
  if( is_readable(getcwd()) ) {
    $dir = opendir(getcwd());
    while( ($file = readdir($dir)) !== false ) {
      if( is_readable($file) ) {
        if( is_dir($file) && $file != "." && $file != ".." ) {
          $dirs[$file] = $no_desc;
        }
      }
    }
    closedir($dir);
  }
  natSortKey($dirs);
  return $dirs;
}

function get_files()
{
  global $no_desc;
  $files = array();
  if( is_readable(getcwd()) ) {
    $dir = opendir(getcwd());
    while( ($file = readdir($dir)) !== false ) {
      if( is_readable($file) ) {
        if( is_file($file) && $file != "index.php" && $file != "index.dat" ) {
          $files[$file] = $no_desc;
        }
      }
    }
    closedir($dir);
  }
  natSortKey($files);
  return $files;
}

function get_dir_listing()
{
  $result = array();
  $dirs = get_dirs();
  $files = get_files();
  $result = $dirs + $files;
  return $result;
}

function get_index_dat()
{
  global $no_desc;
  $result = array();
  if ( file_exists("index.dat") && (($fp = fopen("index.dat", "r")) !== NULL) ) {
    while($line = fgets($fp)) {
      $data = explode("\t", $line);
      $filename = $data[0];
      $desc = $data[1];
      if( $desc == "" )
        $desc = $no_desc;
      if(is_readable($filename)) {
        if(is_dir($filename))
          $dirs[$filename] = $desc;
        else
          $files[$filename] = $desc;
      }
    }
    fclose($fp);
  }
  natSortKey($dirs);
  natSortKey($files);
  $result = $dirs + $files;
  return $result;
}

function get_data()
{
  $entries = array_merge(get_dir_listing(), get_index_dat());
  return $entries;
}

?>
<html>
<head>
<title><?php echo $title; ?></title>
</head>
<body bgcolor="<?php echo $bg_color;?>" text="<?php echo $text_color;?>" link="<?php echo $link_color;?>" vlink="<?php echo $visited_link_color;?>">
<table width="100%">
<tr>
  <td colspan="3" bgcolor="<?php echo $bg_color;?>" align="center"><h1><?php echo $title;?></h1></td>
</tr>
<?php if ( count($files) < 1 ) { ?>
  <tr><td colspan="3" align="center"><h3>No files available.</h3></td></tr>
<?php } else { ?>
<tr>
  <td width="25%" bgcolor="<?php echo $parent_color;?>"><a href="..">..</a></td>
  <td bgcolor="<?php echo $parent_color;?>">Parent Directory</td>
  <td align="left" width="10%" bgcolor="<?php echo $parent_color;?>">Date</td>
  <td align="left" width="10%" bgcolor="<?php echo $parent_color;?>">Size</td>
</tr>
<?php
  $i = 0;
  foreach ( $files as $key => $value ) {
    if(is_dir($key)) {
      $bgcolor = $dir_colors[ $i % 2 ];
      $size = "(Directory)";
      $date = date("Y-m-d", filemtime($key)); // filectime()
    } else {
      $bgcolor = $file_colors[ $i % 2 ];
      $size = human_size(filesize($key));
      $date = date("Y-m-d", filemtime($key));
    }
    ?>
    <tr>
      <td width="25%" bgcolor="<?php echo $bgcolor;?>"><strong><a href="<?php echo $key;?>"><?php echo $key;?></a></strong></td>
      <td bgcolor="<?php echo $bgcolor;?>"><?php echo $value;?></td>
      <td align="left" width="10%" bgcolor="<?php echo $bgcolor;?>"><?php echo $date;?></td>
      <td align="left" width="10%" bgcolor="<?php echo $bgcolor;?>"><?php echo $size;?></td>
    </tr>
    <?php
    $i++;
  }
}
?>
</table>
</body>
</html>
