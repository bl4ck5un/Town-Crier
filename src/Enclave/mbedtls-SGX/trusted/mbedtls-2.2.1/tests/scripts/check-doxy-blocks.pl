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
# Detect comment blocks that are likely meant to be doxygen blocks but aren't.
#
# More precisely, look for normal comment block containing '\'.
# Of course one could use doxygen warnings, eg with:
#   sed -e '/EXTRACT/s/YES/NO/' doxygen/mbedtls.doxyfile | doxygen -
# but that would warn about any undocumented item, while our goal is to find
# items that are documented, but not marked as such by mistake.

use warnings;
use strict;
use File::Basename;

# C/header files in the following directories will be checked
my @directories = qw(include/mbedtls library doxygen/input);

# very naive pattern to find directives:
# everything with a backslach except '\0' and backslash at EOL
my $doxy_re = qr/\\(?!0|\n)/;

sub check_file {
    my ($fname) = @_;
    open my $fh, '<', $fname or die "Failed to open '$fname': $!\n";

    # first line of the last normal comment block,
    # or 0 if not in a normal comment block
    my $block_start = 0;
    while (my $line = <$fh>) {
        $block_start = $.   if $line =~ m/\/\*(?![*!])/;
        $block_start = 0    if $line =~ m/\*\//;
        if ($block_start and $line =~ m/$doxy_re/) {
            print "$fname:$block_start: directive on line $.\n";
            $block_start = 0; # report only one directive per block
        }
    }

    close $fh;
}

sub check_dir {
    my ($dirname) = @_;
    for my $file (<$dirname/*.[ch]>) {
        check_file($file);
    }
}

# locate root directory based on invocation name
my $root = dirname($0) . '/..';
chdir $root or die "Can't chdir to '$root': $!\n";

# just do it
for my $dir (@directories) {
    check_dir($dir)
}

__END__
