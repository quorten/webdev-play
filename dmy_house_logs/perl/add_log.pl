#! /usr/bin/env perl

use strict;
use warnings;

use POSIX;
use Fcntl ':flock'; # Import LOCK_* constants

use CGI::Fast;

sub write_log_error {
    my $q = shift;
    my $log_entry = shift;
    my $open_write_error = shift;

    warn "Failed log entry: $log_entry";
    warn "Error code: $open_write_error";

    print $q->header(-status=>'500 Internal Server Error');
    print $q->start_html(-title => '500 Internal Server Error',
			 -meta=>{'viewport'=>'width=device-width, initial-scale=1'});
    print $q->h1('500 Internal Server Error');
    print $q->p('Could not write the signature to the log.  ' .
		'Your data has been copied to the ' .
		'server error log instead.  ' .
		'Contact your website administrator.');
    print $q->h2('Log entry:');
    print $q->pre($log_entry);
    print $q->h2('Error code:');
    print $q->pre($open_write_error);
    print $q->p($q->a({href=>".."}, 'Return to landing page'));
    print $q->end_html;
}

while (my $q = new CGI::Fast) {
    # Generate the log entry.
    my $date = strftime "%F %T", localtime $^T;
    my $log_date_time = $q->param('log-date-time');
    $log_date_time = $date if ($log_date_time eq '');
    my $log_name = $q->param('log-name');
    my $session_log_name = $q->cookie('dmy_log_name');
    $log_name = $session_log_name if ($log_name eq '');
    my $log_action = $q->param('log-action');
    my $log_entry = "$log_date_time: $log_name: $log_action\n";

    # Verify all required parameters are provided.
    if ($log_name eq '' or $log_action eq '') {
	print $q->header(-status=>'400 Bad Request');
	print $q->start_html(-title => '400 Bad Request',
			     -meta=>{'viewport'=>'width=device-width, initial-scale=1'});
	print $q->h1('400 Bad Request');
	print $q->p('The input parameters were not correct.  ' .
		    'Check your input and try again.');
	print $q->end_html;
	next;
    }

    # Generate log filename, relative to script filename.
    my $script_filename = $ENV{'SCRIPT_FILENAME'};
    my $script_dirname = $script_filename;
    $script_dirname =~ s~(.*)/([^/]+)$~$1~; # dirname()
    my $log_dirname = $script_dirname;
    $log_dirname =~ s~(.*)/([^/]+)$~$1~; # dirname()
    my $log_filename = "$log_dirname/dmy_house_logs.log";

    # Open with an exclusive write lock, append mode.
    my $result = open(my $fh, ">>", $log_filename);
    if (!$result) {
	write_log_error($q, $log_entry, "Error opening log file: $!");
	next;
    }
    $result = flock($fh, LOCK_EX);
    if (!$result) {
	close($fh);
	write_log_error($q, $log_entry, "Error locking log file: $!");
	next;
    }

    # Append our log entry.
    print $fh $log_entry;

    # Close.
    $result = close($fh);
    if (!$result) {
	write_log_error($q, $log_entry, "Error closing log file: $!");
	next;
    }

    # Set the session login cookie.
    my $session_cookie = $q->cookie(
	-name=>'dmy_log_name',
	-value=>$log_name,
	-expires=>'+10m' # '+2h'
	);

    print $q->header(
	-type => 'text/html',
	-cookie  => [$session_cookie]);
    print $q->start_html(-title => 'Successful signature',
			 -meta=>{'viewport'=>'width=device-width, initial-scale=1'});
    print $q->h1('Successful signature');
    print $q->pre($log_entry);
    print $q->p($q->a({href=>$ENV{'HTTP_REFERER'}}, 'Add another'));
    print $q->p($q->a({href=>".."}, 'Return to landing page'));
    print $q->end_html;
}
