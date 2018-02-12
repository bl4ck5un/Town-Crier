//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

/*
    A Simple CSV Parser
    Inspired by - The Practice of Programming by Rob Pike and Brian Kernighan

    TODO
    Does not handle the embedded line breaks case. Provide support for that
    For instance the following is not yet supported
    1997,Ford,E350,"Go get one now
    they are going fast"
*/

#ifndef __CSV_PARSER_HPP
#define __CSV_PARSER_HPP

// Define your header files here
#include <string>
#include <map>
#include <vector>

// Place to put your constants
#define CSV_DELIMITER ","
#define CSV_QUOTE '"'

// Place for type definitions
typedef std :: string STR;
typedef std :: vector < STR > CSV_FIELDS;
typedef std :: map < STR, STR > KEY_VAL_FIELDS;
typedef std :: pair < STR, STR > MAP_ENTRY;
typedef std :: vector < STR > :: iterator VECTOR_ITR;
typedef std :: vector < STR > :: const_iterator CONST_VECTOR_ITR;
typedef std :: map < STR, STR > :: iterator MAP_ITR;
typedef std :: map < STR, STR > :: const_iterator CONST_MAP_ITR;

class CSV_Parser
{
    public:
        CSV_Parser() {};
        ~CSV_Parser() {};
        bool parse_line(const STR&, CSV_FIELDS&);
        bool parse_line(const STR&, CSV_FIELDS&, KEY_VAL_FIELDS&);
        

    private:
        bool parse(const STR&, CSV_FIELDS&);
        int parse_quoted_fields(const STR&, STR&, int&);
        int parse_normal_fields(const STR&, STR&, int&);
};

#endif
