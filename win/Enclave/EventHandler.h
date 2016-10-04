#define REQ_TYPE_FINANCE 1

#if defined(__cplusplus)
extern "C" {
#endif
int handle_request(int nonce, uint64_t request_id, uint8_t request_type, 
                   uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len);
#if defined(__cplusplus)
}
#endif