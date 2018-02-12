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
#

use strict;

my ($include_dir, $data_dir, $feature_file);

if( @ARGV ) {
    die "Invalid number of arguments" if scalar @ARGV != 3;
    ($include_dir, $data_dir, $feature_file) = @ARGV;

    -d $include_dir or die "No such directory: $include_dir\n";
    -d $data_dir or die "No such directory: $data_dir\n";
} else {
    $include_dir = 'include/mbedtls';
    $data_dir = 'scripts/data_files';
    $feature_file = 'library/version_features.c';

    unless( -d $include_dir && -d $data_dir ) {
        chdir '..' or die;
        -d $include_dir && -d $data_dir
            or die "Without arguments, must be run from root or scripts\n"
    }
}

my $feature_format_file = $data_dir.'/version_features.fmt';

my @sections = ( "System support", "mbed TLS modules",
                 "mbed TLS feature support" );

my $line_separator = $/;
undef $/;

open(FORMAT_FILE, "$feature_format_file") or die "Opening feature format file '$feature_format_file': $!";
my $feature_format = <FORMAT_FILE>;
close(FORMAT_FILE);

$/ = $line_separator;

open(CONFIG_H, "$include_dir/config.h") || die("Failure when opening config.h: $!");

my $feature_defines = "";
my $in_section = 0;

while (my $line = <CONFIG_H>)
{
    next if ($in_section && $line !~ /#define/ && $line !~ /SECTION/);
    next if (!$in_section && $line !~ /SECTION/);

    if ($in_section) {
        if ($line =~ /SECTION/) {
            $in_section = 0;
            next;
        }

        my ($define) = $line =~ /#define (\w+)/;
        $feature_defines .= "#if defined(${define})\n";
        $feature_defines .= "    \"${define}\",\n";
        $feature_defines .= "#endif /* ${define} */\n";
    }

    if (!$in_section) {
        my ($section_name) = $line =~ /SECTION: ([\w ]+)/;
        my $found_section = grep $_ eq $section_name, @sections;

        $in_section = 1 if ($found_section);
    }
};

$feature_format =~ s/FEATURE_DEFINES\n/$feature_defines/g;

open(ERROR_FILE, ">$feature_file") or die "Opening destination file '$feature_file': $!";
print ERROR_FILE $feature_format;
close(ERROR_FILE);
