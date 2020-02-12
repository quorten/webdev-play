#! /usr/bin/env perl
# Generate the HTML code for a "sign your name" sheet, using a YAML
# data file as input.

use warnings;
use strict;

use YAML::Syck;

# Set this for interoperability with other YAML/Syck bindings:
# e.g. Load(Yes) becomes 1 and Load(No) becomes .
$YAML::Syck::ImplicitTyping = 1;

my $data_filename = $ARGV[0];
my $level_name = 'level 1';
my $head_name = 'head.html';
if (@ARGV >= 2) { $level_name = $ARGV[1]; }
if (@ARGV >= 3) { $head_name = $ARGV[2]; }

my $fh;
my @lines;

open($fh, "<", $head_name) or die "HTML header: $!";
@lines = <$fh>;
# Execute any template replacements within the HTML header.
for my $line (@lines) {
    my $tmpl_pos = index($line, 'TEMPLATE-level_name');
    my $tmpl_len = length('TEMPLATE-level_name');
    if ($tmpl_pos != -1) {
	$line = substr($line, 0, $tmpl_pos) .
	    $level_name .
	    substr($line, $tmpl_pos + $tmpl_len);
    }
}
my $html_head = join('', @lines);
close($fh) or die "$!";

open($fh, "<", "tail.html") or die "HTML trailer: $!";
@lines = <$fh>;
my $html_tail = join('', @lines);
close($fh) or die "$!";

open($fh, "<", $data_filename) or die "YAML template: $!";
@lines = <$fh>;
my $yaml = join('', @lines);
close($fh) or die "$!";

my $data = Load($yaml);

# Systems may be specified in multiple sections, to account for
# different frequency actions.  For our purposes, we combine them all
# into only a single section for each system.

my @system_order = ();
my %system_actions = ();

for my $system (@{$data}) {
    my $syscode = $system->{"codename"};
    if (exists $system_actions{$syscode}) {
	my $exist_system = $system_actions{$syscode};
	push @{$exist_system->{'actions'}}, @{$system->{'actions'}};
    } else {
	push @system_order, $syscode;
	$system_actions{$syscode} = $system;
    }
}

# Print the HTML header.
print $html_head;

for my $syscode (@system_order) {
    # Print the system header.
    my $system = $system_actions{$syscode};
    my $sysname = $system->{"system"};
    print "\n";
    print "    <p>\n";
    print "      <button type=\"button\" id=\"show-cntr-$syscode\">\n";
    print "      $sysname</button>\n";
    print "    </p>\n";
    print "    <ul class=\"cntr-system\" id=\"cntr-$syscode\">\n";
    # Print the system actions.
    for my $action (@{$system->{"actions"}}) {
	my $actname = $action->{"name"};
	my $actcode = $action->{"codename"};
	print "      <li><input id=\"$syscode-$actcode\" name=\"log-action\" type=\"radio\"\n";
	print "             value=\"$sysname: $actname\" />\n";
	print "      <label for=\"$syscode-$actcode\">\n";
	print "        $actname\n";
	print "      </label></li>\n";
    }
    # End of system container.
    print "    </ul>\n";
}

# Print the HTML trailer/tail.
print $html_tail;
