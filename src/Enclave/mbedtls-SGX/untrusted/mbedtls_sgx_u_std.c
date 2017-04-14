#include <stdio.h>

int ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    int ret = printf("%s", str);
    fflush(stdout);
    return ret;
}

int ocall_fprint_err(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    int ret = fprintf(stderr, "%s", str);
    fflush(stdout);
    return ret;
}
