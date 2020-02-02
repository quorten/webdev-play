#! /usr/bin/env perl
# Generate a file from a template and a substitutions file.
#
# Example:
# cat >substs.txt <<EOF
# one:two
# three:four
# EOF
#
# cat >template.txt <<EOF
# This is one example.
# EOF
#
# ./cmd_tmpl.pl substs.txt template.txt result.txt
#
# The substitutions file is processed as follows:
# * The part before the colon is matched.
# * The part after is the replacement.
# * Additional colons after the first colon are treated as part of
#   the second part.
# * Blank lines are ignored.
# * Non-template lines trigger an error.

use strict;
use warnings;

use FindBin qw($RealBin);
use lib $RealBin;

use simple_template;

my $DEBUG = 0;

my $subst_filename = $ARGV[0];
my $tmpl_filename = $ARGV[1];
my $output_filename = $ARGV[2];

open(my $subst_file, '<', $subst_filename) or
    die "Error opening substitution file: $!";
open(my $tmpl_file, '<', $tmpl_filename) or
    die "Error opening template file: $!";
open(my $output_file, '>', $output_filename) or
    die "Error opening output file: $!";

my @subst_list = read_subst($subst_file);
close($subst_file) or die "Error closing substitution file: $!";

if ($DEBUG) {
    for my $elem (@subst_list) {
	for my $sel (@{ $elem }) {
	    print "$sel ";
	}
	print "\n";
    }
}

exec_subst(\@subst_list, $tmpl_file, $output_file);

close($tmpl_file) or die "Error closing template file: $!";
close($output_file) or die "Error closing output file: $!";
