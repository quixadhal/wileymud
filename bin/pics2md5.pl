#!/usr/bin/perl -w

use strict;
use English qw( −no_match_vars );
use Image::Size;
use Cwd qw(getcwd);

#die "NO!" if getcwd() =~ /\/home\/.*?\/public_html\/gfx\/wallpaper/i;

my $arg = shift;
my $nomove = 0;
$nomove = 1 if defined $arg and $arg =~ /--(nomove|nodir)/;

opendir DP, '.' or die "Can't opendir '.' $!";
my @pics = grep { /\.(jpeg|jpg|png|gif|webp)$/i && -f "./$_" } readdir DP;
closedir DP;

foreach my $filename (@pics) {
    next if ! -f $filename;
    open(FP, "-|", "md5sum", "--", $filename) or die "Can't open $filename $!";
    my $md5 = <FP>;
    close FP;

    chomp $md5;
    $md5 = (split /\s/, $md5)[0];
    my $ext = (split /\./, $filename)[-1];
    $ext = "jpg" if $ext =~ /(jpeg|jpg)/i;
    $ext = lc $ext;
    my $new_filename = "$md5.$ext";

    if( $filename ne $new_filename ) {
        rename $filename, $new_filename;
        printf "%s -> %s\n", $filename, $new_filename;
    }

    if(! $nomove) {
        my ($width, $height) = imgsize($new_filename);
        my $mode = ($width >= $height) ? "landscape" : "portrait";
        my $aspect;
        my ($aspect_x, $aspect_y);

        if ($mode eq "portrait") {
            $aspect = ($height != 0) ? ($width / $height) : -1;
            ($aspect_x, $aspect_y) = (int(16 * $aspect), 16);
        } else {
            $aspect = ($width != 0) ? ($height / $width) : -1;
            ($aspect_x, $aspect_y) = (16, int(16 * $aspect));
        }

        mkdir $mode if ! -d $mode;
        rename $new_filename, "$mode/$new_filename" if -d $mode;

        if ($aspect != 0.562500 ) {
            # This is not a 16:9 (or 9:16) image, and will need adjustment to fill
            # a "normal" monitor.
            printf  "WARNING! %9s %s: (%4d:%4d) (%2d:%2d) %6.3f\n",
                    $mode, $new_filename, $width, $height, $aspect_x, $aspect_y, $aspect;
        }
    }
}
