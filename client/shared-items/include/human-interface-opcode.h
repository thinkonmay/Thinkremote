#ifndef __HUMAN_INTERFACE_OPCODE_H__
#define __HUMAN_INTERFACE_OPCODE_H__


/*HID data channel opcode*/
typedef enum
{
	KEYRAW,
	MOUSERAW,

	MOUSE_WHEEL,

	GAMEPAD_IN,
	GAMEPAD_OUT,

}HidOpcode;


#endif