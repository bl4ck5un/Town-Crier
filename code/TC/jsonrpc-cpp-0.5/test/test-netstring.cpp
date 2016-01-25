/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file test-netstring.cpp
 * \brief Netstring unit tests.
 * \author Sebastien Vincent
 */

#include <cppunit/extensions/HelperMacros.h>

#include "netstring.h"

namespace netstring
{
  /**
   * \class TestNetstring
   * \brief Unit tests for netstrings.
   */
  class TestNetstring : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(netstring::TestNetstring);
    CPPUNIT_TEST(testEncoding);
    CPPUNIT_TEST(testDecoding);
    CPPUNIT_TEST_EXCEPTION(testDecodingTooLong, netstring::NetstringException);
    CPPUNIT_TEST_EXCEPTION(testDecodingTooLongString, netstring::NetstringException);
    CPPUNIT_TEST_EXCEPTION(testDecodingTooShort, netstring::NetstringException);
    CPPUNIT_TEST_EXCEPTION(testDecodingMissingComma, netstring::NetstringException);
    CPPUNIT_TEST_EXCEPTION(testDecodingMissingColon, netstring::NetstringException);
    CPPUNIT_TEST_SUITE_END();

    public:
      /**
       * \brief Initialize data before launching test.
       */
      void setUp()
      {
      }

      /**
       * \brief Cleanup data after test finished.
       */
      void tearDown()
      {
      }

      /**
       * \brief Test if encoding works as expected.
       */
      void testEncoding()
      {
        const std::string origStr = "Hello World!";
        const std::string encodedStr = "12:Hello World!,";
        std::string str;

        str = netstring::encode(origStr);
        CPPUNIT_ASSERT(str == encodedStr);
      }

      /**
       * \brief Test if decoding works as expected.
       */
      void testDecoding()
      {
        const std::string origStr = "Hello World!";
        const std::string encodedStr = "12:Hello World!,";
        std::string str;

        str = netstring::decode(encodedStr);
        CPPUNIT_ASSERT(str == origStr);
      }

      /**
       * \brief Test if decoding a too long netstring throw
       * an exception.
       */
      void testDecodingTooLong()
      { 
        const std::string encodedStr = "13:Hello World!,";
        std::string str;

        str = netstring::decode(encodedStr);
      }

      /**
       * \brief Test if decoding a too long string in netstring throw
       * an exception.
       */
      void testDecodingTooLongString()
      {
        const std::string encodedStr = "12:Hello World!!!,";
        std::string str;

        str = netstring::decode(encodedStr);
      }

      /**
       * \brief Test if decoding a too short netstring throw
       * an exception.
       */
      void testDecodingTooShort()
      { 
        const std::string encodedStr = "11:Hello World!,";
        std::string str;

        str = netstring::decode(encodedStr);
      }

      /**
       * \brief Test if decoding netstring which is missing comma throw
       * an exception.
       */
      void testDecodingMissingComma()
      { 
        const std::string encodedStr = "12:Hello World!";
        std::string str;

        str = netstring::decode(encodedStr);
      }

      /**
       * \brief Test if decoding netstring which is missing colon throw
       * an exception.
       */
      void testDecodingMissingColon()
      { 
        const std::string encodedStr = "12Hello World!,";
        std::string str;

        str = netstring::decode(encodedStr);
      }
  };
} /* namespace netstring */

/* add the test suite in the global registry */
CPPUNIT_TEST_SUITE_REGISTRATION(netstring::TestNetstring);

