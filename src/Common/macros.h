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

#endif //TOWN_CRIER_MACROS_H
