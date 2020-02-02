#! /usr/bin/env perl
# Generate directory links for our template files.

use strict;
use warnings;

use simple_template;
use photo_map;

my @user_subst_list = ();
my @subst_list = ();

# Process command-line arguments.

my $user_subst_filename = $ARGV[0];
open(my $user_subst_file, '<', $user_subst_filename) or
    die "Error opening substitution file: $!";

my $output_filename = $ARGV[1];
open(my $output_file, '>', $output_filename) or
    die "Error opening output file: $!";

# Read the user-defined substitutions.
@user_subst_list = read_subst($user_subst_file);
close($user_subst_file) or die "Error closing substitution file: $!";

# TODO: Remove directory links from the user links if they are
# present.

# Generate the directory links.
@subst_list = gen_dir_links();

# Output the file contents.
write_subst(\@subst_list, $output_file);
write_subst(\@user_subst_list, $output_file);
close($output_file) or die "Could not write links file: $!";
