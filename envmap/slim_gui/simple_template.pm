# Common functions, written in Perl, for implementing a very simple
# templating engine.

package simple_template;

BEGIN {
    use strict;
    use warnings;
    use Exporter ();
    our @ISA = qw(Exporter);
    our @EXPORT = qw(&read_subst &write_subst &exec_subst);
}

# Read a colon-newline delimited file containing text strings and
# their corresponding text to replace, and return a corresponding list
# of text-substitution lists.  Blank lines are skipped, and
# unrecognized lines (i.e. missing a delimiter) raise a syntax error
# exception.
#
# Arguments:
#   1. file descriptor, file to read from
# Return value:
#   On success, returns the substitution list.
#   On failure (syntax error), an exception is raised.
sub read_subst
{
    my $substs_file = shift;

    my @subst_list = ();
    my $line = 0;
    while (<$substs_file>) {
	chomp;
	$line++;
	my $delim_idx = index $_, ':';
	if ($delim_idx == -1) {
	    next if (length($_) == 0 or $_ =~ /\s/);
	    # Syntax error
	    die "Line $line: syntax error";
	}
	my $f1 = substr $_, 0, $delim_idx;
	my $f2 = substr $_, $delim_idx + 1;
	push @subst_list, [ $f1, $f2 ];
    }
    return @subst_list;
}

# Write a substitutions list out to a file.
# Arguments:
#   1. List reference, substitutions generated from read_subst
#   2. file descriptor, file to read from
sub write_subst
{
    my $subst_list = shift;
    my $output_file = shift;

    for my $elem (@$subst_list) {
	print $output_file "$elem->[0]:$elem->[1]\n";
    }
}

# Read an input file descriptor line-by-line, execute substitutions
# specified in a hash, and write to an output file descriptor.
#
# Arguments:
#   1. list reference, substitutions generated from read_subst
#   2. file descriptor, template file to read
#   3. file descriptor, output file to write
sub exec_subst
{
    my $subst_list = shift;
    my $tmpl_file = shift;
    my $output_file = shift;

    while (<$tmpl_file>) {
	my $new_str = $_;
	# TODO FIXME: Multiple template strings may match.  In this
	# case, make sure to match and replace the longest string
	# first.
	for my $elem (@$subst_list) {
	    my $match_pos = 0;
	    while (($match_pos = index($new_str, $elem->[0],
				       $match_pos)) != -1) {
		my $s1 = substr($new_str, 0, $match_pos);
		my $s2 = substr($new_str, $match_pos + length($elem->[0]));
		$new_str = $s1 . $elem->[1] . $s2;
		$match_pos += length($elem->[1]);
	    }
	}
	print $output_file $new_str;
    }
}

1;
