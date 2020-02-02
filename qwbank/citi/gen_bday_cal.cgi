#! /bin/sh

echo 'Content-Type: text/plain'
echo ''

grep qw_dob records/*.txt | sed -e 's/records\///g' -e 's/\.txt//g' -e 's/circa //g' | awk 'BEGIN { FS = ":"; } { print $3, $1 }' | sed -e 's/^[0-9]\{4\}-//g' | sort
