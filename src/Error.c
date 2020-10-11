#include "Error.h"
#include <string.h>

#define MICRO_ERRORBUFFER_SIZE 128

static int errorCode = 0;
static char errorMsg[MICRO_ERRORBUFFER_SIZE];

int microGetErrorCode()
{
    return errorCode;
}

const char* microGetErrorMessage()
{
    return errorMsg;
}

void microSendError(const int code, const char* message)
{
    errorCode = code;
    memcpy(errorMsg, message, sizeof(char) * (strlen(message) + 1));
}