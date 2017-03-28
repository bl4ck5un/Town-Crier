
#ifndef BASE64_HXX
#define BASE64_HXX

namespace ext {
int b64_ntop(u_char const *src, size_t srclength, char *target,
             size_t targsize);
int b64_pton(const char *src, u_char *target, size_t targsize);
}

#endif /* BASE64_HXX */
