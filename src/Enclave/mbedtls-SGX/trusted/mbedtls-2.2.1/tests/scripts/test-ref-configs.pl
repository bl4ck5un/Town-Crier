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
# test standard configurations:
# - build
# - run test suite
# - run compat.sh
#
# Usage: tests/scripts/test-ref-configs.pl [config-name [...]]

use warnings;
use strict;

my %configs = (
    'config-mini-tls1_1.h' => {
        'compat' => '-m tls1_1 -f \'^DES-CBC3-SHA$\|^TLS-RSA-WITH-3DES-EDE-CBC-SHA$\'',
    },
    'config-suite-b.h' => {
        'compat' => "-m tls1_2 -f 'ECDHE-ECDSA.*AES.*GCM' -p mbedTLS",
    },
    'config-picocoin.h' => {
    },
    'config-ccm-psk-tls1_2.h' => {
        'compat' => '-m tls1_2 -f \'^TLS-PSK-WITH-AES-...-CCM-8\'',
    },
    'config-thread.h' => {
        'opt' => '-f ECJPAKE.*nolog',
    },
);

# If no config-name is provided, use all known configs.
# Otherwise, use the provided names only.
if ($#ARGV >= 0) {
    my %configs_ori = ( %configs );
    %configs = ();

    foreach my $conf_name (@ARGV) {
        if( ! exists $configs_ori{$conf_name} ) {
            die "Unknown configuration: $conf_name\n";
        } else {
            $configs{$conf_name} = $configs_ori{$conf_name};
        }
    }
}

-d 'library' && -d 'include' && -d 'tests' or die "Must be run from root\n";

my $config_h = 'include/mbedtls/config.h';

system( "cp $config_h $config_h.bak" ) and die;
sub abort {
    system( "mv $config_h.bak $config_h" ) and warn "$config_h not restored\n";
    die $_[0];
}

while( my ($conf, $data) = each %configs ) {
    system( "cp $config_h.bak $config_h" ) and die;
    system( "make clean" ) and die;

    print "\n******************************************\n";
    print "* Testing configuration: $conf\n";
    print "******************************************\n";

    system( "cp configs/$conf $config_h" )
        and abort "Failed to activate $conf\n";

    system( "make CFLAGS='-Os -Werror'" ) and abort "Failed to build: $conf\n";
    system( "make test" ) and abort "Failed test suite: $conf\n";

    my $compat = $data->{'compat'};
    if( $compat )
    {
        print "\nrunning compat.sh $compat\n";
        system( "tests/compat.sh $compat" )
            and abort "Failed compat.sh: $conf\n";
    }
    else
    {
        print "\nskipping compat.sh\n";
    }

    my $opt = $data->{'opt'};
    if( $opt )
    {
        print "\nrunning ssl-opt.sh $opt\n";
        system( "tests/ssl-opt.sh $opt" )
            and abort "Failed ssl-opt.sh: $conf\n";
    }
    else
    {
        print "\nskipping ssl-opt.sh\n";
    }
}

system( "mv $config_h.bak $config_h" ) and warn "$config_h not restored\n";
system( "make clean" );
exit 0;
