//
// Created by sgx on 2/7/17.
//

#ifndef TOWN_CRIER_UTILS_H
#define TOWN_CRIER_UTILS_H

/* allows to differentiate between User error and server-side error */
enum err_code{
    NO_ERROR=0,
    INVALID_PARAMS, /* user supplied invalid parameters to the function */
    WEB_ERROR       /* Unable to get web request */
};

#endif //TOWN_CRIER_UTILS_H
