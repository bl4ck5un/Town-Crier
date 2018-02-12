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
# Based on NIST gcmEncryptIntIVxxx.rsp validation files
# Only first 3 of every set used for compile time saving

use strict;

my $file = shift;

open(TEST_DATA, "$file") or die "Opening test cases '$file': $!";

sub get_suite_val($)
{
    my $name = shift;
    my $val = "";

    while(my $line = <TEST_DATA>)
    {
        next if ($line !~ /^\[/);
        ($val) = ($line =~ /\[$name\s\=\s(\w+)\]/);
        last;
    }

    return $val;
}

sub get_val($)
{
    my $name = shift;
    my $val = "";
    my $line;

    while($line = <TEST_DATA>)
    {
        next if($line !~ /=/);
        last;
    }

    ($val) = ($line =~ /^$name = (\w+)/);

    return $val;
}

my $cnt = 1;;
while (my $line = <TEST_DATA>)
{
    my $key_len = get_suite_val("Keylen");
    next if ($key_len !~ /\d+/);
    my $iv_len = get_suite_val("IVlen");
    my $pt_len = get_suite_val("PTlen");
    my $add_len = get_suite_val("AADlen");
    my $tag_len = get_suite_val("Taglen");

    for ($cnt = 0; $cnt < 3; $cnt++)
    {
        my $Count = get_val("Count");
        my $key = get_val("Key");
        my $pt = get_val("PT");
        my $add = get_val("AAD");
        my $iv = get_val("IV");
        my $ct = get_val("CT");
        my $tag = get_val("Tag");

        print("GCM NIST Validation (AES-$key_len,$iv_len,$pt_len,$add_len,$tag_len) #$Count\n");
        print("gcm_encrypt_and_tag");
        print(":\"$key\"");
        print(":\"$pt\"");
        print(":\"$iv\"");
        print(":\"$add\"");
        print(":\"$ct\"");
        print(":$tag_len");
        print(":\"$tag\"");
        print(":0");
        print("\n\n");
    }
}

print("GCM Selftest\n");
print("gcm_selftest:\n\n");

close(TEST_DATA);
