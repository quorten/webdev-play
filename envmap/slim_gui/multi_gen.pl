#! /usr/bin/env perl
# Process a directory with photos and split it out into layers or
# angles, depending on whether there is an empty "L1" or "A1"
# directory within.

use strict;
use warnings;

# JJJ TODO: Add support for non-aspect ratio images.

opendir(my $dh, '.') || die "can't opendir: $!";
my @entries = sort(readdir($dh));
closedir($dh);

my $layer_found = 0;
my $angle_found = 0;

for my $entry (@entries) {
    if ($entry eq 'L1') {
	$layer_found = 1;
	last;
    }
    if ($entry eq 'A1') {
	$angle_found = 1;
	last;
    }
}

if ($layer_found) {
    rmdir('L1') or die "rmdir: $!";
    my $idx = 1;
    for my $entry (@entries) {
	if ($entry =~ /\.jpg$/ or $entry =~ /\.JPG$/) {
	    mkdir("L$idx") or die "mkdir: $!";
	    rename($entry, "L$idx/$entry") or die "rename: $!";
	    $idx++;
	}
    }
} elsif ($angle_found) {
    rmdir('A1') or die "rmdir: $!";
    my $idx = 1;
    for my $entry (@entries) {
	if ($entry =~ /\.jpg$/ or $entry =~ /\.JPG$/) {
	    mkdir("A$idx") or die "mkdir: $!";
	    rename($entry, "A$idx/$entry") or die "rename: $!";
	    $idx++;
	}
    }
}
