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
# rename identifiers (functions, types, enum constant, etc)
# on upgrades of major version according to a list

use warnings;
use strict;

use utf8;
use open qw(:std utf8);

my $usage = "Usage: $0 [-f datafile] [-s] [--] [filenames...]\n";

(my $datafile = $0) =~ s/rename.pl$/data_files\/rename-1.3-2.0.txt/;
my $do_strings = 0;

while( @ARGV && $ARGV[0] =~ /^-/ ) {
    my $opt = shift;
    if( $opt eq '--' ) {
        last;
    } elsif( $opt eq '-f' ) {
        $datafile = shift;
    } elsif( $opt eq '-s' ) {
        $do_strings = 1; shift;
    } else {
        die $usage;
    }
}

my %subst;
open my $nfh, '<', $datafile or die "Could not read $datafile\n";
my $ident = qr/[_A-Za-z][_A-Za-z0-9]*/;
while( my $line = <$nfh> ) {
    chomp $line;
    my ( $old, $new ) = ( $line =~ /^($ident)\s+($ident)$/ );
    if( ! $old || ! $new ) {
        die "$0: $datafile:$.: bad input '$line'\n";
    }
    $subst{$old} = $new;
}
close $nfh or die;

my $string = qr/"(?:\\.|[^\\"])*"/;
my $space = qr/\s+/;
my $idnum = qr/[a-zA-Z0-9_]+/;
my $symbols = qr/[-!#\$%&'()*+,.\/:;<=>?@[\\\]^_`{|}~]+|"/;

# if we replace inside strings, we don't consider them a token
my $token = $do_strings ?         qr/$space|$idnum|$symbols/
                        : qr/$string|$space|$idnum|$symbols/;

my %warnings;

while( my $filename = shift )
{
    print STDERR "$filename... ";
    if( -d $filename ) { print STDERR "skip (directory)\n"; next }

    open my $rfh, '<', $filename or die;
    my @lines = <$rfh>;
    close $rfh or die;

    my @out;
    for my $line (@lines) {
        if( $line =~ /#include/ ) {
            $line =~ s/polarssl/mbedtls/;
            $line =~ s/POLARSSL/MBEDTLS/;
            push( @out, $line );
            next;
        }

        my @words = ($line =~ /$token/g);
        my $checkline = join '', @words;
        if( $checkline eq $line ) {
            my @new = map { exists $subst{$_} ? $subst{$_} : $_ } @words;
            push( @out, join '', @new );
        } else {
            $warnings{$filename} = [] unless $warnings{$filename};
            push @{ $warnings{$filename} }, $line;
            push( @out, $line );
        }
    }

    open my $wfh, '>', $filename or die;
    print $wfh $_ for @out;
    close $wfh or die;
    print STDERR "done\n";
}

if( %warnings ) {
    print "\nWarning: lines skipped due to unexpected characters:\n";
    for my $filename (sort keys %warnings) {
        print "in $filename:\n";
        print for @{ $warnings{$filename} };
    }
}
