#! /usr/bin/env perl

use strict;
use warnings;

use POSIX;
use Fcntl ':flock'; # Import LOCK_* constants

use CGI::Fast;

sub read_log_error {
    my $q = shift;
    my $open_read_error = shift;

    warn "Log read error code: $open_read_error";

    print $q->header(-status=>'500 Internal Server Error');
    print $q->start_html(-title => '500 Internal Server Error',
			 -meta=>{'viewport'=>'width=device-width, initial-scale=1'});
    print $q->h1('500 Internal Server Error');
    print $q->p('Could not read the log.  ' .
		'Contact your website administrator.');
    print $q->h2('Error code:');
    print $q->pre($open_read_error);
    print $q->p($q->a({href=>".."}, 'Return to landing page'));
    print $q->end_html;
}

while (my $q = new CGI::Fast) {
    my $fh;
    my $result = open($fh, "<", "dmy_house_logs.log");
    if (!$result) {
	read_log_error($q, "Error opening log file: $!");
	next;
    }
    print $q->header(-type => 'text/html');
    print $q->start_html(-title => 'View house logs',
			 -meta=>{'viewport'=>'width=device-width, initial-scale=1'});

    print "<pre>\n";
    while (<$fh>) {
	print $q->escapeHTML($_);
    }
    $result = close($fh);
    if (!$result) {
	read_log_error($q, "Error closing log file: $!");
	next;
    }
    print "</pre>\n";

    print "<script type=\"text/javascript\">window.scrollTo(0,document.body.scrollHeight);</script>\n";
    print $q->end_html;
}
