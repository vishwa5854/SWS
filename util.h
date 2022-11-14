#ifndef UTIL_H_
#define UTIL_H_

#include"structures.h"

int create_response_string(RESPONSE* response, char* response_str);
void create_request_frame(REQUEST* request, char* request_str, int request_str_length);
void get_gmt_date_str(char* time_str, size_t time_length);

#endif /* !UTIL_H_ */