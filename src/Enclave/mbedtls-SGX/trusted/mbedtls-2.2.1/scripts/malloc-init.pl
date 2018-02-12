#!/usr/bin/perl
#
# Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
#
# Permission to use the "TownCrier" software ("TownCrier"), officially
# docketed at the Center for Technology Licensing at Cornell University
# as D-7364, developed through research conducted at Cornell University,
# and its associated copyrights solely for educational, research and
# non-profit purposes without fee is hereby granted, provided that the
# user agrees as follows:
#
# The permission granted herein is solely for the purpose of compiling
# the TowCrier source code. No other rights to use TownCrier and its
# associated copyrights for any other purpose are granted herein,
# whether commercial or non-commercial.
#
# Those desiring to incorporate TownCrier software into commercial
# products or use TownCrier and its associated copyrights for commercial
# purposes must contact the Center for Technology Licensing at Cornell
# University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
# ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
# commercial license.
#
# IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
# INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
# ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
# UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
# ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
# REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
# OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
# OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
# PATENT, TRADEMARK OR OTHER RIGHTS.
#
# TownCrier was developed with funding in part by the National Science
# Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
# CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
# Google Faculty Research Awards, and a VMWare Research Award.
#
# Check for malloc calls not shortly followed by initialisation.
#
# Known limitations:
# - false negative: can't see allocations spanning more than one line
# - possible false negatives, see patterns
# - false positive: malloc-malloc-init-init is not accepted
# - false positives: "non-standard" init functions (eg, the things being
# initialised is not the first arg, or initialise struct members)
#
# Since false positives are expected, the results must be manually reviewed.
#
# Typical usage: scripts/malloc-init.pl library/*.c

use warnings;
use strict;

use utf8;
use open qw(:std utf8);

my $limit = 7;
my $inits = qr/memset|memcpy|_init|fread|base64_..code/;

# cases to bear in mind:
#
# 0. foo = malloc(...); memset( foo, ... );
# 1. *foo = malloc(...); memset( *foo, ... );
# 2. type *foo = malloc(...); memset( foo, ...);
# 3. foo = malloc(...); foo_init( (type *) foo );
# 4. foo = malloc(...); for(i=0..n) { init( &foo[i] ); }
#
# The chosen patterns are a bit relaxed, but unlikely to cause false positives
# in real code (initialising *foo or &foo instead of foo will likely be caught
# by functional tests).
#
my $id = qr/([a-zA-Z-0-9_\->\.]*)/;
my $prefix = qr/\s(?:\*?|\&?|\([a-z_]* \*\))\s*/;

my $name;
my $line;
my @bad;

die "Usage: $0 file.c [...]\n" unless @ARGV;

while (my $file = shift @ARGV)
{
    open my $fh, "<", $file or die "read $file failed: $!\n";
    while (<$fh>)
    {
        if( /mbedtls_malloc\(/ ) {
            if( /$id\s*=.*mbedtls_malloc\(/ ) {
                push @bad, "$file:$line:$name" if $name;
                $name = $1;
                $line = $.;
            } else {
                push @bad, "$file:$.:???" unless /return mbedtls_malloc/;
            }
        } elsif( $name && /(?:$inits)\($prefix\Q$name\E\b/ ) {
            undef $name;
        } elsif( $name && $. - $line > $limit ) {
            push @bad, "$file:$line:$name";
            undef $name;
            undef $line;
        }
    }
    close $fh or die;
}

print "$_\n" for @bad;
