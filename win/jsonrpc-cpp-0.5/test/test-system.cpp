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
 * \file test-system.cpp
 * \brief System unit tests.
 * \author Sebastien Vincent
 */

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "system.h"

namespace system_util
{
  /**
   * \class Callback
   * \brief Callback example.
   */
  class Callback
  {
    public:
      /**
       * \brief Method called by the thread.
       */
      void* Call(void* arg)
      {
        void* a = (void*)0xABCD;
        
        arg = arg; /* not used */
        
        return a;
      }
      
      /**
       * \brief Method called by the thread.
       */
      void* Call2(void* arg)
      {
        /* infinite loop */
        void* a = Call(arg);
        
        while(1)
        {
          /* need a 'cancelable' function
           * if there have no cancelable call in method,
           * the Thread::Stop method cannot stop the thread.
           */
          system_util::msleep(1000);
        }
        
        return a;
      }
  };

  /** 
   * \class TestSystem
   * \brief Unit tests for system objects.
   */
  class TestSystem : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(system_util::TestSystem);
    CPPUNIT_TEST(testThreadCreate);
    CPPUNIT_TEST(testThreadCancel);
    CPPUNIT_TEST(testMutex);
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
       * \brief Test the creation and join of thread. 
       */
      void testThreadCreate()
      {
        void* ret = NULL;
        Callback obj;
        Thread th = Thread(new ThreadArgImpl<Callback>(obj, &Callback::Call,
              NULL));

        th.Start(false);
        th.Join(&ret);

        CPPUNIT_ASSERT(ret == (void*)0xABCD);
      }

      /**
       * \brief Test the cancelation of thread. 
       */
      void testThreadCancel()
      {
        void* ret = NULL;
        Callback obj;
        Thread th = Thread(new ThreadArgImpl<Callback>(obj, &Callback::Call2,
              NULL));

        th.Start(false);
        th.Stop();
        th.Join(&ret);
        
        /* on x86 it returns 0xffffffff
         * on x86-64 0xffffffffffffffff
         * so (void*)-1
         */
        CPPUNIT_ASSERT(ret == (void*)-1);
      }

      /**
       * \brief Test mutex.
       */
      void testMutex()
      {
        Mutex mutex;

        CPPUNIT_ASSERT(mutex.Lock());
        CPPUNIT_ASSERT(mutex.Unlock());
      }
  };

} /* namespace system */

/* add the test suite in the global registry */
CPPUNIT_TEST_SUITE_REGISTRATION(system_util::TestSystem);

