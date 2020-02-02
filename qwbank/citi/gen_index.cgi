#! /usr/bin/env perl

use strict;
use warnings;
use CGI;

use simple_template;
use qwidnum;

srand();

# Show HTML boilerplate around generated SVG image?
my $show_dummy = 0;

my $post_error = "";

my $q = CGI->new();

opendir(my $dh, "records") || die "can't opendir: $!";
my @entries = readdir($dh);
closedir($dh);
my @citizens = ();
for my $entry (@entries) {
    if ($entry =~ /(.*)\.txt$/ && $entry ne 'qw_g_serial.txt') {
	push(@citizens, $1);
    }
}
@citizens = sort(@citizens);

print $q->header(-type=>'text/html', -charset=>'utf-8');

print $q->start_html("Citizens Index");
print $q->h1("Citizens Index"), "\n";
print $q->start_ul(), "\n";
for my $name (@citizens) {
    my $safe_name = $name;
    $safe_name =~ s/#/%23/g;
    print $q->li(
	$name,
	$q->a({href=>"citizen_form.html#$name"}, 'DB'),
	$q->a({href=>"gen_book.cgi?q-name=$safe_name"}, 'CBK')),
	"\n";
}
print $q->end_ul(), "\n";
print $q->end_html, "\n";
