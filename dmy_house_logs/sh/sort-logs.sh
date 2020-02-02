#! /bin/sh

LOGFILE="$1"
# DIR_LOGFILE=`dirname "$LOGFILE"`
# BASE_LOGFILE=`basename "$LOGFILE"`
PATCHFILE="$PWD/`date --iso-8601`-sorted.diff"

sort "$LOGFILE" | diff -u "$LOGFILE" - >"$PATCHFILE"

# Previously we also updated the log file automatically, but I prefer
# not to do that as I tend to catch manual date entry errors after
# sorting, and I need to go back and fix those manually.  Go figure.

# For some reason `patch' does not like `..' directory path prefixes,
# even with single file names and patches, so we must insert this
# weird logic to get to to work in all cases.
# ( cd "$DIR_LOGFILE" && patch "$BASE_LOGFILE" "$PATCHFILE" )
