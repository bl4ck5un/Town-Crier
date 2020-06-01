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

#include <gtest/gtest.h>

#include "App/utils.h"
#include "App/Enclave_u.h"
#include "App/test/SGXTestBase.h"

class Scraper : public SGXTestBase {};

TEST_F(Scraper, yahoo) {
  int ocall_status, ret;
  ocall_status = yahoo_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(Scraper, coinmarket) {
  int ocall_status, ret;
  ocall_status = coin_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(Scraper, steam) {
  int ocall_status, ret;
  ocall_status = steam_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, google) {
  int ocall_status, ret;
  ocall_status = google_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(Scraper, bloomberg) {
  int ocall_status, ret;
  ocall_status = bloomberg_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(Scraper, flight) {
  int ocall_status, ret;
  ocall_status = flight_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, stock) {
  int ocall_status, ret;
  ocall_status = stockticker_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, weather) {
  int ocall_status, ret;
  ocall_status = weather_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, ups) {
  int ocall_status, ret;
  ocall_status = ups_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, wolfram) {
  int ocall_status, ret;
  ocall_status = wolfram_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, fb) {
  int ocall_status, ret;
  ocall_status = fb_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, fbgraph){
  int ocall_status, ret;
  ocall_status = fb_graph_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, ssalogin){
  int ocall_status, ret;
  std::string input;
  std::cin >> input;
  ocall_status = ssa_self_test(eid, &ret, input.c_str());
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

TEST_F(Scraper, coned){
  int ocall_status, ret;
  ocall_status = coned_self_test(eid, &ret);
  ASSERT_EQ(0, ocall_status);
  ASSERT_EQ(0, ret);
}

#include <time.h>
#include <sys/time.h>
#include <algorithm>

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}

TEST_F(Scraper, benchmarkssa){
  double start_wall, finish_wall, start_cpu, finish_cpu;
  int ocall_status, ret;
  double cpu_results[100];
  double wall_results[100];
  for (int i = 0; i < 100; i++){
    start_wall = get_wall_time();
    start_cpu = get_cpu_time();
    ocall_status = fb_self_test(eid, &ret);
    finish_cpu = get_cpu_time();
    finish_wall = get_wall_time();
    std::cout.precision(17);
    std::cout << "Wall time: " << std::fixed << (finish_wall - start_wall) << std::endl;
    std::cout.precision(17);
    std::cout << "CPU time: " << std::fixed << (finish_cpu - start_cpu) << std::endl;
    wall_results[i] = (finish_wall - start_wall);
    cpu_results[i] = (finish_cpu - start_cpu);
  }
  std::sort(wall_results, wall_results + 100);
  std::sort(cpu_results, cpu_results + 100);
  double median_cpu = (cpu_results[49] + cpu_results[50])/2;
  double median_wall = (wall_results[49] + wall_results[50])/2;

  std::cout << "Wall time (Median): " << std::fixed << (median_wall) << std::endl;
  std::cout << "CPU time (Median): " << std::fixed << (median_cpu) << std::endl;
}

TEST_F(Scraper, benchmarkconed){
  double start_wall, finish_wall, start_cpu, finish_cpu;
  int ocall_status, ret;
  double cpu_results[100];
  double wall_results[100];
  for (int i = 0; i < 100; i++){
    start_wall = get_wall_time();
    start_cpu = get_cpu_time();
    ocall_status = coned_self_test(eid, &ret);
    finish_cpu = get_cpu_time();
    finish_wall = get_wall_time();
    std::cout.precision(17);
    std::cout << "Wall time: " << std::fixed << (finish_wall - start_wall) << std::endl;
    std::cout.precision(17);
    std::cout << "CPU time: " << std::fixed << (finish_cpu - start_cpu) << std::endl;
    wall_results[i] = (finish_wall - start_wall);
    cpu_results[i] = (finish_cpu - start_cpu);
  }
  std::sort(wall_results, wall_results + 100);
  std::sort(cpu_results, cpu_results + 100);
  double median_cpu = (cpu_results[49] + cpu_results[50])/2;
  double median_wall = (wall_results[49] + wall_results[50])/2;

  std::cout << "Wall time (Median): " << std::fixed << (median_wall) << std::endl;
  std::cout << "CPU time (Median): " << std::fixed << (median_cpu) << std::endl;
}

TEST_F(Scraper, benchmarklev){
  double start_wall, finish_wall, start_cpu, finish_cpu;
  int ocall_status, ret;
  double cpu_results[50];
  double wall_results[50];
  for (int i = 0; i < 50; i++){
    start_wall = get_wall_time();
    start_cpu = get_cpu_time();
    ocall_status = fb_graph_self_test(eid, &ret);
    finish_cpu = get_cpu_time();
    finish_wall = get_wall_time();
    std::cout.precision(17);
    std::cout << "Wall time: " << std::fixed << (finish_wall - start_wall) << std::endl;
    std::cout.precision(17);
    std::cout << "CPU time: " << std::fixed << (finish_cpu - start_cpu) << std::endl;
    wall_results[i] = (finish_wall - start_wall);
    cpu_results[i] = (finish_cpu - start_cpu);
  }
  std::sort(wall_results, wall_results + 50);
  std::sort(cpu_results, cpu_results + 50);
  double median_cpu = (cpu_results[24] + cpu_results[25])/2;
  double median_wall = (wall_results[24] + wall_results[25])/2;

  std::cout << "Wall time (Median): " << std::fixed << (median_wall) << std::endl;
  std::cout << "CPU time (Median): " << std::fixed << (median_cpu) << std::endl;
}