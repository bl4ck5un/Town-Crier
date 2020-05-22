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

#include <mbedtls-SGX/include/mbedtls/bignum.h>
#include <string.h>
#include <stdexcept>
#include <string>
#include <stdlib.h>

#include "debug.h"
#include "log.h"
#include "pedersen_commit.h"
#include "external/keccak.h"
#include "../Common/macros.h"

#include "sgx.h"
#include "sgx_trts.h"

using std::runtime_error;

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif

#include "mbedtls/bignum.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"

// CONSTANTS
// JUBJUB_A = Fq(168700)
// JUBJUB_D = Fq(168696)
// JUBJUB_COFACTOR = Fr(8) // doesn't look needed
// Fq.ZERO = Fq(0)
// Fq.ONE = Fq(1)
// Fq.MINUS_ONE = Fq(-1)
// Point.ZERO = Point(Fq.ZERO, Fq.ONE)
// q_j = 21888242871839275222246405745257275088548364400416034343698204186575808495617 
// r_j = 2736030358979909402780800718157159386076813972158567259200215660948447373041


std::string JUBJUB_A_s("168700");
std::string JUBJUB_D_s("168696");
std::string Q_J_s("21888242871839275222246405745257275088548364400416034343698204186575808495617");
std::string R_J_s("2736030358979909402780800718157159386076813972158567259200215660948447373041");
std::string QM1D2_s("10944121435919637611123202872628637544274182200208017171849102093287904247808"); //qm1d2 = (21888242871839275222246405745257275088548364400416034343698204186575808495617 - 1)//2
std::string ZERO("0");
std::string ONE("1");
std::string Hu("16540640123574156134436876038791482806971768689494387082833631921987005038935");
std::string Hv("20819045374670962167435360035096875258406992893633759881276124905556507972311");
std::string BASEu("17777552123799933955779906779655732241715742912184938656739573121738514868268");
std::string BASEv("2626589144620713026669568689430873010625803728049924121243784502389097019475");


mbedtls_mpi bignum_from_string(const char* number){
    mbedtls_mpi result;
    mbedtls_mpi_init(&result);
    int errorCode = mbedtls_mpi_read_string(&result, 10, number);
    return result;
}

struct field_element{
    mbedtls_mpi modulus;
    mbedtls_mpi point;
};

field_element add_field(field_element a, field_element b){
    field_element result;
    result.modulus = a.modulus;
    mbedtls_mpi addition;
    mbedtls_mpi modulus;
    mbedtls_mpi_init(&addition);
    mbedtls_mpi_init(&modulus);

    mbedtls_mpi_add_mpi(&addition, &a.point, &b.point);
    mbedtls_mpi_mod_mpi(&modulus, &addition, &result.modulus);
    result.point = modulus;
    return result;
}

field_element sub_field(field_element a, field_element b){
    field_element result;
    result.modulus = a.modulus;
    mbedtls_mpi subtraction;
    mbedtls_mpi modulus;
    mbedtls_mpi_init(&subtraction);
    mbedtls_mpi_init(&modulus);

    mbedtls_mpi_sub_mpi(&subtraction, &a.point, &b.point);
    mbedtls_mpi_mod_mpi(&modulus, &subtraction, &result.modulus);
    result.point = modulus;
    return result;
}

field_element mul_field(field_element a, field_element b){
    field_element result;
    result.modulus = a.modulus;
    mbedtls_mpi multiplication;
    mbedtls_mpi modulus;
    mbedtls_mpi_init(&multiplication);
    mbedtls_mpi_init(&modulus);

    mbedtls_mpi_mul_mpi(&multiplication, &a.point, &b.point);
    mbedtls_mpi_mod_mpi(&modulus, &multiplication, &result.modulus);
    result.point = modulus;
    return result;
}

field_element exp_field(field_element a, field_element b){
    field_element result;
    result.modulus = a.modulus;
    mbedtls_mpi exponentiation;
    mbedtls_mpi_init(&exponentiation);

    int error_code = mbedtls_mpi_exp_mod(&exponentiation, &a.point, &b.point, &a.modulus, NULL); 
    result.point = exponentiation;
    return result;
}

field_element inv_field(field_element a){
    field_element result;
    result.modulus = a.modulus;
    field_element minus2Point;
    minus2Point.modulus = a.modulus;
    mbedtls_mpi minus2;
    mbedtls_mpi_init(&minus2);
    mbedtls_mpi_sub_int(&minus2, &a.modulus, 2);
    minus2Point.point = minus2;
    result = exp_field(a, minus2Point); // change the second element to bignum m-2
    return result;
}

field_element div_field(field_element a, field_element b){
    field_element result = mul_field(a, inv_field(b));
    return result;
}


struct field_q{
    field_element point;
}; // possibly just implement straight on mod p, not sure

field_q new_element(const char* buf){
    field_q result;
    field_element fq_point;
    mbedtls_mpi modulus = bignum_from_string(Q_J_s.c_str());
    fq_point.modulus = modulus;
    mbedtls_mpi point = bignum_from_string(buf);
    fq_point.point = point;
    result.point = fq_point;
    return result;
}

field_q field_element_from_bytes(const char* buf){
    field_q result;
    field_element fq_point;
    mbedtls_mpi modulus = bignum_from_string(Q_J_s.c_str());
    fq_point.modulus = modulus;
    mbedtls_mpi point;
    mbedtls_mpi_init(&point);

    size_t size = strlen(buf);
    int error_code = mbedtls_mpi_read_binary(&point, (const unsigned char *) buf, size);

    fq_point.point = point;
    result.point = fq_point;
    return result;
}

field_q add(field_q a, field_q b){
    field_q result;
    result.point = add_field(a.point, b.point);
    return result;
}

field_q sub(field_q a, field_q b){
    field_q result;
    result.point = sub_field(a.point, b.point);
    return result;
}

field_q mul(field_q a, field_q b){
    field_q result;
    result.point = mul_field(a.point, b.point);
    return result;
}

field_q div(field_q a, field_q b){
    field_q result;
    result.point = div_field(a.point, b.point);
    return result;
}

field_q exp(field_q a, field_q b){
    field_q result;
    result.point = exp_field(a.point, b.point);
    return result;
}

field_q inv(field_q a){
    field_q result;
    result.point = inv_field(a.point);
    return result;
}

std::string fq_to_string(field_q a){
    mbedtls_mpi s = a.point.point;
    size_t x;
    char s1[1000];
    int error_code = mbedtls_mpi_write_string(&s, 10,s1,1000, &x);
    std::string str(s1);
    return str;
}

field_q sqrt(field_q a){
    field_q result;
    // field_q a = exp(a, qm1d2);
    // unimplemented
    return result;
}

struct jj_point {
    field_q u;
    field_q v;
}; // point on jubjub EC

// helper function, multiplies five numbers
field_q ternary_product(field_q a, field_q b, field_q c){
    field_q result = mul(a, mul(b, c));
    return result;
}

// helper function, multiplies five numbers
field_q quintary_product(field_q a, field_q b, field_q c, field_q d, field_q e){
    field_q result = mul(a, mul(b, mul(c, mul(d, e))));
    return result;
}

jj_point init_point(field_q u, field_q v){
    jj_point result;
    result.u = u;
    result.v = v;
    return result;
}

jj_point add_point(jj_point a, jj_point b){
    jj_point result;
    field_q u1 = a.u;
    field_q u2 = b.u;
    field_q v1 = a.v;
    field_q v2 = b.v;
    field_q one = new_element(ONE.c_str());
    field_q JUBJUB_D = new_element(JUBJUB_D_s.c_str());
    field_q JUBJUB_A = new_element(JUBJUB_A_s.c_str());

    field_q numerator = add(mul(u1, v2), mul(v1, u2));
    field_q denominator = add(one, quintary_product(JUBJUB_D, u1, u2, v1, v2));
    field_q u3 = div(numerator, denominator);

    field_q v3 = div((sub(mul(v1,v2), ternary_product(JUBJUB_A, u1, u2))), 
            (sub(one, quintary_product(JUBJUB_D, u1, u2, v1, v2))));

    result.u = u3;
    result.v = v3;
    return result;
}

jj_point double_point(jj_point a){
    jj_point result;
    result = add_point(a, a);
    return result;
}

jj_point mul_point(jj_point a, field_q b){
    jj_point ret;

    mbedtls_mpi s = b.point.point;
    size_t x;
    char s1[1000];
    int error_code = mbedtls_mpi_write_string(&s, 2,s1,1000, &x);
    std::string str(s1);

    ret.u = new_element(ZERO.c_str());
    ret.v = new_element(ONE.c_str());

    for(std::string::size_type i = 0; i < str.size(); ++i) {
        ret = double_point(ret);
        if (str[i] == '1'){
            ret = add_point(ret, a);
        }
    }

    return ret;
}

// generate random string for seed;
std::string random_seed(){
    char output[1000];
    sgx_read_rand((unsigned char*) &output, 31);
    std::string str(output);
    return str;
}


const char* commit(const char* message, const char* secret) {
    jj_point H = init_point(new_element(Hu.c_str()), new_element(Hv.c_str()));
    jj_point base = init_point(new_element(BASEu.c_str()), new_element(BASEv.c_str()));

    field_q m = field_element_from_bytes(message);
    field_q r = new_element(random_seed().c_str());
    
    jj_point commitment = add_point(mul_point(base, m), mul_point(H, r));
    // LL_DEBUG("U is %s", fq_to_string(commitment.u).c_str());
    // LL_DEBUG("V is %s", fq_to_string(commitment.v).c_str());
    
    std::string u = fq_to_string(commitment.u);
    std::string v = fq_to_string(commitment.v);
    
    std::string commit = "(" + u +"," + v + ")";
    const char* output = commit.c_str();

    // field_q zero = new_element(ZERO.c_str()); This is my testing code
    // field_q one = new_element(ONE.c_str());
    // field_q resultAdd = add(zero, one);
    // LL_DEBUG("Add result: %s", fq_to_string(resultAdd).c_str());
    // field_q resultSub = add(one, zero);
    // LL_DEBUG("Sub result: %s", fq_to_string(resultSub).c_str());
    // field_q resultMul = mul(one, one);
    // LL_DEBUG("Mul result: %s", fq_to_string(resultMul).c_str());    
    // field_q resultMul2 = mul(one, zero);
    // LL_DEBUG("Other Mul result: %s", fq_to_string(resultMul2).c_str());
    // field_q resultExp = exp(one, one);
    // LL_DEBUG("Exp result: %s", fq_to_string(resultExp).c_str());
    // field_q resultInv = inv(one);
    // LL_DEBUG("Inv result: %s", fq_to_string(resultInv).c_str());    
    
    // jj_point zero_point = init_point(zero, one);
    // jj_point point_add_result = double_point(zero_point);
    // LL_DEBUG("Points initilaized successfully");
    // LL_DEBUG("Addition yields (%s, %s)", fq_to_string(point_add_result.u).c_str(), fq_to_string(point_add_result.v).c_str());

    
    // std::string m_str("123123123123312321321");
    // std::string r_str("123123123123123123123123");
    // field_q m = new_element(m_str.c_str());
    // field_q r = new_element(r_str.c_str());

    // jj_point commitment = add_point(mul_point(base, m), mul_point(H, r));

    // LL_DEBUG("Result: %s", result.point.point); 
    return output;
}