#! /usr/bin/env perl

use strict;
use warnings;
use CGI;
use File::Spec;
use POSIX; # for `strftime()`
use Text::Wrap;

use simple_template;
use qwidnum;

srand();

# Show HTML boilerplate around generated SVG image?
my $show_dummy = 0;

my $post_error = "";

my $q = CGI->new();

my $filename = "records/records.txt";

my $q_mode = $q->param('q-mode');

my $q_photo = $q->param("q-photo");
my $q_name = $q->param("q-name");
my $q_idnum = $q->param("q-idnum");
my $q_uuid = $q->param("q-uuid");
my $q_issued = $q->param("q-issued");
my $q_dob = $q->param("q-dob");
my $q_nat_on = $q->param("q-nat-on");
my $q_expires = $q->param("q-expires");

my $qb_custom = $q->param("qb-custom");
my $qb_full_name = $q->param("qb-full-name");
my $qb_height = $q->param("qb-height");
my $qb_weight = $q->param("qb-weight");
my $qb_fur_color = $q->param("qb-fur-color");
my $qb_eye_color = $q->param("qb-eye-color");
my $qb_born_at = $q->param("qb-born-at");
my $q_biography = $q->param("q-biography");
my $q_bio_custom = $q->param("q-bio-custom");

my $qa_education = $q->param("qa-edu");
my $qa_employ = $q->param("qa-employ");
my $qa_crimes = $q->param("qa-crimes");
my $qa_exid_txt = $q->param('qa-exid-txt');

if ($q_name) {
    $filename = "records/$q_name.txt";
}

sub encode_tmpl {
    my $in_str = shift;
    $in_str =~ s/\\/\\\\/g;
    $in_str =~ s/\r\n?/\n/g;
    $in_str =~ s/\n/\\n/g;
    return $in_str;
}

sub decode_tmpl {
    my $in_str = shift;
    $in_str =~ s/\\n/\n/g;
    $in_str =~ s/\\\\/\\/g;
    return $in_str;
}

open(my $fin, "<", $filename);
my @data_recs = read_subst($fin);
close($fin);

for my $elem (@data_recs) {
    if ($elem->[0] eq 'qw_name') {
	$q_name = $elem->[1];
    } elsif ($elem->[0] eq 'qw_idnum') {
	$q_idnum = $elem->[1];
    } elsif ($elem->[0] eq 'qw_uuid') {
	$q_uuid = $elem->[1];
    } elsif ($elem->[0] eq 'qw_issued') {
	$q_issued = $elem->[1];
    } elsif ($elem->[0] eq 'qw_dob') {
	$q_dob = $elem->[1];
    } elsif ($elem->[0] eq 'qw_nat_on') {
	$q_nat_on = $elem->[1];
    } elsif ($elem->[0] eq 'qw_expires') {
	$q_expires = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_custom') {
	$qb_custom = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_full_name') {
	$qb_full_name = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_height') {
	$qb_height = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_weight') {
	$qb_weight = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_fur_color') {
	$qb_fur_color = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_eye_color') {
	$qb_eye_color = $elem->[1];
    } elsif ($elem->[0] eq 'qwb_born_at') {
	$qb_born_at = $elem->[1];
    } elsif ($elem->[0] eq 'qw_biography') {
	$q_biography = decode_tmpl($elem->[1]);
    } elsif ($elem->[0] eq 'qw_bio_custom') {
	$q_bio_custom = $elem->[1];
    } elsif ($elem->[0] eq 'qw_education') {
	$qa_education = decode_tmpl($elem->[1]);
    } elsif ($elem->[0] eq 'qw_employment') {
	$qa_employ = decode_tmpl($elem->[1]);
    } elsif ($elem->[0] eq 'qw_crimes') {
	$qa_crimes = decode_tmpl($elem->[1]);
    } elsif ($elem->[0] eq 'qw_exid_txt') {
	$qa_exid_txt = decode_tmpl($elem->[1]);
    }
}

# For displaying the biography, convert all lines to tspans.
$Text::Wrap::columns = 60;
my @bio_lines = split(/\n/, fill('', '', $q_biography));
my $q_disp_biography;
my $cur_y = 57.874779;
for my $line (@bio_lines) {
    $q_disp_biography .= '<tspan x="413.06799" y="' . $cur_y . '">' .
	$line . '</tspan>';
    $cur_y += 15.0;
}

# Only display the date, and not the time, for "issued on," etc.
my @pieces = split(/ /, $q_issued);
my $q_disp_issued = @pieces[0];
@pieces = split(/ /, $q_dob);
my $q_disp_dob = @pieces[0];
$q_disp_dob = "@pieces[0] @pieces[1]" if (@pieces[0] eq 'circa');
@pieces = split(/ /, $q_nat_on);
my $q_disp_nat_on = @pieces[0];
$q_disp_nat_on = "@pieces[0] @pieces[1]" if (@pieces[0] eq 'circa');

my @subst_list = (
    ['TEMPLATE-qw_photo', "photo.jpg"],
    ['TEMPLATE-qw_name', $q_name],
    ['TEMPLATE-qw_idnum', $q_idnum],
    ['TEMPLATE-qw_uuid', $q_uuid],
    ['TEMPLATE-qw_issued', $q_disp_issued],
    ['TEMPLATE-qw_dob', $q_disp_dob],
    ['TEMPLATE-qw_nat_on', $q_disp_nat_on],
    ['TEMPLATE-qw_expires', $q_expires],
    ['TEMPLATE-qwb_custom', $qb_custom],
    ['TEMPLATE-qwb_full_name', $qb_full_name],
    ['TEMPLATE-qwb_height', $qb_height],
    ['TEMPLATE-qwb_weight', $qb_weight],
    ['TEMPLATE-qwb_fur_color', $qb_fur_color],
    ['TEMPLATE-qwb_eye_color', $qb_eye_color],
    ['TEMPLATE-qwb_born_at', $qb_born_at],
    ['TEMPLATE-qw_biography', $q_disp_biography],
    ['TEMPLATE-qw_bio_custom', $q_bio_custom],
    ['TEMPLATE-qw_education', $qa_education],
    ['TEMPLATE-qw_employment', $qa_employ],
    ['TEMPLATE-qw_crimes', $qa_crimes],
    ['TEMPLATE-qw_exid_txt', $qa_exid_txt],
);

if ($show_dummy) {
    print $q->header(-type=>'text/html', -charset=>'utf-8');
} else {
    print $q->header(-type=>'image/svg+xml', -charset=>'utf-8');
}

if ($post_error)
{
    print $q->start_html("Error posting on forum"),
	$q->h1("Error posting on forum"), "\n",
	$q->p($post_error), "\n",
	$q->end_html;
    exit;
}

# Generate the UUID QR barcode to display in the SVG image.

my $OUT_SVG = `mktemp /tmp/out.svg.XXXXXXXXXX`;
chomp $OUT_SVG;
my $PREFIX = '/home/andrew/.www/genqruuid';
$ENV{LD_LIBRARY_PATH} = "$ENV{LD_LIBRARY_PATH}:$PREFIX/lib";
$ENV{PATH} = "$ENV{PATH}:$PREFIX/bin";
system("genqruuid.sh -u $q_uuid -o $OUT_SVG");
my $BARCODE_TXT = `cat $OUT_SVG | sed -e '1,4d' -e '/<\\/svg>/d'`;
chomp $BARCODE_TXT;
unlink $OUT_SVG;
push @subst_list, ['TEMPLATE-qw_qr_barcode', $BARCODE_TXT];

if ($show_dummy) {
    print $q->start_html("Successful image upload"),
        $q->h1("Successful image upload"), "\n",
        $q->p("Your image has been uploaded."), "\n";
}

my $ftmpl;
open($ftmpl, "<", "dtemplate.svg");
exec_subst([ @subst_list ], $ftmpl, *STDOUT);
close($ftmpl);

if ($show_dummy) {
    print $q->end_html;
}
