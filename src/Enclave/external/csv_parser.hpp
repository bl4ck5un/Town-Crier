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
