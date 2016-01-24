#include "ELClientCmd.h"

ELClientCmd::ELClientCmd(ELClient* elc) :_elc(elc) {}

uint32_t ELClientCmd::GetTime() {
  uint16_t crc = _elc->Request(CMD_GET_TIME, 0, 1, 0);
  _elc->Request(crc);

  if (_elc->WaitReturn() == false || _elc->return_cmd != CMD_GET_TIME || _elc->return_value == 0)
    return 0;

  return _elc->return_value;
}

