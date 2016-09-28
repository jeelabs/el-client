/*! \file ELClientRest.h
    \brief Definitions for ELClientRes
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#ifndef _EL_CLIENT_REST_H_
#define _EL_CLIENT_REST_H_

#include <Arduino.h>
#include "FP.h"
#include "ELClient.h"

// Default timeout for REST requests when waiting for a response
#define DEFAULT_REST_TIMEOUT  5000 /**< Default timeout for REST requests when waiting for a response */

typedef enum {
  HTTP_STATUS_OK = 200 /**< HTTP status OK response. */
} HTTP_STATUS;

// The ELClientRest class makes simple REST requests to a remote server. Each instance
// is used to communicate with one server and multiple instances can be created to make
// requests to multiple servers.
// The ELClientRest class does not support concurrent requests to the same server because
// only a single response can be recevied at a time and the responses of the two requests
// may arrive out of order.
// A major limitation of the REST class is that it does not store the response body. The
// response status is saved in the class instance, so after a request completes and before
// the next request is made a call to getResponse will return the status. However, only a pointer
// to the response body is saved, which means that if any other message arrives and is
// processed then the response body is overwritten by it. What this means is that if you
// need the response body you best use waitResponse or ensure that any call to ELClient::process
// is followed by a call to getResponse. Ideally someone improves this class to take a callback
// into the user's sketch?
// Another limitation is that the response body is 100 chars long at most, this is due to the
// limitation of the SLIP protocol buffer available.
class ELClientRest {
  public:
    ELClientRest(ELClient *e);

    // Initialize communication to a remote server, this communicates with esp-link but does not
    // open a connection to the remote server. Host may be a hostname or an IP address,
    // security causes HTTPS to be used (not yet supported). Returns 0 if the set-up is
    // successful, returns a negative error code if it failed.
    int begin(const char* host, uint16_t port=80, boolean security=false);

    // Make a request to the remote server. The data must be null-terminated
    void request(const char* path, const char* method, const char* data=NULL);

    // Make a request to the remote server.
    void request(const char* path, const char* method, const char* data, int len);

    // Make a GET request to the remote server with NULL-terminated data
    void get(const char* path, const char* data=NULL);

    // Make a POST request to the remote server with NULL-terminated data
    void post(const char* path, const char* data);

    // Make a PUT request to the remote server with NULL-terminated data
    void put(const char* path, const char* data);

    // Make a DELETE request to the remote server
    void del(const char* path);

    // Retrieve the response from the remote server, returns the HTTP status code, 0 if no
    // response (may need to wait longer)
    uint16_t getResponse(char* data, uint16_t maxLen);

    // Wait for the response from the remote server, returns the HTTP status code, 0 if no
    // response (timeout occurred). This is not recommended except for quick demos, use
    // getResponse periodically instead.
    uint16_t waitResponse(char* data, uint16_t maxLen, uint32_t timeout=DEFAULT_REST_TIMEOUT);

    // Set the user-agent for all subsequent requests
    void setUserAgent(const char* value);

    // Set the Content-Type Header for all subsequent requests
    void setContentType(const char* value);

    // Set a custom header for all subsequent requests
    void setHeader(const char* value);

  private:
    int32_t remote_instance; /**< Connection number, value can be 0 to 3 */
    ELClient *_elc; /**< ELClient instance */
    void restCallback(void* resp);
    FP<void, void*> restCb; /**< Pointer to external callback function */

    int16_t _status; /**< Connection status */
    uint16_t _len; /**< Number of sent/received bytes */
    void *_data; /**< Buffer for received data */


};
#endif // _EL_CLIENT_REST_H_
