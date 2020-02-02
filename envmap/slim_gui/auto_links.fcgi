#! /usr/bin/env perl
# FastCGI script to generate a default `links.txt' file.

use strict;
use warnings;
use CGI::Fast;

use FindBin qw($RealBin);
use lib $RealBin;

use simple_template;
use photo_map;

# If zero, assume running as a (F)CGI script.  If 1, assume running as
# a command-line tool and thus do not emit the Content-Type header,
# etc.
my $BATCH_MODE = 0;

if (@ARGV >= 1 and $ARGV[0] eq '-b') {
    $BATCH_MODE = 1;
}

while (my $q = new CGI::Fast) {
    if (!$BATCH_MODE) {
	print $q->header('text/plain');
    }

    my @subst_list = gen_default_links(0);
    write_subst(\@subst_list, *STDOUT);
}
