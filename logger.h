#ifndef LOGGER
#define LOGGER_H
#include "structures.h"
int writelog(char* filename, struct HTTP_REQUEST request, struct HTTP_RESPONSE response, char* requestip);
#endif /* LOGGER_H */
