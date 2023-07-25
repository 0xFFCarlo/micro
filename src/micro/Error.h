#ifndef ERROR_H
#define ERROR_H

#define MICRO_FALSE 0
#define MICRO_TRUE 1

#define MICRO_ERROR_NONE 0
#define MICRO_ERROR_FAIL 1

extern int microGetErrorCode();
extern const char *microGetErrorMessage();
void microSendError(const int code, const char *message);

#endif
