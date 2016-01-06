// Miscellaneous commands
#ifndef _EL_CLIENT_CMD_H_
#define _EL_CLIENT_CMD_H_

#include <Arduino.h>
#include "ELClient.h"
#include "FP.h"

class ELClientCmd {
  public:
    // Constructor
    ELClientCmd(ELClient* elc);
    // Get the current time in seconds since the epoch, 0 if the time is unknown
    uint32_t GetTime();
    // ???
    void WifiInit();
    // Clear all callbacks from esp-link to Arduino
    void ClearCallbacks();
    // Check that ???
    boolean IsReady();

    FP<void, void*> wifiCb;

  private:
    ELClient* _elc;
};
#endif _EL_CLIENT_CMD_H_
