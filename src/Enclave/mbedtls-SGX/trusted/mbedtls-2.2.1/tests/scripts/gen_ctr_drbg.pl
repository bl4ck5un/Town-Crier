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
# Based on NIST CTR_DRBG.rsp validation file
# Only uses AES-256-CTR cases that use a Derivation function
# and concats nonce and personalization for initialization.

use strict;

my $file = shift;

open(TEST_DATA, "$file") or die "Opening test cases '$file': $!";

sub get_suite_val($)
{
    my $name = shift;
    my $val = "";

    my $line = <TEST_DATA>;
    ($val) = ($line =~ /\[$name\s\=\s(\w+)\]/);

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
    next if ($line !~ /^\[AES-256 use df/);

    my $PredictionResistanceStr = get_suite_val("PredictionResistance");
    my $PredictionResistance = 0;
    $PredictionResistance = 1 if ($PredictionResistanceStr eq 'True');
    my $EntropyInputLen = get_suite_val("EntropyInputLen");
    my $NonceLen = get_suite_val("NonceLen");
    my $PersonalizationStringLen = get_suite_val("PersonalizationStringLen");
    my $AdditionalInputLen = get_suite_val("AdditionalInputLen");

    for ($cnt = 0; $cnt < 15; $cnt++)
    {
        my $Count = get_val("COUNT");
        my $EntropyInput = get_val("EntropyInput");
        my $Nonce = get_val("Nonce");
        my $PersonalizationString = get_val("PersonalizationString");
        my $AdditionalInput1 = get_val("AdditionalInput");
        my $EntropyInputPR1 = get_val("EntropyInputPR") if ($PredictionResistance == 1);
        my $EntropyInputReseed = get_val("EntropyInputReseed") if ($PredictionResistance == 0);
        my $AdditionalInputReseed = get_val("AdditionalInputReseed") if ($PredictionResistance == 0);
        my $AdditionalInput2 = get_val("AdditionalInput");
        my $EntropyInputPR2 = get_val("EntropyInputPR") if ($PredictionResistance == 1);
        my $ReturnedBits = get_val("ReturnedBits");
    
        if ($PredictionResistance == 1)
        {
            print("CTR_DRBG NIST Validation (AES-256 use df,$PredictionResistanceStr,$EntropyInputLen,$NonceLen,$PersonalizationStringLen,$AdditionalInputLen) #$Count\n");
            print("ctr_drbg_validate_pr");
            print(":\"$Nonce$PersonalizationString\"");
            print(":\"$EntropyInput$EntropyInputPR1$EntropyInputPR2\"");
            print(":\"$AdditionalInput1\"");
            print(":\"$AdditionalInput2\"");
            print(":\"$ReturnedBits\"");
            print("\n\n");
        }
        else
        {
            print("CTR_DRBG NIST Validation (AES-256 use df,$PredictionResistanceStr,$EntropyInputLen,$NonceLen,$PersonalizationStringLen,$AdditionalInputLen) #$Count\n");
            print("ctr_drbg_validate_nopr");
            print(":\"$Nonce$PersonalizationString\"");
            print(":\"$EntropyInput$EntropyInputReseed\"");
            print(":\"$AdditionalInput1\"");
            print(":\"$AdditionalInputReseed\"");
            print(":\"$AdditionalInput2\"");
            print(":\"$ReturnedBits\"");
            print("\n\n");
        }
    }
}
close(TEST_DATA);
