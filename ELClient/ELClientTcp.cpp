// Copyright (c) 2016 by BeeGee
// adapted from ELClientRest.cpp Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClientTcp.h"

ELClientTcp::ELClientTcp(ELClient *e) {
	_elc = e;
	remote_instance = -1;
}

void ELClientTcp::tcpCallback(void *res) {
	if (!res) return;

	ELClientResponse *resp = (ELClientResponse *)res;

	#ifdef DEBUG_EN
		int argNum = resp->argc();
		Serial.println("Number of arguments: "+String(argNum));
		uint16_t _cmd = resp->cmd();
		Serial.println("Command: "+String(_cmd));
		uint16_t _value = resp->value();
		Serial.println("Value: "+String(_value));
	#endif
	
	resp->popArg(&_resp_type, 1);
	resp->popArg(&_client_num, 1);
	resp->popArg(&_len, 2);
	#ifdef DEBUG_EN
		Serial.print("Type: ");
		Serial.print(_resp_type);
		Serial.print(" client: ");
		Serial.print(_client_num);
		Serial.print(" size: "+String(_len));
	#endif
	if (_resp_type == 1) {
		#ifdef DEBUG_EN
			int argLen = resp->argLen();
			Serial.print(" data length: "+String(argLen));
		#endif
		resp->popArgPtr(&_data);
		#ifdef DEBUG_EN
			_data[_len] = '\0';
			Serial.print(" data: "+String(_data));
		#endif
	}
	#ifdef DEBUG_EN
		Serial.println("");
	#endif
	_status = 1;
	if (_hasUserCb) {
		_userCb(_resp_type, _client_num, _len, _data);
	}
}

int ELClientTcp::begin(const char* host, uint16_t port, uint8_t sock_mode, void (*userCb)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)) {
	if (userCb != 0) {
		_userCb = userCb;
		_hasUserCb = true;
	}

	tcpCb.attach(this, &ELClientTcp::tcpCallback);

	_elc->Request(CMD_TCP_SETUP, (uint32_t)&tcpCb, 3);
	_elc->Request(host, strlen(host));
	_elc->Request(&port, 2);
	_elc->Request(&sock_mode, 1);
	_elc->Request();

	ELClientPacket *pkt = _elc->WaitReturn();

	if (pkt && (int32_t)pkt->value >= 0) {
		remote_instance = pkt->value;
		// return 0;
	}
	return (int)pkt->value;
}

void ELClientTcp::send(const char* data, int len) {
	_status = 0;
	if (remote_instance < 0) return;
	_elc->Request(CMD_TCP_SEND, remote_instance, 2);
	_elc->Request(data, strlen(data));
	if (data != NULL && len > 0) {
		_elc->Request(data, len);
	}

	_elc->Request();
}

void ELClientTcp::send(const char* data) {
	send(data, strlen(data));
}

uint16_t ELClientTcp::getResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen) {
	if (_status == 0) return 0;
	memcpy(data, _data, _len>maxLen?maxLen:_len);
	*resp_type = _resp_type;
	*client_num = _client_num;
	_status = 0;
	return _len;
}

uint16_t ELClientTcp::waitResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen, uint32_t timeout) {
	uint32_t wait = millis();
	while (_status == 0) {
		if ( millis() - wait < timeout) {
			_elc->Process();
		} else {
			return -3;
		}
	}
	return getResponse(resp_type, client_num, data, maxLen);
}
