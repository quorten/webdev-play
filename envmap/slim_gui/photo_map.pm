# Common functions, written in Perl, for implementing the photo
# mapping system.  This mainly covers generating links and programmed
# constraints that the photo structures must follow.

package photo_map;

use strict;
use warnings;

BEGIN {
    use Exporter ();
    use Cwd;
    our @ISA = qw(Exporter);
    our @EXPORT = qw($MIN_FILE_IDX
                     $FS_PREFIX
                     $SERVER_ROOT
                     &search_re_num_file
                     &gen_prev_next_links
                     &gen_outer_link
                     &gen_dir_links
                     &gen_view_links
                     &gen_empty_user_links
                     &gen_default_links);
}

# TODO ADD NEW MODULE: So you want to display thumbnails?  Add a
# "search query" module where we select a subset of images to display.
# Then for slideshows, we first do a search query, then play back the
# slide show, generating the link pages dynamically.

# TODO NOTE: Take note of "image sublinks" where you link to a region
# of an image.  What should you do?  Highlight the object being
# linked.  Maybe yes, but there's a better way when you think about
# search.  Crop the image to the linked subject.  Overlays for
# non-rectangular region highlighting are now optional at this point,
# but probably recommended.

# Our application assumes minimum index numbers of one, but who knows,
# a Unixy person might want to change this to zero...
our $MIN_FILE_IDX = 1;

our $FS_PREFIX = "/home/andrew/laser-test/envmap";
our $SERVER_ROOT = "/~andrew/envmap-poc/envmap";

# If applicable, dereference a symlink file and return a relative path
# to the target file.  Otherwise, return the original relative path.
# Arguments:
#   1. string, filesystem path to directory to use
#   1. string, path from directory to potential symlink
# Returns:
#   string, the relative path to the original file
sub deref_symlink_to_rel
{
    my $dir = shift;
    my $file = shift;
    my $path = "$dir/$file";
    my $target;
    if (-l $path) {
	# Use Win32::Shortcut to handle Windows desktop links.  This
	# is the alternative for Windows users since Windows doesn't
	# support symlinks from the GUI level.
	$target = readlink($path);
    } else {
	$target = $path;
    }
    return $target;
}

# For file names with numbers, numbers must be unsigned integers.

# Search for the smallest numbered file matching a regular expression
# and a number.
# Arguments:
#   1. list, list of directory entries
#   2. regular expression, regular expression to use
# Returns:
#   string, the name of the matching file or '' (empty string) if
#     there was no match
sub search_re_num_file_min
{
    my $entries = shift;
    my $file_re = shift;
    my $tgt_num = 0;
    my $tgt_name = '';
    for my $entry (@$entries) {
	if ($entry =~ /$file_re/) {
	    if (length($tgt_name) == 0 or $1 < $tgt_num) {
		$tgt_num = $1;
		$tgt_name = $entry;
	    }
	}
    }
    return $tgt_name;
}

# Search for the largest numbered file matching a regular expression
# and a number.
# Arguments:
#   1. list, list of directory entries
#   2. regular expression, regular expression to use
# Returns:
#   string, the name of the matching file or '' (empty string) if
#     there was no match
sub search_re_num_file_max
{
    my $entries = shift;
    my $file_re = shift;
    my $tgt_num = 0;
    my $tgt_name = '';
    for my $entry (@$entries) {
	if ($entry =~ /$file_re/) {
	    if (length($tgt_name) == 0 or $1 > $tgt_num) {
		$tgt_num = $1;
		$tgt_name = $entry;
	    }
	}
    }
    return $tgt_name;
}

# Search for a file that matches a regular expression and a number.
# Returns the matching filename, otherwise returns an empty string.
# Arguments:
#   1. list, list of directory entries
#   2. regular expression, regular expression to use
#   3. integer, number to use
# Returns:
#   string, the name of the matching file or '' (empty string) if
#     there was no match
sub search_re_num_file
{
    my $entries = shift;
    my $file_re = shift;
    my $file_num = shift;
    for my $entry (@$entries) {
	if ($entry =~ /$file_re/ and $1 == $file_num) {
	    return $entry;
	}
    }
    return '';
}

# Search for the largest numbered file less than a given number,
# matching a regular expression and a number.
#   1. list, list of directory entries
#   2. regular expression, regular expression to use
#   3. integer, number to use
# Returns:
#   string, the name of the matching file or '' (empty string) if
#     there was no match
sub search_re_num_file_max_lt
{
    my $entries = shift;
    my $file_re = shift;
    my $file_num = shift;
    my $tgt_num = 0;
    my $tgt_name = '';
    for my $entry (@$entries) {
	if ($entry =~ /$file_re/ and $1 < $file_num) {
	    if (length($tgt_name) == 0 or $1 > $tgt_num) {
		$tgt_num = $1;
		$tgt_name = $entry;
	    }
	}
    }
    return $tgt_name;
}

# Search for the smallest numbered file greater than a given number,
# matching a regular expression and a number.
sub search_re_num_file_min_gt
{
    my $entries = shift;
    my $file_re = shift;
    my $file_num = shift;
    my $tgt_num = 0;
    my $tgt_name = '';
    for my $entry (@$entries) {
	if ($entry =~ /$file_re/ and $1 > $file_num) {
	    if (length($tgt_name) == 0 or $1 < $tgt_num) {
		$tgt_num = $1;
		$tgt_name = $entry;
	    }
	}
    }
    return $tgt_name;
}

# TODO FIXME: Do not make assumptions about L1 and A1 names when
# searching.  Find the lowest number in the directory and use that.

# NOTE: This function requires that paths be relative to the current
# working directory so that file existence tests may be performed.

# Generate previous and next links.
# Arguments:
#   1. string, path to parent directory of entries
#   2. list, list of directory entries
#   3. regular expression, regular expression to use
#   4. integer, number of current directory
# Returns:
#   list, (prev_link, next_link)
#     If one direction cannot be navigated, an empty string is
#     returned for that link.
sub gen_prev_next_links
{
    my $parent = shift;
    my $entries = shift;
    my $file_re = shift;
    my $file_num = shift;

    my $prev_idx = $file_num - 1;
    my $next_idx = $file_num + 1;
    my $str_prev = '';
    my $str_next = '';
    if ($prev_idx >= $MIN_FILE_IDX) {
	$str_prev = search_re_num_file($entries, $file_re, $prev_idx);
	if (length($str_prev) > 0) {
	    $str_prev = $parent . $str_prev;
	}
	if (-e "$str_prev/A1") {
	    $str_prev .= '/A1';
	}
    }
    $str_next = search_re_num_file($entries, $file_re, $next_idx);
    if (length($str_next) > 0) {
	$str_next = $parent . $str_next;
    }
    if (-e "$str_next/A1") {
	$str_next .= '/A1';
    }
    return ($str_prev, $str_next);
}

# TODO FIXME: Do not make assumptions about L1 and A1 names when
# searching.  Find the lowest number in the directory and use that.

# TODO FIXME: Add a flag for "GUI mode" where we link back to first
# angles to differentiate from "physical mode" where we link directly
# to the outer container.

# Generate a relative path link for the "outer container."  This will
# always go somewhere except at the root.
# Returns:
#   string, path link or '' if none.
sub gen_outer_link
{
    my $cwd = getcwd();
    my @segs = split(/\//, $cwd);
    my $outer_link = '';

    # $outer_link = '..';

    # In the simple case that we assume the user generated symlinks so
    # that all path segments resolve, '..' will always resolve to a
    # valid parent.  However, since we want to support our application
    # on filesystems without symlink support, we include full code for
    # handling the directory nesting cases here.

    # If we are in an angle, change to Angle 1 and stop there.
    if ($segs[-1] =~ /^A(\d+)$/ and $1 != $MIN_FILE_IDX) {
	$outer_link = '../A1';
    } else {
	my $last_seg = -1;

	# If we are already at Angle 1 or not within an angle,
	# continue to change to the parent layer/entity.
	$outer_link = '..';
	if ($segs[$last_seg] =~ /^A\d+$/) {
	    $outer_link .= '/..';
	    $last_seg--;
	}

	# If we are within a layer and changing to the parent, change
	# to Layer 1.  If we are already at Layer 1, change to the
	# parent layer/entity.  Note that the entity containing Layer
	# 1 is a synonym for the Layer 1 photo map.
	if ($segs[$last_seg] =~ /^L(\d+)$/) {
	    if ($1 != $MIN_FILE_IDX) {
		$outer_link .= '/L1';
	    } else {
		$outer_link .= '/..';

		# Check for layers within the destination thus far.
		# If there are layers, change to Layer 1 within the
		# new entity.  (Note that we only perform this check
		# within this code path because layers cannot be
		# directly nested.)
		opendir(my $dh, $outer_link) || die "can't opendir: $!";
		my @entries = readdir($dh);
		closedir($dh);
		for my $entry (@entries) {
		    if ($entry =~ /^L\d+$/) {
			$outer_link .= '/L1';
			last;
		    }
		}
	    }
	}

	# Check for angles within the destination thus far.  If there
	# are angles, change to Angle 1 within the new entity.
	opendir(my $dh, $outer_link) || die "can't opendir: $!";
	my @entries = readdir($dh);
	closedir($dh);
	for my $entry (@entries) {
	    if ($entry =~ /^A\d+$/) {
		$outer_link .= '/A1';
		last;
	    }
	}
    }
    return $outer_link;
}

# Generate directory links for our template files.  Returns the
# substitutions list.
sub gen_dir_links
{
    my @subst_list = ();

    # Get the current working directory and do a sanity check on it.

    my $cwd = getcwd();

    if (index($cwd, $FS_PREFIX) != 0) {
	die "Current working directory is outside of server root.";
    }

    # Generate some meta-fields specifying the path to the server root
    # and the current working directory.

    my @segs = split(/\//, $cwd);
    my @segs_fs_prefix = split(/\//, $FS_PREFIX);
    my $num_rel_segs = $#segs - $#segs_fs_prefix;
    my $rel_root = '';
    $rel_root .= '../' x ($num_rel_segs - 1) if ($num_rel_segs >= 2);
    $rel_root .= '..' if ($num_rel_segs >= 1);
    my $server_cwd = $SERVER_ROOT . substr($cwd, length($FS_PREFIX));

    my $basename = pop(@segs);
    my $dirname = join('/', @segs);
    # Restore `@segs' so that we can use it later.
    push(@segs, $basename);

    push @subst_list, [ 'TEMPLATE-abs_root', $SERVER_ROOT ];
    push @subst_list, [ 'TEMPLATE-rel_root', $rel_root ];
    push @subst_list, [ 'TEMPLATE-cwd', $server_cwd ];

    # Get directory entries of the previous two parents so that we can
    # generate previous/next layers/angles.

    opendir(my $dh, '..') || die "can't opendir: $!";
    my @p1_entries = readdir($dh);
    closedir($dh);
    opendir($dh, '../..') || die "can't opendir: $!";
    my @p2_entries = readdir($dh);
    closedir($dh);

    # Generate a relative path link for the "outer container."  This
    # will always go somewhere except at the root.

    my $outer_link = '';
    if (index($dirname, $FS_PREFIX) == 0 and $segs[-1] !~ /^T\d+$/) {
	$outer_link = gen_outer_link();
    }
    push @subst_list, [ 'TEMPLATE-outer', $outer_link ];

    # Generate layers, if applicable.

    my $layer_idx = 0;
    my $layer_parent = '';
    my @layer_entries = ();
    if ($segs[-1] =~ /^L\d+$/) {
	$layer_idx = $segs[-1];
	$layer_parent = '../';
	@layer_entries = @p1_entries;
    } elsif ($segs[-1] =~ /^A\d+$/ and $segs[-2] =~ /^L\d+$/) {
	$layer_idx = $segs[-2];
	$layer_parent = '../../';
	@layer_entries = @p2_entries;
    }
    $layer_idx =~ s/^L//;
    my @layer_links = gen_prev_next_links($layer_parent, \@layer_entries,
					  qr/^L(\d+)$/, $layer_idx);

    push @subst_list, [ 'TEMPLATE-layer_up', $layer_links[0] ];
    push @subst_list, [ 'TEMPLATE-layer_down', $layer_links[1] ];

    # Generate angles, if applicable.

    my $angle_idx = 0;
    my $angle_parent = '';
    my @angle_entries = ();
    if ($segs[-1] =~ /^A\d+$/) {
	$angle_idx = $segs[-1];
	$angle_parent = '../';
	@angle_entries = @p1_entries;
    }
    $angle_idx =~ s/^A//;
    my @angle_links = gen_prev_next_links($angle_parent, \@angle_entries,
					  qr/^A(\d+)$/, $angle_idx);

    push @subst_list, [ 'TEMPLATE-angle_prev', $angle_links[0] ];
    push @subst_list, [ 'TEMPLATE-angle_next', $angle_links[1] ];

    return @subst_list;
}

# Generate view links for our template files.  Takes a single integer
# argument `view_mode', and returns the substitutions list.
#
# 0: Photo view
# 1: Vector view
# 2: Text view
# 4: Flag: Overlays off if set, on if clear
sub gen_view_links
{
    my $view_mode = shift;
    my @subst_list = ();

    my $overlay_name;
    my $other_overlay_name;
    if ($view_mode & 0x4) {
	# Overlays off
	$overlay_name = '_map';
	$other_overlay_name = '_overlay';
    } else {
	# Overlays on
	$overlay_name .= '_overlay';
	$other_overlay_name = '_map';
    }

    my $view_name;
    my $vector_toggle_name;
    my $overlay_toggle_name;
    if (($view_mode & (0x4 - 1)) == 0) {
	$view_name = 'photo' . $overlay_name . '.html';
	$vector_toggle_name = 'vector' . $overlay_name . '.html';
	$overlay_toggle_name = 'photo' . $other_overlay_name . '.html';
    } elsif (($view_mode & (0x4 - 1)) == 1) {
	$view_name = 'vector' . $overlay_name . '.html';
	$vector_toggle_name = 'photo' . $overlay_name . '.html';
	$overlay_toggle_name = 'vector' . $other_overlay_name . '.html';
    } elsif (($view_mode & (0x4 - 1)) == 2) {
	$view_name = 'text.html';
	$vector_toggle_name = 'vector_overlay.html';
	$overlay_toggle_name = 'text.html';
    }

    # TODO FIXME: SVG bungle here, idealism versus pragmatism.
    push @subst_list, [ 'TEMPLATE-svg_cur_view', "photo_overlay.svg" ];
    push @subst_list, [ 'TEMPLATE-cur_view', $view_name ];
    push @subst_list, [ 'TEMPLATE-text_view', 'text.html' ];
    push @subst_list, [ 'TEMPLATE-vector_view', $vector_toggle_name ];
    push @subst_list, [ 'TEMPLATE-overlays', $overlay_toggle_name ];
    push @subst_list, [ 'TEMPLATE-thumbnails', '' ];
    push @subst_list, [ 'TEMPLATE-3d_view', '' ];

    return @subst_list;
}

# Generate empty links for the user substitutions.
sub gen_empty_user_links
{
    my @subst_list = (
	[ 'TEMPLATE-linked_data', '' ],
	[ 'TEMPLATE-raw_scan', '' ],
	[ 'TEMPLATE-dig_positive', '' ],
	[ 'TEMPLATE-step_back', '' ],
	[ 'TEMPLATE-step_fwd', '' ]
    );

    return @subst_list;
}

# Generate default links in the case that the user does not supply
# any.
# Arguments:
#   integer, `view_mode' as described in` gen_view_links()'.
# Returns:
#    list, substitutions list
sub gen_default_links
{
    my $view_mode = shift;
    my @subst_list  = ();

    push @subst_list, gen_dir_links();
    push @subst_list, gen_view_links($view_mode);
    push @subst_list, gen_empty_user_links();

    return @subst_list;
}

1;
