#! /usr/bin/env perl

use strict;
use warnings;

use CGI::Fast;
use tmpl_log_tail;

my $LOG_TAIL_LEN = 5;
my $TMPL_NAME = "level_3.html";

while (my $q = new CGI::Fast) {
    gen_tmpl_log_tail($LOG_TAIL_LEN, $TMPL_NAME, $q);
}
