<html>
<!--
WHERE:	home.html
WHY:	Homepage of WileyMUD III
WHO:	Dread Quixadhal, Dark Lord of VI.
WHEN:	950527
WHAT:
950527	Open to the World at Large!
-->
<head>
<title> WileyMUD III, the sickness of Dread Quixadhal. </title>
<link rev="made" href="mailto:quixadhal@shadowlord.org">
</head>
<body background="gfx/marble_background.jpg" bgcolor="#505050" text="#d0d0d0" link="#ffffbf" vlink="#ffa040">
<table border=0 cellspacing=5 cellpadding=5 width=100%>
<tr> <td>
<a href="linux.txt">
<img src="gfx/linux95.jpg" border=0 align=center width=111 height=80
 alt="(Linux95!)">
</a> </td>
<td align=center>
<center>
  <a href="mailto:quixadhal@shadowlord.org">Dread Quixadhal</a> welcomes you to<br>
<a href="telnet://wiley.shadowlord.org:3000/">
WileyMUD III!<br>
</a></center>
 </td>
<td align=right>
<a href="telnet://wiley.shadowlord.org:3000/">
<img src="gfx/orc.jpg" border=0 align=center width=116 height=163
 alt="(telnet)">
</a></td> 
</tr>
</table>
<p></p>
     As you finish your last sip of dark Nestharian Ale, the clash of steel
interrupts your dreamy repose yet again.  Running to the door, you see some
poor besotted elf trying to cast as the villager he just tried to rob brings
a thick wooden club down on his head.
     <p>
     "Get out of MY town you bastard!"  The sorry excuse that is Shylar's
only cityguard runs up and starts beating on the villager... blissfully
unaware that the fool elf started the fight in the first place.  You know
that after 12 days of playing time, you could walk out and slaughter ALMOST
everything in the room... but it's more fun to watch the newbie get pummeled
as Eli starts healing the villager.
<p>
     WileyMUD is a venerable DikuMUD, whose origins lie nearly six years in
the past.  Once upon a time, a wizard named Cyric took his brand new DikuMUD
code and began building a world to run in it.  He wisely eschewed ALL the
boring areas that came with the game, believing that only pathetic players
would enjoy playing a game that looked just like everyone else's game.  He
gathered such powerful names as Muthaarin'akya, Harlequin, Grimwell,
Derkhil, Elcid, Daisy, and even Dark Muidnar.  With their help, he managed
to terrorize the students of WMU for nearly three years with the evil
concoction known as WileyMUD II.
<p>
     After a time, Cyric grew bored with his creation.  He had acquired new
and more devious things known as "work", and "jobs".  As the ancient sun 3/60
gasped its last breath, so too did WileyMUD fade away.  For a time, hopes
held that WileyMUD III would be done, but the wretched "job" saw to it that
this never came to be.
<p>
     Then Dread Quixadhal, Dark Lord of VI and player of WileyMUD II,
discovered a way of summoning the shades of the dead.  With a mighty blast
of power, he travelled back in time and wrested a copy of the Source from
Cyric's treasure horde!  A battle ensued, and some of the World was damaged
in the scuffle.  However, he dragged Source, World, and Vile Muidnar back
into the present, and thus began the writing of WileyMUD III.  The addition
of Sedna and Dirk has helped to balance out our team, and occasional visits
by Highlander and Zar go mostly unnoticed.
<p>
     We have a relatively quiet mud here.  We have only recently started to
advertise our presence again, because only now have I decided that the game
is stable and finished enough to show to any but the most dedicated.  We were
once known as one of the hardest, yet well balanced, DikuMUD's on the planet.
After trying several other muds recently, I have to say this is still true.
So, if you would like to see an ALL original world that is NOT a cakewalk...
Please stop on by!
<p>
<table border=1 cellspacing=5 cellpadding=5 width=100%>
  <tr>
    <td>
      <center><h2>10 Most Popular KILLS!</h2></center>
    </td>
    <td>
      <center><h2>10 Most Recent DEATHS!</h2></center>
    </td>
  </tr>
  <tr>
    <td align="center">
      <font size="-1">
      <pre>
      <?php
      $handle = popen("/usr/bin/head -10 /home/wiley/killcount.log", "r");
      if ($handle) {
        printf("\n%-24s %-5s\n","Victim","Count");
        printf("%-24s %-5s\n","------------------------","-----");
        while (!feof($handle)) {
          $buffer = fgets($handle, 4096);
          echo $buffer;
        }
        fclose($handle);
      }
      ?>
      </pre>
      </font>
    </td>
    <td align="center">
      <font size="-1">
      <pre>
      <?php
      $handle = popen("/usr/bin/tail -10 /home/wiley/death.log", "r");
      if ($handle) {
        printf("\n%-19s %-16s %-24s %-24s\n","Date","Victim","Perp","Location");
        printf("%-19s %-16s %-24s %-24s\n","-------------------","----------------","------------------------","------------------------");
        while (!feof($handle)) {
          $buffer = fgets($handle, 4096);
          echo $buffer;
        }
        fclose($handle);
      }
      ?>
      </pre>
      </font>
    </td>
  </tr>
</table>
<a href="wiley.txt">
<img src="gfx/clouds.gif" border=0 align=center width=177 height=115
     alt="(docs)"> Building</a> information IS available!
</body>
</html>
