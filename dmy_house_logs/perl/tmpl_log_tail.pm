package tmpl_log_tail;

use strict;
use warnings;

use POSIX;
use Fcntl ':flock'; # Import LOCK_* constants

# use CGI::Fast; # Not needed in Perl-isms

BEGIN {
    use Exporter ();
    use Cwd;
    our @ISA = qw(Exporter);
    our @EXPORT = qw(&gen_tmpl_log_tail);
}

sub read_log_error {
    my $q = shift;
    my $open_read_error = shift;
    print $q->header(-status=>'500 Internal Server Error');
    print $q->start_html(-title => '500 Internal Server Error',
			 -meta=>{'viewport'=>'width=device-width, initial-scale=1'});
    print $q->h1('500 Internal Server Error');
    print $q->p('Could not generate sign your name page with log tail.  ' .
		'Contact your website administrator.');
    print $q->h2('Error code:');
    print $q->pre($open_read_error);
    print $q->end_html;
}

sub gen_tmpl_log_tail {
    my $LOG_TAIL_LEN = shift;
    my $TMPL_NAME = shift;
    my $q = shift;

    # Read the template file.
    my $fh;
    my $result = open($fh, "<", $TMPL_NAME);
    if (!$result) {
	read_log_error($q, "Error opening template file: $!");
	return;
    }
    my @tmpl_lines = <$fh>;
    $result = close($fh);
    if (!$result) {
	read_log_error($q, "Error closing template file: $!");
	return;
    }
    # print $template_text;

    # Generate log filename, relative to script filename.
    my $script_filename = $ENV{'SCRIPT_FILENAME'};
    my $script_dirname = $script_filename;
    $script_dirname =~ s~(.*)/([^/]+)$~$1~; # dirname()
    my $log_dirname = $script_dirname;
    $log_dirname =~ s~(.*)/([^/]+)$~$1~; # dirname()
    my $log_filename = "$log_dirname/dmy_house_logs.log";

    # Open the log file with a read lock.
    $result = open($fh, "<", $log_filename);
    if (!$result) {
	read_log_error($q, "Error opening log file: $!");
	return;
    }
    $result = flock($fh, LOCK_SH);
    if (!$result) {
	close($fh);
	read_log_error($q, "Error locking log file: $!");
	return;
    }

    # Read the tail of the log lines into an array.
    my @log_lines = ();
    while (<$fh>) {
	push(@log_lines, $_);
	if (@log_lines > $LOG_TAIL_LEN) {
	    shift(@log_lines);
	}
    }

    # Close.
    $result = close($fh);
    if (!$result) {
	read_log_error($q, "Error closing log file: $!");
	return;
    }

    # Paste the log lines into the template text.

    print $q->header('text/html');
    for my $line (@tmpl_lines) {
	my $tmpl_pos = index($line, '<!-- INSERT LOG TAIL HERE -->');
	if ($tmpl_pos != -1) {
	    for my $log_line (@log_lines) {
		chomp($log_line);
		print $log_line;
		print "<br />\n";
	    }
	} else {
	    print $line;
	}
    }
}

1;
