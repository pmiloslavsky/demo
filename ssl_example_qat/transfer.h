#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <resolv.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/engine.h"
#include "openssl/conf.h"
#include <time.h>
#define FAIL    -1
#define MICROS_IN_SEC  1000000L
#define NANOS_IN_MICRO 1000L
#define CYCLES 1000
#define MAX_PACKET (8192*2)
