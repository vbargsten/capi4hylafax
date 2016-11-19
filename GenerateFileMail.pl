#! /usr/bin/perl -w
#
# This Script is called by sample_faxrcvd to generate
# a mail with an attached file.
#
# It's based on the hylafax-sample from
# Noel Burton-Krahn <noel@burton-krahn.com>
# (see www.hylafax.org / howto)
#
use strict;
my($fromaddr, $toaddr, $subject, $text, $textdesc, $file, $filemime, $virtfilename, $filedesc) = @ARGV;
my($boundary);

$boundary=join('---',
	       "=Boundary=",
	       $$,
	       sprintf('%x', rand(0xffffffff)));

print <<EOF
From: $fromaddr
To: $toaddr
Subject: $subject
Mime-Version: 1.0
Content-Type: Multipart/Mixed; Boundary=\"$boundary\"
Content-Transfer-Encoding: 7bit

This is a multi-part message in MIME format.

--$boundary
Content-Type: text/plain; charset=us-ascii
Content-Description: $textdesc
Content-Transfer-Encoding: 7bit

$text


EOF
;
print <<EOF
--$boundary
Content-Type: $filemime; name="$virtfilename"
Content-Description: $filedesc
Content-Transfer-Encoding: base64
Content-Disposition: attachment; filename="$virtfilename"

EOF
;

open(IN, "mimencode \"$file\" |") || die ("couldn't mimencode $file: $!");
print while(<IN>);
close(IN) || die("mimencode: $?");

print <<EOF
--$boundary--
EOF
