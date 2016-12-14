/*! \file ELClientWebServer.cpp
    \brief Constructor and functions for ELClientWebServer
*/
#include "ELClientWebServer.h"

typedef enum {
  WS_LOAD=0,  // page first load
  WS_REFRESH, // page refresh
  WS_BUTTON,  // button press
  WS_SUBMIT,  // form submission
} RequestReason;

typedef enum
{
  WEB_STRING=0,  // value type string
  WEB_NULL,      // value type null
  WEB_INTEGER,   // value type integer
  WEB_BOOLEAN,   // value type boolean
  WEB_FLOAT,     // value type float
  WEB_JSON       // value type json
} WebValueType;


ELClientWebServer * ELClientWebServer::instance = 0;


/*! ELClientWebServer(ELClient* elClient)
@brief Creates a Web-Server instance.
@details This method creates a web-server object.
@param elClient
  Reference to ELClient object.
@par Example code
@code
  // Initialize a connection to esp-link using the normal hardware serial port
  // DEBUG is disabled because of performance reasons
  ELClient esp(&Serial);

  // Initialize the Web-Server client
  ELClientWebServer webServer(&esp);
@endcode
*/

ELClientWebServer::ELClientWebServer(ELClient* elc) :_elc(elc),handlers(0), arg_ptr(0) {
  // save the current packet handler and register a new one
  instance = this;

  webServerCb.attach(&ELClientWebServer::webServerPacketHandler);
}

// packet handler for web-server
void ELClientWebServer::webServerPacketHandler(void * response)
{
  ELClientWebServer::getInstance()->processResponse((ELClientResponse*)response);
}


/*! createURLHandler(const char * URL)
@brief Creates and registers an URL handler.
@details This method is responsible for creating and registering an URL handler object.
@param URL
  The URL the handler handles. URL is "/" + the HTML file name + ".json".
@returns
  The created URLHandler object.
@par Example code
@code
  void myLoadCb(char * url) {
    ...
  }

  void myRefreshCb(char * url) {
    ...
  }

  void myButtonPressCb(char * button_id) {
    ...
  }

  void mySetFieldCb(char * field_id) {
    ...
  }

  void setup()
  {
    ...
    URLHandler *handler = webServer.createURLHandler("/mypage.html.json");
    handler->loadCb.attach(&myLoadCb);
    handler->refreshCb.attach(&myRefreshCb);
    handler->buttonCb.attach(&myButtonPressCb);
    handler->setFieldCb.attach(&mySetFieldCb);
    ...
  }
@endcode
*/

URLHandler * ELClientWebServer::createURLHandler(const char * URL)
{
  String s = URL;
  return createURLHandler(s);
}


/*! createURLHandler(const __FlashStringHelper * URL)
@brief Creates and registers an URL handler.
@details This method is responsible for creating and registering an URL handler object.
@param URL
  The URL the handler handles. URL is "/" + the HTML file name + ".json".
@returns
  The created URLHandler object.
@par Example code
@code
  void myLoadCb(char * url) {
    ...
  }

  void myRefreshCb(char * url) {
    ...
  }

  void myButtonPressCb(char * button_id) {
    ...
  }

  void mySetFieldCb(char * field_id) {
    ...
  }

  void setup()
  {
    ...
    URLHandler *handler = webServer.createURLHandler(F("/mypage.html.json"));
    handler->loadCb.attach(&myLoadCb);
    handler->refreshCb.attach(&myRefreshCb);
    handler->buttonCb.attach(&myButtonPressCb);
    handler->setFieldCb.attach(&mySetFieldCb);
    ...
  }
@endcode
*/

URLHandler * ELClientWebServer::createURLHandler(const __FlashStringHelper * URL)
{
  String s = URL;
  return createURLHandler(s);
}


/*! createURLHandler(const String &URL)
@brief Creates and registers an URL handler.
@details This method is responsible for creating and registering an URL handler object.
@param URL
  The URL the handler handles. URL is "/" + the HTML file name + ".json".
@returns
  The created URLHandler object.
@par Example code
@code
  void myLoadCb(char * url) {
    ...
  }

  void myRefreshCb(char * url) {
    ...
  }

  void myButtonPressCb(char * button_id) {
    ...
  }

  void mySetFieldCb(char * field_id) {
    ...
  }

  void setup()
  {
    ...
    String url = F("/mypage.html.json");
    URLHandler *handler = webServer.createURLHandler(url);
    handler->loadCb.attach(&myLoadCb);
    handler->refreshCb.attach(&myRefreshCb);
    handler->buttonCb.attach(&myButtonPressCb);
    handler->setFieldCb.attach(&mySetFieldCb);
    ...
  }
@endcode
*/

URLHandler * ELClientWebServer::createURLHandler(const String &URL)
{
  struct URL_HANDLER * hnd = new struct URL_HANDLER(); // "new" is used here instead of malloc to call String destructor at freeing. DOn't use malloc/free.
  hnd->URL = URL;             // handler URL
  hnd->next = handlers;       // next handler
  handlers = hnd;             // change the first handler
  return hnd;
}


/*! destroyURLHandler(URLHandler * handler)
@brief Unregisters an destroys an URL handler.
@details This method is responsible destroying an URL handler object.
@param handler
  The handler to destroy.
@par Example code
@code
  URLHandler *handler = ...

  destroyURLHandler(handler);
@endcode
*/

void ELClientWebServer::destroyURLHandler(URLHandler * handler)
{
  struct URL_HANDLER *prev = 0;
  struct URL_HANDLER *hnd = handlers;
  while( hnd != 0 )
  {
    if( hnd == handler )
    {
      if( prev == 0 )
        handlers = hnd->next;
      else
        prev->next = hnd->next;

      delete hnd;
      return;
    }
    prev = hnd;
    hnd = hnd->next;
  }
}


/*! setup()
@brief Initializes web-server.
@details Initialization means to subscribe to Web-Server callback of Esp-Link.
@par Example code
@code
  void resetCb(void) {
    Serial.println("EL-Client (re-)starting!");
    bool ok = false;
    do {
      ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
      if (!ok) Serial.println("EL-Client sync failed!");
    } while(!ok);
    Serial.println("EL-Client synced!");

    webServer.setup();
  }

  void setup()
  {
    Serial.begin(115200);

    URLHandler *handler = webServer.createURLHandler(F("/mypage.html.json"));
    handler->loadCb.attach(&myLoadCb);
    handler->refreshCb.attach(&myRefreshCb);
    handler->buttonCb.attach(&myButtonPressCb);
    handler->setFieldCb.attach(&mySetFieldCb);

    esp.resetCb = resetCb;
    resetCb();
  }
@endcode
*/

void ELClientWebServer::setup()
{
  // WebServer doesn't send messages to MCU only if asked
  // register here to the web callback
  // periodic reregistration is required in case of ESP8266 reset
  _elc->Request(CMD_WEB_SETUP, 0, 1);
  uint32_t cb = (uint32_t)&webServerCb;
  _elc->Request(&cb, 4);
  _elc->Request();
}

void ELClientWebServer::processResponse(ELClientResponse *response)
{
  uint16_t shrt;
  response->popArg(&shrt, 2);
  RequestReason reason = (RequestReason)shrt; // request reason

  response->popArg(remote_ip, 4);    // remote IP address
  response->popArg(&remote_port, 2); // remote port

  char * url;
  uint16_t urlLen = response->popArgPtr((void**)&url);

  struct URL_HANDLER *hnd = handlers;
  while( hnd != 0 )
  {
    if( hnd->URL.length() == urlLen && memcmp( url, hnd->URL.c_str(), urlLen ) == 0 )
      break;
    hnd = hnd->next;
  }

  if( hnd == 0 ) // no handler found for the URL
  {
    if( _elc->_debugEn )
    {
      _elc->_debug->print(F("Handler not found for URL:"));

      for(uint16_t i=0; i < urlLen; i++)
        _elc->_debug->print( url[i] );
      _elc->_debug->println();
    }
    return;
  }

  switch(reason)
  {
    case WS_BUTTON: // invoked when a button pressed
      {
        char * idPtr;
        int idLen = response->popArgPtr((void**)&idPtr);

        // add terminating 0
        char id[idLen+1];
        memcpy(id, idPtr, idLen);
        id[idLen] = 0;

        hnd->buttonCb(id);
      }
      break;
    case WS_SUBMIT: // invoked when a form submitted
      {
        uint16_t cnt = 4;

        while( cnt < response->argc() )
        {
          char * idPtr;
          int idLen = response->popArgPtr((void**)&idPtr);
          int nameLen = strlen(idPtr+1);
          int valueLen = idLen - nameLen -2;

          // add terminating 0
          arg_ptr = (char *)malloc(valueLen+1);
          arg_ptr[valueLen] = 0;
          memcpy(arg_ptr, idPtr + 2 + nameLen, valueLen);

          hnd->setFieldCb(idPtr+1);

          free(arg_ptr);
          arg_ptr = 0;
          cnt++;
        }
      }
      return;
    case WS_LOAD: // invoked at refresh / load
    case WS_REFRESH:
      break;
    default:
      return;
  }

  // the response is generated here with the fields to refresh

  _elc->Request(CMD_WEB_DATA, 100, VARIABLE_ARG_NUM);
  _elc->Request(remote_ip, 4);               // send remote IP address
  _elc->Request((uint8_t *)&remote_port, 2); // send remote port

  if( reason == WS_LOAD )
    hnd->loadCb( (char*)hnd->URL.c_str() );
  else
    hnd->refreshCb( (char*)hnd->URL.c_str() );

  _elc->Request((uint8_t *)NULL, 0);         // end indicator
  _elc->Request();                           // finish packet
}

/*! setArgJson(const char * name, const char * value)
@brief Sets JSON value of a field
@details Sets JSON value to display an HTML field (list, table).
@param name
  The name of the field
@param value
  JSON value
@par Supported HTML controls
@li  UL
@li  OL
@li  TABLE
@warning Use this method only in refreshCb/loadCb.
@par Example List HTML
@code
  <UL id="list"/>
@endcode
@par Example List code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgJson( "list", "[\"A\",\"B\",\"C\"]" );
  }
@endcode
@par Example Table HTML
@code
  <TABLE id="table"/>
@endcode
@par Example Table code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgJson( "table", "[[\"A\",\"B\"],[\"1\",\"2\"]]" );
  }
@endcode
*/
void ELClientWebServer::setArgJson(const char * name, const char * value)
{
  uint8_t nlen = strlen(name);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy(buf+1, name);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value)
@brief Sets JSON value of a field
@details Sets JSON value to display an HTML field (list, table). It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  JSON value (stored in flash)
@par Supported HTML controls
@li  UL
@li  OL
@li  TABLE
@warning Use this method only in refreshCb/loadCb.
@par Example List HTML
@code
  <UL id="list"/>
@endcode
@par Example List code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgJson( F("list"), F("[\"A\",\"B\",\"C\"]") );
  }
@endcode
@par Example Table HTML
@code
  <TABLE id="table"/>
@endcode
@par Example Table code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgJson( F("table"), F("[[\"A\",\"B\"],[\"1\",\"2\"]]") );
  }
@endcode
*/
void ELClientWebServer::setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  const char * value_p = reinterpret_cast<const char *>(value);

  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen_P(value_p);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy_P(buf+1, name_p);
  strcpy_P(buf+2+nlen, value_p);
  _elc->Request(buf, nlen+vlen+2);
}


/*! setArgJson(const __FlashStringHelper * name, const char * value)
@brief Sets JSON value of a field
@details Sets JSON value to display an HTML field (list, table). It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  JSON value
@par Supported HTML controls
@li  UL
@li  OL
@li  TABLE
@warning Use this method only in refreshCb/loadCb.
@par Example List HTML
@code
  <UL id="list"/>
@endcode
@par Example List code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgJson( F("list"), "[\"A\",\"B\",\"C\"]" );
  }
@endcode
@par Example Table HTML
@code
  <TABLE id="table"/>
@endcode
@par Example Table code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgJson( F("table"), "[[\"A\",\"B\"],[\"1\",\"2\"]]" );
  }
@endcode
*/
void ELClientWebServer::setArgJson(const __FlashStringHelper * name, const char * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);

  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy_P(buf+1, name_p);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}


/*! setArgString(const char * name, const char * value)
@brief Sets string value of a field
@details Sets the string or inner HTML value of an HTML field.
@param name
  The name of the field
@param value
  String value
@par Supported HTML controls

| *HTML Field* |    *Type*    |
|--------------|--------------|
|       P      | Inner HTML   |
|      DIV     | Inner HTML   |
|      TR      | Inner HTML   |
|      TH      | Inner HTML   |
|      TD      | Inner HTML   |
|   TEXTAREA   | Inner HTML   |
|     SPAN     | Inner HTML   |
|    INPUT     | String value |
|    SELECT    | String value |

@warning Use this method only in refreshCb/loadCb.
@par Example for inner HTML (HTML)
@code
  <DIV id="div"/>
@endcode
@par Example for inner HTML (code)
@code
  void refreshCb(const char * url)
  {
    webServer.setArgString( "div", "Line 1 <br/> Line 2" );
  }
@endcode
@par Example for string value (HTML)
@code
  <form>
    <input name="input" type="text"/>
    <input type="submit">
  </form>
@endcode
@par Example for string value (code)
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgString( "input", "This is the value" );
  }
@endcode
*/

void ELClientWebServer::setArgString(const char * name, const char * value)
{
  uint8_t nlen = strlen(name);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy(buf+1, name);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value)
@brief Sets string value of a field
@details Sets the string or inner HTML value of an HTML field. It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  String value (stored in flash)
@par Supported HTML controls

| *HTML Field* |    *Type*    |
|--------------|--------------|
|       P      | Inner HTML   |
|      DIV     | Inner HTML   |
|      TR      | Inner HTML   |
|      TH      | Inner HTML   |
|      TD      | Inner HTML   |
|   TEXTAREA   | Inner HTML   |
|     SPAN     | Inner HTML   |
|    INPUT     | String value |
|    SELECT    | String value |

@warning Use this method only in refreshCb/loadCb.
@par Example for inner HTML (HTML)
@code
  <DIV id="div"/>
@endcode
@par Example for inner HTML (code)
@code
  void refreshCb(const char * url)
  {
    webServer.setArgString( F("div"), F("Line 1 <br/> Line 2") );
  }
@endcode
@par Example for string value (HTML)
@code
  <form>
    <input name="input" type="text"/>
    <input type="submit">
  </form>
@endcode
@par Example for string value (code)
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgString( F("input"), F("This is the value") );
  }
@endcode
*/

void ELClientWebServer::setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  const char * value_p = reinterpret_cast<const char *>(value);

  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen_P(value_p);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy_P(buf+1, name_p);
  strcpy_P(buf+2+nlen, value_p);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgString(const __FlashStringHelper * name, const char * value)
@brief Sets string value of a field
@details Sets the string or inner HTML value of an HTML field. It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  String value
@par Supported HTML controls

| *HTML Field* |    *Type*    |
|--------------|--------------|
|       P      | Inner HTML   |
|      DIV     | Inner HTML   |
|      TR      | Inner HTML   |
|      TH      | Inner HTML   |
|      TD      | Inner HTML   |
|   TEXTAREA   | Inner HTML   |
|     SPAN     | Inner HTML   |
|    INPUT     | String value |
|    SELECT    | String value |

@warning Use this method only in refreshCb/loadCb.
@par Example for inner HTML (HTML)
@code
  <DIV id="div"/>
@endcode
@par Example for inner HTML (code)
@code
  void refreshCb(const char * url)
  {
    webServer.setArgString( F("div"), "Line 1 <br/> Line 2" );
  }
@endcode
@par Example for string value (HTML)
@code
  <form>
    <input name="input" type="text"/>
    <input type="submit">
  </form>
@endcode
@par Example for string value (code)
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgString( F("input"), "This is the value" );
  }
@endcode
*/
void ELClientWebServer::setArgString(const __FlashStringHelper * name, const char * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);

  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy_P(buf+1, name_p);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}


/*! setArgBoolean(const char * name, uint8_t value)
@brief Sets boolean value of a field
@details Sets boolean value of an HTML field.
@param name
  The name of the field
@param value
  Boolean value
@par Supported HTML controls
@li  INPUT
@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <form>
    <input name="input" type="checkbox"/>
    <input type="submit">
  </form>
@endcode
@par Example code
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgBoolean( "input", TRUE );
  }
@endcode
*/

void ELClientWebServer::setArgBoolean(const char * name, uint8_t value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 4];
  buf[0] = WEB_BOOLEAN;
  strcpy(buf+1, name);
  buf[2 + nlen] = value;
  _elc->Request(buf, nlen+3);
}


/*! setArgBoolean(const __FlashStringHelper * name, uint8_t value)
@brief Sets boolean value of a field
@details Sets boolean value of an HTML field. It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  Boolean value
@par Supported HTML controls
@li  INPUT
@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <form>
    <input name="input" type="checkbox"/>
    <input type="submit">
  </form>
@endcode
@par Example code
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgBoolean( F("input"), TRUE );
  }
@endcode
*/

void ELClientWebServer::setArgBoolean(const __FlashStringHelper * name, uint8_t value)
{
  const char * name_p = reinterpret_cast<const char *>(name);

  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 4];
  buf[0] = WEB_BOOLEAN;
  strcpy_P(buf+1, name_p);
  buf[2 + nlen] = value;
  _elc->Request(buf, nlen+3);
}


/*! setArgInt(const char * name, int32_t value)
@brief Sets integer value of a field
@details Sets integer value of an HTML field.
@param name
  The name of the field
@param value
  Integer value
@par Supported HTML controls
@li  INPUT
@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <form>
    <input name="input" type="number"/>
    <input type="submit">
  </form>
@endcode
@par Example code
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgInt( "input", 123 );
  }
@endcode
*/

void ELClientWebServer::setArgInt(const char * name, int32_t value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 7];
  buf[0] = WEB_INTEGER;
  strcpy(buf+1, name);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}


/*! setArgInt(const __FlashStringHelper * name, int32_t value)
@brief Sets integer value of a field
@details Sets integer value of an HTML field. It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  Integer value
@par Supported HTML controls
@li  INPUT
@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <form>
    <input name="input" type="number"/>
    <input type="submit">
  </form>
@endcode
@par Example code
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgInt( F("input"), 123 );
  }
@endcode
*/

void ELClientWebServer::setArgInt(const __FlashStringHelper * name, int32_t value)
{
  const char * name_p = reinterpret_cast<const char *>(name);

  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 7];
  buf[0] = WEB_INTEGER;
  strcpy_P(buf+1, name_p);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}


/*! setArgNull(const char * name)
@brief Sets null value to a field
@details Sets null value to an HTML field.
@param name
  The name of the field
@par Supported HTML controls
@li P
@li DIV
@li TR
@li TH
@li TD
@li TEXTAREA
@li SPAN
@li INPUT
@li SELECT

@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <DIV id="div"/>
@endcode
@par Example code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgNull( "div" );
  }
@endcode
*/

void ELClientWebServer::setArgNull(const char * name)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 2];
  buf[0] = WEB_NULL;
  strcpy(buf+1, name);
  _elc->Request(buf, nlen+2);
}


/*! setArgNull(const __FlashStringHelper * name)
@brief Sets null value to a field
@details Sets null value to an HTML field. It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@par Supported HTML controls
@li P
@li DIV
@li TR
@li TH
@li TD
@li TEXTAREA
@li SPAN
@li INPUT
@li SELECT

@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <DIV id="div"/>
@endcode
@par Example code
@code
  void refreshCb(const char * url)
  {
    webServer.setArgNull( F("div") );
  }
@endcode
*/

void ELClientWebServer::setArgNull(const __FlashStringHelper * name)
{
  const char * name_p = reinterpret_cast<const char *>(name);

  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 2];
  buf[0] = WEB_NULL;
  strcpy_P(buf+1, name_p);
  _elc->Request(buf, nlen+2);
}


/*! setArgFloat(const char * name, float value)
@brief Sets float value of a field
@details Sets float value of an HTML field.
@param name
  The name of the field
@param value
  Float value
@par Supported HTML controls
@li  INPUT
@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <form>
    <input name="input" type="number" step="0.01"/>
    <input type="submit">
  </form>
@endcode
@par Example code
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgFloat( "input", 12.3f );
  }
@endcode
*/

void ELClientWebServer::setArgFloat(const char * name, float value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 7];
  buf[0] = WEB_FLOAT;
  strcpy(buf+1, name);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}


/*! setArgFloat(const __FlashStringHelper * name, float value)
@brief Sets float value of a field
@details Sets float value of an HTML field. It can free up RAM memory if string constants are stored in flash instead of RAM.
@param name
  The name of the field (stored in flash)
@param value
  Float value
@par Supported HTML controls
@li  INPUT
@warning Use this method only in refreshCb/loadCb.
@par Example HTML
@code
  <form>
    <input name="input" type="number" step="0.01"/>
    <input type="submit">
  </form>
@endcode
@par Example code
@code
  void loadCb(const char * url)
  {
    // sets the default value at loading
    webServer.setArgFloat( F("input"), 12.3f );
  }
@endcode
*/

void ELClientWebServer::setArgFloat(const __FlashStringHelper * name, float value)
{
  const char * name_p = reinterpret_cast<const char *>(name);

  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 7];
  buf[0] = WEB_FLOAT;
  strcpy_P(buf+1, name_p);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

/*! getArgInt()
@brief Returns an HTML field value as integer
@details Returns an HTML field value as integer. This method is used at setFieldCb. WebServer doesn't know the type of a field, every field arrives as string.
  This method converts the string to the expected type. Don't call this method outside of setFieldCb, otherwise it can reset/crash/freeze MCU.
@return Integer value of the field
@warning Use this method only in setFieldCb, otherwise crash, freeze, MCU reset, or other unexpected behaviour can happen.
@par Example
@code
  int32_t int_value = 0;

  // this method should be fast to prevent UART receive buffer overrun
  void setFieldCb(const char * field_str)
  {
    String field = field_str;
    if( field == F("field_id") )
    {
      int_value = webServer.getArgInt();
    }
  }
@endcode
*/

int32_t ELClientWebServer::getArgInt()
{
  return (int32_t)atol(arg_ptr);
}

/*! getArgString()
@brief Returns an HTML field value as string
@details Returns an HTML field value as string. Don't call this method outside of setFieldCb, otherwise it can reset/crash/freeze MCU.
@return String value of the field
@warning Use this method only in setFieldCb, otherwise crash, freeze, MCU reset, or other unexpected behaviour can happen.
@par Example
@code
  String str;

  // this method should be fast to prevent UART receive buffer overrun
  void setFieldCb(const char * field_str)
  {
    String field = field_str;
    if( field == F("field_id") )
    {
      str = webServer.getArgString();
    }
  }
@endcode
*/

char * ELClientWebServer::getArgString()
{
  return arg_ptr;
}

/*! getArgBoolean()
@brief Returns an HTML field value as boolean
@details Returns an HTML field value as boolean. This method is used at setFieldCb. WebServer doesn't know the type of a field, every field arrives as string.
  This method converts the string to the expected type. Don't call this method outside of setFieldCb, otherwise it can reset/crash/freeze MCU.
@return Boolean value of the field
@warning Use this method only in setFieldCb, otherwise crash, freeze, MCU reset, or other unexpected behaviour can happen.
@par Example
@code
  uint8_t boolean_value;

  // this method should be fast to prevent UART receive buffer overrun
  void setFieldCb(const char * field_str)
  {
    String field = field_str;
    if( field == F("field_id") )
    {
      boolean_value = webServer.getArgBoolean();
    }
  }
@endcode
*/

uint8_t ELClientWebServer::getArgBoolean()
{
  if( strcmp_P(arg_ptr, PSTR("on")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("true")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("yes")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("1")) == 0 )
    return 1;
  return 0;
}

/*! getArgFloat()
@brief Returns an HTML field value as float
@details Returns an HTML field value as float. This method is used at setFieldCb. WebServer doesn't know the type of a field, every field arrives as string.
  This method converts the string to the expected type. Don't call this method outside of setFieldCb, otherwise it can reset/crash/freeze MCU.
@return Float value of the field
@warning Use this method only in setFieldCb, otherwise crash, freeze, MCU reset, or other unexpected behaviour can happen.
@par Example
@code
  float float_value;

  // this method should be fast to prevent UART receive buffer overrun
  void setFieldCb(const char * field_str)
  {
    String field = field_str;
    if( field == F("field_id") )
    {
      float_value = webServer.getArgFloat();
    }
  }
@endcode
*/

float ELClientWebServer::getArgFloat()
{
  return atof(arg_ptr);
}
