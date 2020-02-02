#! /usr/bin/env perl
# FastCGI script to generate a photo mapping index page from a
# template and an argument file.

use strict;
use warnings;
use CGI::Fast;

use simple_template;
use photo_map;

my $COMM = "$FS_PREFIX/slim_gui";
my $TEMPLATE_FILENAME = "$COMM/index.html";

open(my $tmpl_file, '<', $TEMPLATE_FILENAME) or
    die "Error opening template file: $!";

while (my $q = new CGI::Fast) {
    my @subst_list = ();
    if (-e 'links.txt') {
	open(my $subst_file, '<', 'links.txt') or
	    die "Error opening substitution file: $!";
	@subst_list = read_subst($subst_file);
	close($subst_file) or die "Error closing substitution file: $!";
    } else {
	@subst_list = gen_default_links(0);
    }

    print $q->header('text/html');

    seek($tmpl_file, 0, 0);
    exec_subst(\@subst_list, $tmpl_file, *STDOUT);
}

close($tmpl_file) or die "Error closing template file: $!";
