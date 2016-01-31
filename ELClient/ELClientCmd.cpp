// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClientCmd.h"

ELClientCmd::ELClientCmd(ELClient* elc) :_elc(elc) {}

uint32_t ELClientCmd::GetTime() {
  _elc->Request(CMD_GET_TIME, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}

