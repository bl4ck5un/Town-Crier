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
# Tune the configuration file

use warnings;
use strict;

my $usage = <<EOU;
$0 [-f <file>] unset <name>
$0 [-f <file>] set <name> [<value>]
EOU
# for our eyes only:
# $0 [-f <file>] full|realfull

# Things that shouldn't be enabled with "full".
# Notes:
# - MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3 and
#   MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION could be enabled if the
#   respective tests were adapted
my @excluded = qw(
MBEDTLS_DEPRECATED_REMOVED
MBEDTLS_HAVE_SSE2
MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
MBEDTLS_ECP_DP_M221_ENABLED
MBEDTLS_ECP_DP_M383_ENABLED
MBEDTLS_ECP_DP_M511_ENABLED
MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
MBEDTLS_NO_PLATFORM_ENTROPY
MBEDTLS_REMOVE_ARC4_CIPHERSUITES
MBEDTLS_SSL_HW_RECORD_ACCEL
MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3
MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
MBEDTLS_ZLIB_SUPPORT
MBEDTLS_PKCS11_C
_ALT\s*$
);

# Things that should be enabled in "full" even if they match @excluded
my @non_excluded = qw(
PLATFORM_[A-Z0-9]+_ALT
);

my $config_file = "include/mbedtls/config.h";

# get -f option
if (@ARGV >= 2 && $ARGV[0] eq "-f") {
    shift; # -f
    $config_file = shift;

    -f $config_file or die "No such file: $config_file\n";
} else {
    if (! -f $config_file)  {
        chdir '..' or die;
        -f $config_file
            or die "Without -f, must be run from root or scripts\n"
    }
}

# get action
die $usage unless @ARGV;
my $action = shift;

my ($name, $value);
if ($action eq "full" || $action eq "realfull") {
    # nothing to do
} elsif ($action eq "unset") {
    die $usage unless @ARGV;
    $name = shift;
} elsif ($action eq "set") {
    die $usage unless @ARGV;
    $name = shift;
    $value = shift if @ARGV;
} else {
    die $usage;
}
die $usage if @ARGV;

open my $config_read, '<', $config_file or die "read $config_file: $!\n";
my @config_lines = <$config_read>;
close $config_read;

my ($exclude_re, $no_exclude_re);
if ($action eq "realfull") {
    $exclude_re = qr/^$/;
    $no_exclude_re = qr/./;
} else {
    $exclude_re = join '|', @excluded;
    $no_exclude_re = join '|', @non_excluded;
}

open my $config_write, '>', $config_file or die "write $config_file: $!\n";

my $done;
for my $line (@config_lines) {
    if ($action eq "full" || $action eq "realfull") {
        if ($line =~ /name SECTION: Module configuration options/) {
            $done = 1;
        }

        if (!$done && $line =~ m!^//\s?#define! &&
                ( $line !~ /$exclude_re/ || $line =~ /$no_exclude_re/ ) ) {
            $line =~ s!^//\s?!!;
        }
        if (!$done && $line =~ m!^\s?#define! &&
                ! ( $line !~ /$exclude_re/ || $line =~ /$no_exclude_re/ ) ) {
            $line =~ s!^!//!;
        }
    } elsif ($action eq "unset") {
        if (!$done && $line =~ /^\s*#define\s*$name\b/) {
            $line = '//' . $line;
            $done = 1;
        }
    } elsif (!$done && $action eq "set") {
        if ($line =~ m!^(?://)?\s*#define\s*$name\b!) {
            $line = "#define $name";
            $line .= " $value" if defined $value && $value ne "";
            $line .= "\n";
            $done = 1;
        }
    }

    print $config_write $line;
}

close $config_write;

die "configuration section not found" if ($action eq "full" && !$done);
die "$name not found" if ($action ne "full" && !$done);

__END__
