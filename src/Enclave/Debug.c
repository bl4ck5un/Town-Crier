#include "Debug.h"
#include "string.h"
#include "glue.h"

#define MIN(x,y) (x < y ? x : y)

#define DEBUG_LIMIT 2048

void dump_buf(const char *title, const unsigned char *buf, size_t len)
{
  hexdump(title, buf, MIN(DEBUG_LIMIT, len));
  if (DEBUG_LIMIT < len) {
    printf_sgx("...\n");
  }
}

void hexdump(const char* title, void const * data, size_t len)
{
    unsigned int i;
    unsigned int r,c;
    
    if (!data)
	return;

    printf_sgx("%s\n", title);
    
    for (r=0,i=0; r<(len/16+(len%16!=0)); r++,i+=16)
    {
        printf_sgx("0x%04X:   ",i); /* location of first byte in line */
	
        for (c=i; c<i+8; c++) /* left half of hex dump */
	    if (c<len)
        	printf_sgx("%02X ",((unsigned char const *)data)[c]);
	    else
		printf_sgx("   "); /* pad if short line */
	
	printf_sgx("  ");
	
	for (c=i+8; c<i+16; c++) /* right half of hex dump */
	    if (c<len)
		printf_sgx("%02X ",((unsigned char const *)data)[c]); else
		printf_sgx("   "); /* pad if short line */
	
	printf_sgx("   ");
	
	for (c=i; c<i+16; c++) /* ASCII dump */
	    if (c<len)
		if (((unsigned char const *)data)[c]>=32 &&
		    ((unsigned char const *)data)[c]<127)
		    printf_sgx("%c",((char const *)data)[c]);
		else
		    printf_sgx("."); /* put this for non-printables */
	    else
		printf_sgx(" "); /* pad if short line */
	
	printf_sgx("\n");
    }
}

static void bin_to_strhex(const unsigned char *bin, size_t binsz, char *result)
{
  char          hex_str[]= "0123456789abcdef";
  unsigned int  i;

  result[binsz * 2] = 0;

  if (!binsz)
    return;

  for (i = 0; i < binsz; i++)
  {
    result[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
    result[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
  }
}

void print_str_dbg(const char* title, const unsigned char* data, size_t len) {
#ifdef DEBUG
  size_t PRINT_WIDTH = 64;

  char buf[len*2];
  bin_to_strhex(data, len, buf);
  printf_sgx("%s\n", title);

  size_t printed = 0;
  while (printed < strlen(buf)) {
    printf_sgx("%.64s\n", buf + printed);
    printed += MIN(PRINT_WIDTH, strlen(buf + printed));
  }
#endif
}