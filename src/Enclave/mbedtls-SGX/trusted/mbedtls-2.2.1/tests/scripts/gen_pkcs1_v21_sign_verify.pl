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

my $file = shift;

open(TEST_DATA, "$file") or die "Opening test cases '$file': $!";

sub get_val($$)
{
    my $str = shift;
    my $name = shift;
    my $val = "";

    while(my $line = <TEST_DATA>)
    {
        next if($line !~ /^# $str/);
        last;
    }
        
    while(my $line = <TEST_DATA>)
    {
        last if($line eq "\r\n"); 
        $val .= $line;
    }

    $val =~ s/[ \r\n]//g;

    return $val;
}

my $state = 0;
my $val_n = "";
my $val_e = "";
my $val_p = "";
my $val_q = "";
my $mod = 0;
my $cnt = 1;
while (my $line = <TEST_DATA>)
{
    next if ($line !~ /^# Example/);

    ( $mod ) = ($line =~ /A (\d+)/);
    $val_n = get_val("RSA modulus n", "N");
    $val_e = get_val("RSA public exponent e", "E");
    $val_p = get_val("Prime p", "P");
    $val_q = get_val("Prime q", "Q");

    for(my $i = 1; $i <= 6; $i++)
    {
        my $val_m = get_val("Message to be", "M");
        my $val_salt = get_val("Salt", "Salt");
        my $val_sig = get_val("Signature", "Sig");

        print("RSASSA-PSS Signature Example ${cnt}_${i}\n");
        print("pkcs1_rsassa_pss_sign:$mod:16:\"$val_p\":16:\"$val_q\":16:\"$val_n\":16:\"$val_e\":SIG_RSA_SHA1:MBEDTLS_MD_SHA1");
        print(":\"$val_m\"");
        print(":\"$val_salt\"");
        print(":\"$val_sig\":0");
        print("\n\n");

        print("RSASSA-PSS Signature Example ${cnt}_${i} (verify)\n");
        print("pkcs1_rsassa_pss_verify:$mod:16:\"$val_n\":16:\"$val_e\":SIG_RSA_SHA1:MBEDTLS_MD_SHA1");
        print(":\"$val_m\"");
        print(":\"$val_salt\"");
        print(":\"$val_sig\":0");
        print("\n\n");
    } 
    $cnt++;
}
close(TEST_DATA);
