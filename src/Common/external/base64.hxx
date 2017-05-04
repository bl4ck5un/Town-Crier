
#ifndef BASE64_HXX
#define BASE64_HXX

namespace ext {

// encoding
int b64_ntop(unsigned char const *src, size_t srclength, char *target, size_t targsize);

// decoding
int b64_pton(const char *src, unsigned char*target, size_t targsize);
}

#endif /* BASE64_HXX */
