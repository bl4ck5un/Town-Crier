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

#include "csv_parser.hpp"

// Definitions for the Public methods
bool CSV_Parser :: parse_line(const STR& input_line, CSV_FIELDS& output_fields)
{
    /*
        A public method which accepts the following arguments
        a. A string
        b. A vector of strings

        Parse the CSV line and populate the vector with the output
    */
    bool status;
    status = parse(input_line, output_fields);
    return status;
}

bool CSV_Parser :: parse_line(const STR& input_line, CSV_FIELDS& header_fields, KEY_VAL_FIELDS& key_val)
{
    /*
        A public method which accepts the following arguments
        a. A string
        b. A vector of strings
        c. A map with key and value both being strings

        Parse the CSV line, use the header provided in the vector to populate the map
    */
    bool status;
    CSV_FIELDS output_fields;
    status = parse(input_line, output_fields);

    if(status == true && output_fields.size() == header_fields.size())
    {
        VECTOR_ITR it1 = output_fields.begin();
        VECTOR_ITR it2 = header_fields.begin();
        for( ; it1 != output_fields.end(); ++it1, ++it2)
        {
            key_val.insert(MAP_ENTRY(*it2, *it1));
        }
    }
    return status;
}

bool CSV_Parser :: parse(const STR& input_line, CSV_FIELDS& output_fields)
{
    /*
        A private method which handles the parsing logic used by both the overloaded public methods
    */
    STR field;
    int i, j;

    if(input_line.length() == 0)
    {
        return false;
    }

    i = 0;
    do
    {
        if(i < input_line.length() && input_line[i] == CSV_QUOTE)
        {
            j = parse_quoted_fields(input_line, field, ++i);
        }
        else
        {
            j = parse_normal_fields(input_line, field, i);
        }
        output_fields.push_back(field);
        i = j + 1;
    }while(j < input_line.length());

    return true;
}

int CSV_Parser :: parse_normal_fields(const STR& input_line, STR& field, int& i)
{
    /*
        Normal fields are the ones which contain no escaped or quoted characters
        For instance - Consider that input_line is - 1997,Ford,E350,"Super, luxurious truck"
        An example for a normal field would be - Ford
    */
    int j;
    j = input_line.find_first_of(CSV_DELIMITER, i);
    if(j > input_line.length())
    {
        j = input_line.length();
    }
    field = std :: string(input_line, i, j-i);
    return j;
}

int CSV_Parser :: parse_quoted_fields(const STR& input_line, STR& field, int& i)
{
    /*
        Quoted fields are the ones which are enclosed within quotes
        For instance - Consider that input_line is - 1997,Ford,E350,"Super, luxurious truck"
        An example for a quoted field would be - Super luxurious truck
        Another instance being - 1997,Ford,E350,"Super, ""luxurious"" truck"
    */
    int j;
    field = "";

    for(j=i; j<input_line.length(); j++)
    {
        if(input_line[j] == '"' && input_line[++j] != '"')
        {
            int k = input_line.find_first_of(CSV_DELIMITER, j);
            if(k > input_line.length())
            {
                k = input_line.length();
            }
            for(k -= j; k-- > 0; )
            {
                field += input_line[j++];
            }
            break;
        }
        else
        {
            field += input_line[j];
        }
    }
    return j;
}
