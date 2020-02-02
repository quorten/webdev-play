#! /bin/sh
# Convenient wrapper to ImageMagick `convert' for resizing images.

PREFIX=resize
ROTATE=''
#WIDTH=128
#HEIGHT=96
WIDTH=1280
HEIGHT=960
SIZE=${WIDTH}x${HEIGHT}

ARG_ROT=0 # identity (no rotation)
ARG_ORIENT=0 # landscape orientation

while [ $# -gt 0 ]; do
    case "$1" in
	-d) shift; $PREFIX="$1" ;;
	-a) shift; WIDTH=$1 ;;
	-b) shift; HEIGHT=$1 ;;
	-c) ARG_ROT=1 ;; # rotate counterclockwise
	-w) ARG_ROT=2 ;; # rotate clockwise
	-l) ARG_ORIENT=0 ;; # landscape orientation
	-p) ARG_ORIENT=1 ;; # portrait orientation
    esac
    shift
done

if [ $ARG_ORIENT -eq 0 ]; then
    SIZE=${WIDTH}x${HEIGHT}
fi
if [ $ARG_ORIENT -eq 1 ]; then
    SIZE=${HEIGHT}x${WIDTH}
fi
if [ $ARG_ROT -eq 1 ]; then
    ROTATE='-transpose -flip'
fi
if [ $ARG_ROT -eq 2 ]; then
    ROTATE='-flip -transpose'
fi

mkdir -p "$PREFIX"

# This is weird, but it is required in order to get our desired
# expansion behavior.  Well, at least it is one way that works.
FILES1="`echo *.JPG`"
FILES2="`echo *.jpg`"
FILES=''

if [ "$FILES1" != '*.JPG' ]; then
    FILES="$FILES1"
fi
if [ "$FILES2" != '*.jpg' ]; then
    if [ "$FILES" = '' ]; then
	FILES="$FILES $FILES2"
    else
	FILES="$FILES2"
    fi
fi

# -gamma 2.2
# -gamma 0.454545

# Important!  In ImageMagick versions before 6.7.5, there is a bug
# such that the meanings of RGB and sRGB were swapped.

VERSION=`convert --version | grep ^Version: | cut -d' ' -f3`
V6_5=`echo $VERSION | grep -e '^6\.5'`
V6_6=`echo $VERSION | grep -e '^6\.6'`

if [ -n "$V6_5" -o -n "$V6_6" ]; then
  TO_LINEAR='-set colorspace RGB -colorspace sRGB'
  TO_GAMMA='-set colorspace sRGB -colorspace RGB'
  echo 'Ut oh, entering quirks mode!  sRGB and RGB are swapped.' \
    >/dev/stderr
else
  TO_LINEAR='-set colorspace sRGB -colorspace RGB'
  TO_GAMMA='-set colorspace RGB -colorspace sRGB'
fi

for FILE in $FILES; do
  convert "$FILE" $ROTATE \
    -depth 16 \
    $TO_LINEAR \
    -filter lanczos -resize $SIZE \
    $TO_GAMMA \
    -size $SIZE xc:white +swap -gravity center \
    -composite -quality 80 -sampling-factor 1x1 "$PREFIX/$FILE"
done
