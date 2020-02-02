#! /usr/bin/env perl

use strict;
use warnings;

use CGI::Fast;
# use Image::Magick;

# Use `shell_identify()' if 1, otherwise use `perl_identify()'.
# `shell_identify()' is faster but less secure.
my $USE_SHELL_IDENTIFY = 1;

# If zero, assume running as a (F)CGI script.  If 1, assume running as
# a command-line tool and thus do not emit the Content-Type header,
# etc.
my $BATCH_MODE = 0;

my $title = "Citizens Index";
# List the contents of the current working directory by default.
my $dir_name = '.';
my $index_name = 'index.cgi';
# Directory containing resized thumbnail images.
my $thumbs_dir = 'thumbnails';
my $thumb_max_dim = 128;
my $thumb_width = 128;
my $thumb_height = 96;

my $stylesheet = <<END;
div.imgthumb {
    display: inline-block;
    text-align: center;
    margin: 0.25em;
}
END

# Find the width and height of an image via the PerlMagick interface.
# Although this is more convenient, it is also an order of magnitude
# slower than `shell_identify()' for single use on large images.
# However, thumbnail-sized images are faster to process via
# `perl_identify()'.
#
# Arguments:
#   string, filename
# Returns:
#   list, (width, height)

sub perl_identify
{
    my $filename = shift;
    my $image = Image::Magick->new;
    my $x = $image->Read($filename);
    warn "$x" if "$x";
    my @result = ($image->Get('width'), $image->Get('height'));
    undef $image;
    return @result;
}

# Call the ImageMagick `identify' command via a shell invocation so
# that the width and height of an image can be obtained.
#
# WARNING: The filename is escaped using Perl escaping, not
# shell-escaping.  Use at your own risk.
#
# Arguments:
#   string, filename
# Returns:
#   list, (width, height)
sub shell_identify
{
    my $filename = shift;
    my @output = split(/ /, `identify \Q$filename\E`);
    return split(/x/, $output[2]);
}

# IPC::Run is another option to consider, beyond the recommendation in
# `perlsec'.
# 20180123/https://stackoverflow.com/questions/619926/should-i-escape-shell-arguments-in-perl

# Arguments:
#   CGI, CGI object
#   string, error to display
# Returns:
#   Nothing
sub output_error {
    my $q = shift;
    my $list_error = shift;
    print $q->header(-type => 'text/html',
		     -status => '500 Internal Server Error');
    print $q->start_html("Error listing directory"),
	$q->h1("Error listing directory"), "\n",
	$q->p($list_error), "\n",
	$q->end_html;
}

# Output HTML for a single thumbnail.
#
# Arguments:
#   CGI, CGI object
#   string, filename
# Returns:
#   Nothing
sub output_thumbnail {
    my $q = shift;
    my $name = shift;
    my $safe_name = $name;
    $safe_name =~ s/#/%23/g;

    # Now determine filename for the photo, if any.
    my $file = '';
    if (-f "Citizens2/$name.jpg") {
	$dir_name = 'Citizens2';
	$file = "$name.jpg";
    } elsif (-f "Citizens2/$name.png") {
	$dir_name = 'Citizens2';
	$file = "$name.png";
    } elsif (-f "Citizens/$name.JPG") {
	$dir_name = 'Citizens';
	$file = "$name.JPG";
    }
    # return if $file =~ /^\./ or $file eq $thumbs_dir or $file eq $index_name;

    my $img_thumb = '';
    if ($file =~ /\.((PNG)|(png)|(JPG)|(jpg))$/) {
	my $thumb_file = "$dir_name/$thumbs_dir/$file";
	# We use `stat()' (rather than `lstat()') for the file test so
	# that we can test the target of symbolic links.
	stat($thumb_file);
	$thumb_file = $file if (!-f _);
	# If applicable, size the HTML element to the proportions of
	# the thumbnail image.
	if ($USE_SHELL_IDENTIFY) {
	    # SKIP for more speed.
	    # ($thumb_width, $thumb_height) = shell_identify($thumb_file);
	} else {
	    ($thumb_width, $thumb_height) = perl_identify($thumb_file);
	}
	if ($thumb_width and $thumb_height) {
	    if ($thumb_width > $thumb_max_dim or
		$thumb_height > $thumb_max_dim) {
		if ($thumb_width > $thumb_height) {
		    $thumb_height = int($thumb_height / $thumb_width);
		    $thumb_width = $thumb_max_dim;
		} else {
		    $thumb_width = int($thumb_width / $thumb_height);
		    $thumb_height = $thumb_max_dim;
		}
	    }
	}
	my $safe_thumb_file = $thumb_file;
	$safe_thumb_file =~ s/#/%23/g;
	$img_thumb = $q->img(
	    {alt => $file, src => $safe_thumb_file,
	     width => $thumb_width, height => $thumb_height}
	    ) . "\n";
    }
    my $safe_file = "$dir_name/$file";
    $safe_file =~ s/#/%23/g;
    print $q->div(
	{class => 'imgthumb'}, "\n",
	$q->a({href => $safe_file}, "\n",
	    $img_thumb),
	$q->br(), "\n",
	$name, "\n",
	$q->a({href=>"citizen_form.html#$name"}, 'DB'),
	$q->a({href=>"gen_book.cgi?q-name=$safe_name"}, 'CBK'),
	"\n",
	), "\n";
}

######################################################################
# Show HTML boilerplate around generated SVG image?
my $show_dummy = 0;

my $post_error = "";

my $q = CGI->new();

opendir(my $dh, "records") || die "can't opendir: $!";
my @entries = readdir($dh);
closedir($dh);
my @citizens = ();
for my $entry (@entries) {
    if ($entry =~ /(.*)\.txt$/ && $entry ne 'qw_g_serial.txt') {
	push(@citizens, $1);
    }
}
@citizens = sort(@citizens);

print $q->header(-type=>'text/html', -charset=>'utf-8');

print $q->start_html(-title => $title,
		     -style => {-code => $stylesheet});
print $q->h1("Citizens Index"), "\n";
for my $name (@citizens) {
    output_thumbnail($q, $name);
}
print $q->end_html, "\n";
