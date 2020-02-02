#! /usr/bin/env perl
# Generate view links for our template files.

use strict;
use warnings;

use simple_template;
use photo_map;

my @user_subst_list = ();
my @subst_list = ();

# Process command-line arguments.

# 0: Photo view
# 1: Vector view
# 2: Text view
# 4: Flag: Overlays off if set, on if clear
my $view_mode = $ARGV[0];

my $user_subst_filename = $ARGV[1];
open(my $user_subst_file, '<', $user_subst_filename) or
    die "Error opening substitution file: $!";

my $output_filename = $ARGV[2];
open(my $output_file, '>', $output_filename) or
    die "Error opening output file: $!";

# Read the user-defined substitutions.
@user_subst_list = read_subst($user_subst_file);
close($user_subst_file) or die "Error closing substitution file: $!";

# Generate the view links.
@subst_list = gen_view_links($view_mode);

# Output the file contents.
write_subst(\@subst_list, $output_file);
write_subst(\@user_subst_list, $output_file);
close($output_file) or die "Could not write links file: $!";
