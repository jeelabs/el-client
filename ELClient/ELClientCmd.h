#ifndef _EL_CLIENT_CMD_H_
#define _EL_CLIENT_CMD_H_

#include <Arduino.h>
#include "ELClient.h"
#include "FP.h"

class ELClientCmd {
  public:
    ELClientCmd(ELClient* elc);
    uint32_t GetTime();
    void WifiInit();
    void ClearCallbacks();
    boolean IsReady();

    FP<void, void*> wifiCb;

  private:
    ELClient* _elc;
};
#endif _EL_CLIENT_CMD_H_