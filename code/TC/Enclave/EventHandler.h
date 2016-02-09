#define REQ_TYPE_FINANCE 1

#if defined(__cplusplus)
extern "C" {
#endif
int handle_request(uint64_t request_id, uint8_t request_type, 
                   char* req, int req_len, char* resp, int resp_len);
#if defined(__cplusplus)
}
#endif