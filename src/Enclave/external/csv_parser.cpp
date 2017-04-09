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
