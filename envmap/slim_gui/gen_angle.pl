#! /usr/bin/env perl
# Generate the assets required for an "angle" in the photo map.
#
# 1. Generate `photo_overlay.svg' that includes the JPEG photograph.
# 2. (optional) Generate a `links.txt' file with navigation links
#    automatically generated from the directory hierarchy.  Note that
#    a default templates file can be dynamically generated on the fly.
# 3. Generate a thumbnail of the photo.

# 4. (optional) Generate empty `overlay.svg', empty `map.svg', and
#    empty `vector.svg'.  (Yeah, yeah, I know, try to use artificial
#    neural networks to automate generating the vector image, but for
#    most of the early history of computers and the web, AI couldn't
#    do that well.)
# 5. (optional) Generate `photo_map.svg', `vector_map.svg', and
#    `vector_overlay.svg'.  (Note that we can stack SVGs one on top of
#    another, alternatively.)
# 6. (optional) Try to generate a low-resolution, low-color bitmap
#    icon the photo.  (Again, for most of computer history, this was
#    better done by hand than by machine.)

use strict;
use warnings;

use FindBin qw($RealBin);
use lib $RealBin;

use simple_template;
use photo_map;

# Width/height of frame, i.e. container SVG
my $FRAME_WIDTH = 1280;
my $FRAME_HEIGHT = 960;
# Width/height of source photo
my $WIDTH = 1280;
my $HEIGHT = 960;

# Because users can always be foolhearty (i.e. including myself), we
# need to allow for both portrait and landscape orientations.
# Furthermore, rotating in SVG can be nice if you don't want to fool
# around with the JPEG orientation specifier.

# ident = identity
# CCW = counterclockwise
# CW = clockwise

my $svg_transform_ident = 'matrix(1,0,0,1,0,0)';
my $svg_transform_ccw = 'matrix(0,-1,1,0,0,0)';
my $svg_transform_cw = 'matrix(0,1,-1,0,0,0)';

my @landscape_ident = (
    [ 'TEMPLATE-frame-width', $FRAME_WIDTH ],
    [ 'TEMPLATE-frame-height', $FRAME_HEIGHT ],
    [ 'TEMPLATE-x', 0 ],
    [ 'TEMPLATE-y', 0 ],
    [ 'TEMPLATE-width', $WIDTH ],
    [ 'TEMPLATE-height', $HEIGHT ],
    [ 'TEMPLATE-transform', $svg_transform_ident ],
);

my @landscape_ccw = (
    [ 'TEMPLATE-frame-width', $FRAME_HEIGHT ],
    [ 'TEMPLATE-frame-height', $FRAME_WIDTH ],
    [ 'TEMPLATE-x', -$WIDTH ],
    [ 'TEMPLATE-y', 0 ],
    [ 'TEMPLATE-width', $WIDTH ],
    [ 'TEMPLATE-height', $HEIGHT ],
    [ 'TEMPLATE-transform', $svg_transform_ccw ],
);

my @landscape_cw = (
    [ 'TEMPLATE-frame-width', $FRAME_HEIGHT ],
    [ 'TEMPLATE-frame-height', $FRAME_WIDTH ],
    [ 'TEMPLATE-x', 0 ],
    [ 'TEMPLATE-y', -$HEIGHT ],
    [ 'TEMPLATE-width', $WIDTH ],
    [ 'TEMPLATE-height', $HEIGHT ],
    [ 'TEMPLATE-transform', $svg_transform_cw ],
);

my @portrait_ident = (
    [ 'TEMPLATE-frame-width', $FRAME_HEIGHT ],
    [ 'TEMPLATE-frame-height', $FRAME_WIDTH ],
    [ 'TEMPLATE-x', 0 ],
    [ 'TEMPLATE-y', 0 ],
    [ 'TEMPLATE-width', $HEIGHT ],
    [ 'TEMPLATE-height', $WIDTH ],
    [ 'TEMPLATE-transform', $svg_transform_ident ],
);

my @portrait_ccw = (
    [ 'TEMPLATE-frame-width', $FRAME_WIDTH ],
    [ 'TEMPLATE-frame-height', $FRAME_HEIGHT ],
    [ 'TEMPLATE-x', -$HEIGHT ],
    [ 'TEMPLATE-y', 0 ],
    [ 'TEMPLATE-width', $HEIGHT ],
    [ 'TEMPLATE-height', $WIDTH ],
    [ 'TEMPLATE-transform', $svg_transform_ccw ],
);

my @portrait_cw = (
    [ 'TEMPLATE-frame-width', $FRAME_WIDTH ],
    [ 'TEMPLATE-frame-height', $FRAME_HEIGHT ],
    [ 'TEMPLATE-x', 0 ],
    [ 'TEMPLATE-y', -$WIDTH ],
    [ 'TEMPLATE-width', $HEIGHT ],
    [ 'TEMPLATE-height', $WIDTH ],
    [ 'TEMPLATE-transform', $svg_transform_cw ],
);

my @subst_list = ();

# Process command-line arguments.

# Orientation dimensions of original image.
# 0: landscape
# 1: portrait
my $dim_orient = $ARGV[0];

# Transformation to perform on original image.
# 0: identity
# 1: counterclockwise
# 2: clockwise
my $xform_orient = $ARGV[1];

# 1. `photo_overlay.svg' can easily be generated via templating.

# Find the photo filename.
opendir(my $dh, '.') || die "can't opendir: $!";
my @entries = readdir($dh);
closedir($dh);

my $photo_filename = '';
for my $entry (@entries) {
    if ($entry =~ /\.jpg$/ or $entry =~ /\.JPG$/) {
	$photo_filename = $entry;
	last;
    }
}

die 'No photo found.' if (!$photo_filename);

push @subst_list, [ 'TEMPLATE-photo', $photo_filename ];

# Add the orientation arguments.
if ($dim_orient == 0) {
    if ($xform_orient == 0) {
	push @subst_list, @landscape_ident;
    } elsif ($xform_orient == 1) {
	push @subst_list, @landscape_ccw;
    } else { # $xform_orient == 2
	push @subst_list, @landscape_cw;
    }
} else { # $dim_orient == 1
    if ($xform_orient == 0) {
	push @subst_list, @portrait_ident;
    } elsif ($xform_orient == 1) {
	push @subst_list, @portrait_ccw;
    } else { # $xform_orient == 2
	push @subst_list, @portrait_cw;
    }
}

# Execute the substitution on the proper template.
my $tmpl_filename = $FS_PREFIX . '/slim_gui/' . 'photo_overlay.svg';
open(my $tmpl_file, '<', $tmpl_filename) or
    die "Error opening template file: $!";
open(my $output_file, '>', 'photo_overlay.svg') or
    die "Error opening output file: $!";
exec_subst(\@subst_list, $tmpl_file, $output_file);

close($tmpl_file) or die "Error closing template file: $!";
close($output_file) or die "Error closing output file: $!";

# 2. (skipped) Generate `links.txt'.

if (0) {
    open($output_file, '>', 'links.txt') or
	die "Error opening output file: $!";
    @subst_list = gen_default_links(0);
    write_subst(\@subst_list, $output_file);

    close($output_file) or die "Error closing output file: $!";
}

# 3. (Re-)Generate the thumbnail.
unlink('thumbnail.jpg');
my @resize_args = ("$FS_PREFIX/slim_gui/resize.sh", '-a', 128, '-b', 96);
if ($xform_orient == 1) {
    push @resize_args, '-c';
} elsif ($xform_orient == 2) {
    push @resize_args, '-w';
}
if ($dim_orient == 0) {
    push @resize_args, '-l';
} else {
    push @resize_args, '-p';
}
system(@resize_args);
rename("resize/$photo_filename", 'thumbnail.jpg') or die "rename: $!";
rmdir('resize') or die "rmdir: $!";
