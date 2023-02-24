#include <stdio.h> 
#include <conio.h>
#include <windows.h>
#include "teVirtualMIDI.h"

#define MAX_SYSEX_BUFFER	65535

HANDLE Port; //handle for COM port
LPVM_MIDI_PORT port; //handle for MIDI port

void initComPort() {
	Port = CreateFile("COM4", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (Port == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, "Ќевозможно открыть последовательный порт", "Error", MB_OK);
		ExitProcess(1);
	}

	/* SET COMMTIMEOUTS*/

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 1000;

	SetCommTimeouts(Port, &CommTimeOuts);

	/* SET COM DCB TO 57600 */

	DCB ComDCM;
	memset(&ComDCM, 0, sizeof(ComDCM));
	ComDCM.DCBlength = sizeof(DCB);
	GetCommState(Port, &ComDCM);
	ComDCM.BaudRate = 57600;
	ComDCM.ByteSize = 8;
	ComDCM.Parity = NOPARITY;
	ComDCM.StopBits = ONESTOPBIT;
	ComDCM.fAbortOnError = TRUE;
	ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
	ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
	ComDCM.fBinary = TRUE;
	ComDCM.fParity = FALSE;
	ComDCM.fInX = FALSE;
	ComDCM.fOutX = FALSE;
	ComDCM.XonChar = 0;
	ComDCM.XoffChar = (unsigned char)0xFF;
	ComDCM.fErrorChar = FALSE;
	ComDCM.fNull = FALSE;
	ComDCM.fOutxCtsFlow = FALSE;
	ComDCM.fOutxDsrFlow = FALSE;
	ComDCM.XonLim = 128;
	ComDCM.XoffLim = 128;

	SetCommState(Port, &ComDCM);

	printf("COM port opened sucessfully, press any key to close \n");
}

void initMidiPort() {
	printf("using virtualMIDI driver-version: %ws\n", virtualMIDIGetDriverVersion(NULL, NULL, NULL, NULL));
	port = virtualMIDICreatePortEx2(L"Arduino-MIDI control", NULL, 0, MAX_SYSEX_BUFFER, TE_VM_FLAGS_PARSE_RX);
	if (!port) {
		printf("could not create port: %d\n", GetLastError());
		return;
	}

//	virtualMIDILogging( TE_VM_LOGGING_MISC | TE_VM_LOGGING_RX | TE_VM_LOGGING_TX );
}

void sendMIDI(unsigned char *fdValue) {
	char length = 3; //MIDI CMD length
	byte myMidiData[3];

	myMidiData[0] = 0xB0; //CONTROL CHANGE, CHN 0
	myMidiData[1] = 0x01; //CHANGE FADER NO. 1 (MODULATION)
	myMidiData[2] = *fdValue; //FADER VALUE

	if (!virtualMIDISendData(port, myMidiData, length)) {
		printf("error sending data: %d\n" + GetLastError());
		return;
	}

}

void readValue() {
	unsigned char dst[1] = { 0 };
	unsigned long portsize = sizeof(dst);
	OVERLAPPED sync = { 0 };
	unsigned long read = 0, state = 0;

	SetCommMask(Port, EV_RXCHAR);

	while (1) {
		WaitCommEvent(Port, &state, &sync);
		if (state & EV_RXCHAR) {
			ReadFile(Port, dst, portsize, &read, &sync);
			PurgeComm(Port, PURGE_RXCLEAR);
			printf("Received: %d \n", dst[0]);
			sendMIDI(&dst[0]);
			state = 0;
		}
		if (_kbhit()) {
			break;
		}
	}

}

int main(void)
{
	SetConsoleTitle("Arduino COM-MIDI listener");

	initComPort(); //INIT COM4 AT 57600, as Port
	initMidiPort();

	readValue(); //BEGIN VALUE LISTENING


	printf("Listener closed\n");

	virtualMIDIClosePort(port);
	CloseHandle(Port);

	return 0;
}                               