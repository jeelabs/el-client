/*! \file ELClientWebServer.h
    \brief Definitions for ELClientWebServer
    \author Cs. Karai
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by Cs. Karai and T. von Eicken

#ifndef _EL_CLIENT_WEB_SERVER_H_
#define _EL_CLIENT_WEB_SERVER_H_

#include <Arduino.h>
#include "ELClient.h"
#include "FP.h"

/** Web Server URL handler structure. */
typedef struct URL_HANDLER
{

  String                URL;         ///< the URL to handle

  /*! loadCb()
   @brief Callback for HTML page loading
   @details This callback is called when browser loads a custom page for the first time.
     The callback should populate all the fields that are required for displaying the page.
   @param url
     The URL of the page to load.
   @par Example
   @code
   void loadCb(char * url)
   {
     webServer.setArgString(F("field1"), F("value"));
     webServer.setArgInt(F("field2"), 123);
     webServer.setArgFloat(F("field3"), 12.3);
   }
   @endcode
   */
  FP<void, char*>       loadCb;

  /*! refreshCb()
   @brief Callback for HTML page refresh
   @details This callback is called when browser refreshes a page. The callback should populate
     all the fields that are required for displaying changes on the page.
   @param url
     The URL of the page to load.
   @par Example
   @code
   void refreshCb(char * url)
   {
     webServer.setArgString(F("field1"), F("changed_value"));
     webServer.setArgInt(F("field2"), 543);
     webServer.setArgFloat(F("field3"), 54.3);
   }
   @endcode
   */
  FP<void, char*>       refreshCb;

  /*! buttonCb()
   @brief callback for setting a field from a HTML form
   @details This callback is called when a HTML form is submitted. The callback should save
     field changes.  After processing the callback, a page refresh will also be initiated to
     reflect the changes.

     The size of the receive buffer on ELClient is 128 bytes which can be small for receiving
     a large form. When the data exceeds 128 bytes, Esp-Link segments the query into smaller
     parts. If the callback is slow, it's possible that the UART buffer overruns while receiving
     the next packet causing data loss. The callback has 5ms to process the request if data comes
     at 115200 baud.

     If debugging is enabled, data loss is very likely to happen because of packet logging.
   @warning Write this routine to be as fast as possible and turn off debugging for receiving
     large forms.

   @param field_id
     The ID of the field
   @par Example
   @code
   int32_t value;

   // this code should be fast
   void setFieldCb(char * field_id)
   {
     String id = field_id;

     if( id == F("my_field") )
     {
       value = webServer.getArgInt();
     }
   }
   @endcode
   */
  FP<void, char*>       setFieldCb;

  /*! buttonCb()
   @brief callback for a HTML button press
   @details This callback is called when user presses a HTML button. After processing this
     callback, a page refresh will also be called to reflect the changes.
   @param button_id
     The ID of the button
   @par Example
   @code
   void buttonCb(char * button_id)
   {
     String id = button_id;

     if( id == F("my_button") )
     {
       Serial.println("My button was pressed");
     }
   }
   @endcode
   */
  FP<void, char*>       buttonCb;
  struct URL_HANDLER *  next;        ///< next handler
} URLHandler;

// This class implements functions for the web-server
class ELClientWebServer {
public:
  // Initialize with an ELClient object
  ELClientWebServer(ELClient* elc);

  // initializes the web-server
  void    setup();

  // creates a URL handler
  URLHandler * createURLHandler(const char * URL);
  // creates a URL handler from flash
  URLHandler * createURLHandler(const __FlashStringHelper * URL);
  // creates a URL handler from String
  URLHandler * createURLHandler(const String &s);

  // destroys a URL handler
  void    destroyURLHandler(URLHandler * handler);

  // sets int value of a HTML field
  void    setArgInt(const char * name, int32_t value);
  // sets int value of a HTML field
  void    setArgInt(const __FlashStringHelper * name, int32_t value);
  // sets JSON value of a HTML field
  void    setArgJson(const char * name, const char * value);
  // sets JSON value of a HTML field
  void    setArgJson(const __FlashStringHelper * name, const char * value);
  // sets JSON value of a HTML field
  void    setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value);
  // sets string value of a HTML field
  void    setArgString(const char * name, const char * value);
  // sets string value of a HTML field
  void    setArgString(const __FlashStringHelper * name, const char * value);
  // sets string value of a HTML field
  void    setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value);
  // sets boolean value of a HTML field
  void    setArgBoolean(const char * name, uint8_t value);
  // sets boolean value of a HTML field
  void    setArgBoolean(const __FlashStringHelper * name, uint8_t value);
  // sets null value of a HTML field
  void    setArgNull(const char * name);
  // sets null value of a HTML field
  void    setArgNull(const __FlashStringHelper * name);
  // sets float value of a HTML field
  void    setArgFloat(const char * name, float f);
  // sets float value of a HTML field
  void    setArgFloat(const __FlashStringHelper * name, float f);


  // setFieldCb: gets the value of the field as integer
  int32_t getArgInt();
  // setFieldCb: gets the value of the field as string
  char *  getArgString();
  // setFieldCb: gets the value of the field as boolean
  uint8_t getArgBoolean();
  // setFieldCb: gets the value of the field as float
  float   getArgFloat();


  /*! ELClientWebServer::getInstance()
  @brief Returns the singleton web-server instance.
  @details Web-Server is a singleton object. This object can be read by calling ELClientWebServer::getInstance().
  @returns
    The singleton web-server instance.
  @par Example code
  @code
    ELClientWebServer *webServer = ELClientWebServer::getInstance();
  @endcode
  */
  static ELClientWebServer * getInstance() { return instance; }

private:
  ELClient* _elc;

  static void webServerPacketHandler(void * packet);
  void processResponse(ELClientResponse *packet); // internal
  static ELClientWebServer * instance;

  uint8_t                    remote_ip[4];
  uint16_t                   remote_port;

  struct URL_HANDLER       * handlers;
  char *                     arg_ptr;
  FP<void, void*>            webServerCb;
};

#endif // _EL_CLIENT_WEB_SERVER_H_
