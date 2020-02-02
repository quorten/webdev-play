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
my $show_dummy = 1;
# If set, do not rev global serial number.
my $no_rev = 0;
# Allow updating existing files?
my $allow_update = 0;

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

my $qa_lochome = $q->param("qa-lochome");
my $qa_education = $q->param("qa-edu");
my $qa_employ = $q->param("qa-employ");
my $qa_crimes = $q->param("qa-crimes");
my $qa_exid_txt = $q->param('qa-exid-txt');

if ($q_mode eq 'create') {
    $allow_update = 0;
} elsif ($q_mode eq 'edit') {
    $allow_update = 1;
}

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

# We must escape newlines and backslashes to store the biography in
# our simple-minded data format.
my $q_enc_bio = encode_tmpl($q_biography);
my $q_enc_edu = encode_tmpl($qa_education);
my $q_enc_employ = encode_tmpl($qa_employ);
my $q_enc_crimes = encode_tmpl($qa_crimes);
my $q_enc_exid_txt = encode_tmpl($qa_exid_txt);

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

# If photo filename was not provided, use 'photo.jpg'.
if (!$q_photo) {
    $q_photo = 'photo.jpg';
}

# If ID number was not provided, generate it.
my $qw_g_serial = -1;
if (!$q_idnum) {
    $qw_g_serial = `cat qw_g_serial.txt`;
    chomp $qw_g_serial;
    $q_idnum = gen_qw_id_num($qw_g_serial);
}

# If UUID was not provided, generate it.
if (!$q_uuid) {
    $q_uuid = `cat /proc/sys/kernel/random/uuid | \
          tr '[[:lower:]]' '[[:upper:]]' | sed -e 's/^/UUID:/g'`;
    chomp $q_uuid;
}

# Fill in the issue date to today if it has not been provided.
if (!$q_issued) {
    $q_issued = strftime "%F %T", localtime $^T;
}

# Only display the date, and not the time, for "issued on."
my @pieces = split(/ /, $q_issued);
my $q_disp_issued = @pieces[0];
@pieces = split(/ /, $q_dob);
my $q_disp_dob = @pieces[0];
$q_disp_dob = "@pieces[0] @pieces[1]" if (@pieces[0] eq 'circa');
@pieces = split(/ /, $q_nat_on);
my $q_disp_nat_on = @pieces[0];
$q_disp_nat_on = "@pieces[0] @pieces[1]" if (@pieces[0] eq 'circa');

# Fill in default value for "Expires" if not given.
if (!$q_expires) {
    $q_expires = 'Never';
}

my @data_recs = (
    ['qw_photo', $q_photo],
    ['qw_name', $q_name],
    ['qw_idnum', $q_idnum],
    ['qw_uuid', $q_uuid],
    ['qw_issued', $q_issued],
    ['qw_dob', $q_dob],
    ['qw_nat_on', $q_nat_on],
    ['qw_expires', $q_expires],
    ['qwb_custom', $qb_custom],
    ['qwb_full_name', $qb_full_name],
    ['qwb_height', $qb_height],
    ['qwb_weight', $qb_weight],
    ['qwb_fur_color', $qb_fur_color],
    ['qwb_eye_color', $qb_eye_color],
    ['qwb_born_at', $qb_born_at],
    ['qw_biography', $q_enc_bio],
    ['qw_bio_custom', $q_bio_custom],
    ['qw_lochome', $qa_lochome],
    ['qw_education', $q_enc_edu],
    ['qw_employment', $q_enc_employ],
    ['qw_crimes', $q_enc_crimes],
    ['qw_exid_txt', $q_enc_exid_txt],
);

my @subst_list = (
    ['TEMPLATE-qw_photo', $q_photo],
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
    ['TEMPLATE-qw_lochome', $qa_lochome],
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

if (!$post_error && !$allow_update && -e $filename)
{
    $post_error = "A file of the upload image filename already exists.";
}

my $fout;

if (!$post_error && !open($fout, ">", $filename))
{
    $post_error = "Could not create upload image filename on server: $!";
}
chmod(0664, $fout);

if (!$post_error)
{
    write_subst([ @data_recs ], $fout);
    close($fout);
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

if (!$no_rev && $qw_g_serial != -1) {
    # Update the global largest ID serial number.
    $qw_g_serial += 1;
    my $fser;
    open($fser, ">", "qw_g_serial.txt");
    print $fser "$qw_g_serial\n";
    close($fser);
}

if ($show_dummy) {
    print $q->start_html("Successful image upload"),
        $q->h1("Successful image upload"), "\n",
        $q->p("Your image has been uploaded."), "\n",
        $q->p($q->a({href=>"gen_index.cgi"}, "Proceed to index page")), "\n";
}

my $ftmpl;
open($ftmpl, "<", "dtemplate.svg");
exec_subst([ @subst_list ], $ftmpl, *STDOUT);
close($ftmpl);

if ($show_dummy) {
    print $q->end_html;
}
