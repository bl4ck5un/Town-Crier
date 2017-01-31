//
// Created by fanz on 1/27/17.
//

#ifndef TOWN_CRIER_MACROS_H
#define TOWN_CRIER_MACROS_H

#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(func) func
#endif


/*!
 * used in eth_ecdsa.c
 */
#define SECRETKEY_SEALED_LEN 1024
#define SECKEY_LEN  32
#define PUBKEY_LEN  64
#define ADDRESS_LEN 20

#endif //TOWN_CRIER_MACROS_H
