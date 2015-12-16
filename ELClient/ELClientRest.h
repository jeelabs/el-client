#ifndef _EL_CLIENT_REST_H_
#define _EL_CLIENT_REST_H_

#include <Arduino.h>
#include "FP.h"
#include "ELClient.h"

#define DEFAULT_REST_TIMEOUT  5000

typedef enum {
  HEADER_GENERIC = 0,
  HEADER_CONTENT_TYPE,
  HEADER_USER_AGENT
} HEADER_TYPE;

typedef enum {
  HTTP_STATUS_OK = 200
} HTTP_STATUS;

class ELClientRest {
  public:
    ELClientRest(ELClient *e);
    boolean begin(const char* host, uint16_t port, boolean security);
    boolean begin(const char* host);
    void request(const char* path, const char* method, const char* data);
    void request(const char* path, const char* method, const char* data, int len);
    void get(const char* path, const char* data);
    void get(const char* path);
    void post(const char* path, const char* data);
    void put(const char* path, const char* data);
    void del(const char* path, const char* data);

    void setTimeout(uint32_t ms);
    uint16_t getResponse(char* data, uint16_t maxLen);
    void setUserAgent(const char* value);
    // Set Content-Type Header
    void setContentType(const char* value);
    void setHeader(const char* value);

  private:
    uint32_t remote_instance, timeout;
    ELClient *_elc;
    void restCallback(void* resp);
    FP<void, void*> restCb;
    boolean response;
    void *res;

};
#endif // _EL_CLIENT_REST_H_
