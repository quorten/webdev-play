#! /usr/bin/env perl
# Lock the house logs file and run a shell command on it.

# This wrapper assumes that the shell command has no locking
# semantics, hence the locking done by this script.

use strict;
use warnings;

use POSIX;
use Fcntl ':flock'; # Import LOCK_* constants

die "Incorrect command line." if (@ARGV < 2);
my $log_filename = shift @ARGV;

# Open with an exclusive write lock, read mode.
my $result = open(my $fh, "<", $log_filename);
if (!$result) {
    die "Error opening log file: $!";
}
$result = flock($fh, LOCK_EX);
if (!$result) {
    close($fh);
    die "Error locking log file: $!";
}

# Execute our shell command.
my $retval = 0;
$result = system(@ARGV);
if ($result != 0) {
    $retval = $? >> 8;
}

# Close.
$result = close($fh);
if (!$result) {
    die "Error closing log file: $!";
}

exit($retval);
