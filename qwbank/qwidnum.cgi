#! /usr/bin/env perl

use strict;
use warnings;
use CGI;

srand();

# Return the Luhn check digit (as an integer) from a string of ASCII
# digits.
sub compute_luhn_check
{
    my $ascii_num = shift;
    my $ascii_num_len = length($ascii_num);
    my $luhn_digit = 0;
    # Moving right, double every second digit, starting with the first
    # non-check digit.  (We also convert away from ASCII at this
    # step.)  Also sum up all the digits.
    my $j = $ascii_num_len;
    do {
	$j--;
	my $work_digit = ord(substr($ascii_num, $j, 1)) - 0x30;
	if (($ascii_num_len - 1 - $j) % 2 == 0) {
	    $work_digit <<= 1;
	    if ($work_digit > 9) {
		$work_digit -= 9;
	    }
	}
	$luhn_digit += $work_digit;
    } until $j <= 0;
    $luhn_digit = ($luhn_digit * 9) % 10;
    return $luhn_digit
}

# Generate a Quacky's World identification number from a seed serial
# number.
#
# The format of Quacky's World identification numbers is as follows:
# SSSSRRL
#
# Where:
# S = serial digit (i.e. 0001, 0002, 0003, etc.)
# R = random digit
# L = Luhn code check digit, computed from all the previous digits
sub gen_qw_id_num
{
    my $qw_serial_num = shift;
    my $random_digits = int(rand(100));
    my $ascii_num = sprintf("%04d%02d", $qw_serial_num, $random_digits);
    $ascii_num .= compute_luhn_check($ascii_num);
    return $ascii_num;
}

my $q = CGI->new;

my $qw_serial_num = $q->param('serial');

print $q->header('text/plain');

if ($qw_serial_num != -1) {
    my $qw_id_num = gen_qw_id_num($qw_serial_num);
    print "$qw_id_num\n";
} else {
    # Generate them all!
    foreach (0..9999) {
	my $qw_id_num = gen_qw_id_num($_);
	print "$qw_id_num\n";
    }
}
