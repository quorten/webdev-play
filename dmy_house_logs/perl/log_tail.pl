#! /usr/bin/env perl

use strict;
use warnings;

use CGI::Fast;
use tmpl_log_tail;

my $LOG_TAIL_LEN = 5;

my @segs = split /\//, $0;
my $tmpl_name = $segs[-1];
$tmpl_name =~ s/_log_tail\.cgi$/.html/;

while (my $q = new CGI::Fast) {
    gen_tmpl_log_tail($LOG_TAIL_LEN, $tmpl_name, $q);
}
