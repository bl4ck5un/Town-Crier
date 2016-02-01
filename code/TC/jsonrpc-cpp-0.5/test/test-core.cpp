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
 * \file test-core.cpp
 * \brief JsonRpc-Cpp core unit tests.
 * \author Sebastien Vincent
 */

#include <cppunit/extensions/HelperMacros.h>

#include "jsonrpc.h"

namespace Json
{
  namespace Rpc
  {
    /**
     * \class TestRpc
     * \brief RPC example.
     */
    class TestRpc
    {
      public:
        /**
         * \brief Reply with success.
         * \param root JSON-RPC request
         * \param response JSON-RPC response
         * \return true if correctly processed, false otherwise
         */
        bool Print(const Json::Value& root, Json::Value& response)
        {
          response["jsonrpc"] = "2.0";
          response["id"] = root["id"];
          response["result"] = "success";
          return true;
        }

        /**
         * \brief Notification.
         * \param root JSON-RPC request
         * \param response JSON-RPC response
         * \return true if correctly processed, false otherwise
         */
        bool Notify(const Json::Value& root, Json::Value& response)
        {
          std::string version = root["jsonrpc"].asString();
          response = Json::Value::null;
          return true;
        }
    };

    /**
     * \class TestCore
     * \brief Unit tests for core objects.
     */
    class TestCore : public CppUnit::TestFixture
    {
      CPPUNIT_TEST_SUITE(Json::Rpc::TestCore);
      CPPUNIT_TEST(testMethod);
      CPPUNIT_TEST(testBatchedCall);
      CPPUNIT_TEST(testBatchedCallParsing);
      CPPUNIT_TEST(testJsonRpcParsing);
      CPPUNIT_TEST(testJsonRpcId);
      CPPUNIT_TEST(testJsonRpcVersion);
      CPPUNIT_TEST_SUITE_END();
       
      public:
        /**
         * \brief Initialize data before launching test.
         */
        void setUp()
        {
          m_handler = new Handler();
        }

        /**
         * \brief Cleanup data after test finished.
         */
        void tearDown()
        {
          delete m_handler;
        }

        /**
         * \brief Test if we can add/remove method.
         */
        void testMethod()
        {
          const std::string str = "{\"jsonrpc\":\"2.0\", \"method\":\"print\"}";
          const std::string str2 =
              "{\"jsonrpc\":\"2.0\", \"method\":\"method_does_not_exist\"}";
          TestRpc obj;
          Json::Value response;

          m_handler->AddMethod(new Json::Rpc::RpcMethod<TestRpc>(obj,
                &TestRpc::Print, std::string("print")));

          CPPUNIT_ASSERT(m_handler->Process(str, response) == true);
          CPPUNIT_ASSERT(m_handler->Process(str2, response) == false);

          m_handler->DeleteMethod(std::string("print"));
          CPPUNIT_ASSERT(m_handler->Process(str, response) == false);
        }

        /**
         * \brief Test batched call.
         */
        void testBatchedCall()
        {
          const std::string str = "[{\"id\":1, \"jsonrpc\":\"2.0\", \"method\":\"print\"}, {\"id\":2, \"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}]";
          const std::string str2 = "[{\"id\":1, \"jsonrpc\":\"2.0\", \"method\":\"print\"}, {\"jsonrpc\":\"2.0\", \"method\":\"notify\"}]";
          TestRpc obj;
          Json::Value response;

          m_handler->AddMethod(new Json::Rpc::RpcMethod<TestRpc>(obj,
                &TestRpc::Print, std::string("print")));
          m_handler->AddMethod(new Json::Rpc::RpcMethod<TestRpc>(obj,
                &TestRpc::Notify, std::string("notify")));

          CPPUNIT_ASSERT(m_handler->Process(str, response) == true);
          CPPUNIT_ASSERT(response.size() == 2);

          response.clear();
          CPPUNIT_ASSERT(m_handler->Process(str2, response) == true);
          CPPUNIT_ASSERT(response.size() == 1);
        }

        /**
         * \brief Test batched call parsing.
         */
        void testBatchedCallParsing()
        {
          /* invalid JSON array */
          const std::string str = "[{\"id\":1, \"jsonrpc\":\"2.0\", \"method\":\"system }, {\"id\":2, \"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}]";
          TestRpc obj;
          Json::Value response;

          m_handler->AddMethod(new Json::Rpc::RpcMethod<TestRpc>(obj,
                &TestRpc::Print, std::string("print")));
          CPPUNIT_ASSERT(m_handler->Process(str, response) == false);
          /* single error response when batched call itself failed */
          CPPUNIT_ASSERT(response.isArray() == false);
        }

        /**
         * \brief Test if parsing JSON-RPC query works as expected.
         */
        void testJsonRpcParsing()
        {
          /* missing a comma */
          const std::string str = "{\"jsonrpc\":\"2.0\" \"method\":\"system.describe\"}";
          /* missing a "}" */
          const std::string str2 = "{\"jsonrpc\":\"2.0\", \"method\":\"system.describe\"";
          /* missing method parameter */
          const std::string str3 = "{\"jsonrpc\":\"2.0\", \"test\":\"system.describe\"}";
          /* "params" argument missing */
          const std::string str4 = "{\"jsonrpc\":\"2.0\", \"method\":\"system.describe\", \"params\":}";
          /* anything but not in JSON format */
          const std::string str5 = "jsonrpc blabla\n\n";
          /* valid query with array as "params"'s argument */
          const std::string str6 = "{\"jsonrpc\":\"2.0\", \"method\":\"system.describe\", \"params\":[10, 11]}";
          /* valid query with object as "params"'s argument */
          const std::string str7 = "{\"jsonrpc\":\"2.0\", \"method\":\"system.describe\", \"params\":{\"id\":1, \"value\":504}}";
          Json::Value response;

          CPPUNIT_ASSERT(m_handler->Process(str, response) == false);
          CPPUNIT_ASSERT(m_handler->Process(str2, response) == false);
          CPPUNIT_ASSERT(m_handler->Process(str3, response) == false);
          CPPUNIT_ASSERT(m_handler->Process(str4, response) == false);
          CPPUNIT_ASSERT(m_handler->Process(str5, response) == false);
          CPPUNIT_ASSERT(m_handler->Process(str6, response) == true);
          CPPUNIT_ASSERT(m_handler->Process(str7, response) == true);
        }

        /**
         * \brief Test if handler reject query with wrong "jsonrpc" attribute.
         */
        void testJsonRpcVersion()
        {
          const std::string str = "{\"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}";
          const std::string str2 = "{\"jsonrpc\":\"1.0\", \"method\":\"system.describe\"}";
          Json::Value response;

          CPPUNIT_ASSERT(m_handler->Process(str, response) == true);
          CPPUNIT_ASSERT(m_handler->Process(str2, response) == false);
        }

        /**
         * \brief Test if handler reject request with array or object "id" attribute.
         */
        void testJsonRpcId()
        {
          const std::string str = "{\"id\":1, \"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}";
          const std::string str2 = "{\"id\":\"2\", \"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}";
          const std::string str3 = "{\"id\":{\"n\":3}, \"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}";
          const std::string str4 = "{\"id\":{\"n\":\"4\"}, \"jsonrpc\":\"2.0\", \"method\":\"system.describe\"}";
          Json::Value response;

          CPPUNIT_ASSERT(m_handler->Process(str, response) == true);
          CPPUNIT_ASSERT(m_handler->Process(str2, response) == true);
          CPPUNIT_ASSERT(m_handler->Process(str3, response) == false);
          CPPUNIT_ASSERT(m_handler->Process(str4, response) == false);
        }

      private:
        /**
         * \brief Handler for JSON-RPC query.
         */
        Handler* m_handler;
    };
  } /* namespace Rpc */
} /* namespace Json */

/* add the test suite in the global registry */
CPPUNIT_TEST_SUITE_REGISTRATION(Json::Rpc::TestCore);

