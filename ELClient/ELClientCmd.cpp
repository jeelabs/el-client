#include "ELClientCmd.h"

ELClientCmd::ELClientCmd(ELClient* elc) :_elc(elc) {}

uint32_t ELClientCmd::GetTime() {
  uint16_t crc = _elc->Request(CMD_GET_TIME, 0, 1, 0);
  _elc->Request(crc);

  if (_elc->WaitReturn() == false || _elc->return_cmd != CMD_GET_TIME || _elc->return_value == 0)
    return 0;

  return _elc->return_value;
}

void ELClientCmd::WifiInit() {
  uint16_t crc;
  crc = _elc->Request(CMD_WIFI_INIT, (uint32_t)&wifiCb, 0, 0);
  _elc->Request(crc);
}

void ELClientCmd::ClearCallbacks() {
  uint16_t crc = _elc->Request(CMD_CLEAR_CBS, 0, 0, 0);
  _elc->Request(crc);
}

boolean ELClientCmd::IsReady() {
  uint32_t wait;

  for (uint8_t wait_time = 5; wait_time>0; wait_time--){
    _elc->is_return = false;
    _elc->return_value = 0;
    uint16_t crc = _elc->Request(CMD_IS_READY, 0, 1, 0);
    _elc->Request(crc);
    wait = millis();
    while (_elc->is_return == false && (millis() - wait < 1000)) {
      _elc->Run();
    }
    if (_elc->is_return && _elc->return_value)
      return true;
  }
  return false;
}