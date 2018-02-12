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
# Find functions making recursive calls to themselves.
# (Multiple recursion where a() calls b() which calls a() not covered.)
#
# When the recursion depth might depend on data controlled by the attacker in
# an unbounded way, those functions should use interation instead.
#
# Typical usage: scripts/recursion.pl library/*.c

use warnings;
use strict;

use utf8;
use open qw(:std utf8);

# exclude functions that are ok:
# - mpi_write_hlp: bounded by size of mbedtls_mpi, a compile-time constant
# - x509_crt_verify_child: bounded by MBEDTLS_X509_MAX_INTERMEDIATE_CA
my $known_ok = qr/mpi_write_hlp|x509_crt_verify_child/;

my $cur_name;
my $inside;
my @funcs;

die "Usage: $0 file.c [...]\n" unless @ARGV;

while (<>)
{
    if( /^[^\/#{}\s]/ && ! /\[.*]/ ) {
        chomp( $cur_name = $_ ) unless $inside;
    } elsif( /^{/ && $cur_name ) {
        $inside = 1;
        $cur_name =~ s/.* ([^ ]*)\(.*/$1/;
    } elsif( /^}/ && $inside ) {
        undef $inside;
        undef $cur_name;
    } elsif( $inside && /\b\Q$cur_name\E\([^)]/ ) {
        push @funcs, $cur_name unless /$known_ok/;
    }
}

print "$_\n" for @funcs;
exit @funcs;
