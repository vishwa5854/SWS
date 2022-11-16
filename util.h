#ifndef UTIL_H_
#define UTIL_H_

#include<stdbool.h>
#include"structures.h"

/** This function will create a string based on the reposnse object, which
 is used to send the response to the client. */
int create_response_string(RESPONSE* response, char* response_str);

/** This function will validate the request and set the request meta
 into the given request object. */
bool create_request_frame(REQUEST* request, char* request_str, int request_str_length);

/** This function will return a GMT string which looks like HTTP date
 Eg: Mon, 14 Nov 2022 00:18:42 GMT 
 TODO: Make this function to accept a time pointer for file modification time. */
void get_gmt_date_str(char* time_str, size_t time_length);

/** This function will return an appropriate verb based on status code */
void get_status_verb(int status_code, char* status_verb);

#endif /* !UTIL_H_ */