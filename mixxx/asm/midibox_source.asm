;
; Source-Code for MIDI-Faderbox
; Copyright 1998 by Thorsten Klose
; Commercial useage without my permission is strictly forbidden!
;
; EMail: Thorsten.Klose@gmx.de
; Homepage: http://go.to/uCApps
;

	LIST P=PIC16F84, R=DEC

#include <p16f84.inc>

#define DEBUG_MODE 0

RELOAD_VALUE	EQU	256-80+7	; Reload Value for Timer0

ScratchPadRAM	EQU	0x0c
EEPROM		EQU	0x00

MAX_PORT	EQU	PORTA
MAX_TRIS	EQU	TRISA
MAX_INIT_TRIS	EQU	0x03		; MAX_OUT and MAX_STROBE are inputs

MAX_DOUT	EQU	0x00
MAX_STROBE	EQU	0x01
MAX_DIN		EQU	0x02
MAX_CS		EQU	0x03
MAX_CLK		EQU	0x04

PIN_LED		EQU	0x04		; LED-Pin

KEY_INPUT_PORT	EQU	PORTB		; we use the first 4 bits!

MODE_LAYER1	EQU	0x00
MODE_LAYER2	EQU	0x01
MODE_AWE1	EQU	0x02
MODE_AWE2	EQU	0x03
MODE_AWE3	EQU	0x04
MODE_AWE4	EQU	0x05


BoxMode		EQU	ScratchPadRAM+0

W_TMP		EQU	ScratchPadRAM+1
STATUS_TMP	EQU	ScratchPadRAM+2

SerBits		EQU	ScratchPadRAM+3
SerData0	EQU	ScratchPadRAM+4
SerData1	EQU	ScratchPadRAM+5

SerBytes	EQU	ScratchPadRAM+6
SerByte0	EQU	ScratchPadRAM+7
SerByte1	EQU	ScratchPadRAM+8
SerByte2	EQU	ScratchPadRAM+9
SerByte3	EQU	ScratchPadRAM+10
SerByte4	EQU	ScratchPadRAM+11
SerByte5	EQU	ScratchPadRAM+12
SerByte6	EQU	ScratchPadRAM+13

ADC_Mode	EQU	ScratchPadRAM+14
ADC_ResultLo	EQU	ScratchPadRAM+15
ADC_ResultHi	EQU	ScratchPadRAM+16
ADC_Counter	EQU	ScratchPadRAM+17

Last_Pot0	EQU	ScratchPadRAM+18 	; Stored last Pot Values
Last_Pot1	EQU	ScratchPadRAM+19
Last_Pot2	EQU	ScratchPadRAM+20
Last_Pot3	EQU	ScratchPadRAM+21
Last_Pot4	EQU	ScratchPadRAM+22
Last_Pot5	EQU	ScratchPadRAM+23
Last_Pot6	EQU	ScratchPadRAM+24
Last_Pot7	EQU	ScratchPadRAM+25

_Last_Pot0	EQU	ScratchPadRAM+26	; "Real" last Pot Values
_Last_Pot1	EQU	ScratchPadRAM+27
_Last_Pot2	EQU	ScratchPadRAM+28
_Last_Pot3	EQU	ScratchPadRAM+29
_Last_Pot4	EQU	ScratchPadRAM+30
_Last_Pot5	EQU	ScratchPadRAM+31
_Last_Pot6	EQU	ScratchPadRAM+32
_Last_Pot7	EQU	ScratchPadRAM+33

NRPN_Data_Lo	EQU	ScratchPadRAM+34
NRPN_Data_Hi	EQU	ScratchPadRAM+35

KeyStatus	EQU	ScratchPadRAM+36
Act_KeyStatus	EQU	ScratchPadRAM+37
Last_KeyStatus	EQU	ScratchPadRAM+38

MIDI_Chn	EQU	ScratchPadRAM+39


;; ==========================================================================
;;  Macros
;; ==========================================================================

CHECKPOT MACRO adc_mode, sendpotvalue, last_pot, real_last_pot, pot_nr
	LOCAL	NoNewValue
	
	movlw	adc_mode		; select ADC Mode
	call	Conversion		; do AD conversion

	movf	ADC_ResultHi, W		; check if new value
	subwf	real_last_pot, W
	bz	NoNewValue

	movf	ADC_ResultHi, W		; store new value
	movwf	real_last_pot
	movwf	last_pot
	movwf	NRPN_Data_Lo
	clrf	NRPN_Data_Hi

	movlw	pot_nr
	call	CheckIndicator

	call	sendpotvalue

NoNewValue
	ENDM

CHECKKEY MACRO	setkeymode, last_keystatus
	LOCAL	NotifyNewStatus
	LOCAL	NoStatusChange

	rrf	Last_KeyStatus, 1	; typed key?
	bc	NotifyNewStatus
	rrf	Act_KeyStatus, 1	; we dont need it...
	goto	NoStatusChange

NotifyNewStatus
	rrf	Act_KeyStatus, 1
	btfsc	STATUS, C		; no MIDI msg if key released
	call	setkeymode
NoStatusChange
	ENDM


; Reset vector
	ORG	0
	goto	Start

;; ==========================================================================
;;  Timer IRQ
;; ==========================================================================

; IRQ vector
	ORG	4

TimerIRQ
	movwf	W_TMP
	movf	STATUS, W
	movwf	STATUS_TMP

	movlw	RELOAD_VALUE		; reload T0
	movwf	TMR0
	bcf	INTCON, T0IF		; clear Timer0 IRQ Flag

	movf	SerBits, W		; anything to send?
	bz	TI_End
	
	decf	SerBits, 1		; all bits send?
	bnz	TI_SendBit

	movf	SerBytes, W		; any byte to send?
	bz	TI_End			; no...

	call	Get_Next_MIDI_Byte	; prepare next byte
	goto	TI_End

TI_SendBit
	rlf	SerData1, 1
	rlf	SerData0, 1
	bc	TI_SendOne
TI_SendZero
	bcf	PORTB, 7
	goto	TI_End

TI_SendOne
	bsf	PORTB, 7

TI_End
	movf	STATUS_TMP, W
	movwf	STATUS
	movf	W_TMP, 1
	retfie


;; ==========================================================================
;;  Must be on the first 256-byte-segment because of 8-bit-PCL-Problem
;; ==========================================================================

SendPot0Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot0_Mode0
	goto	SendPot0_Mode1
	goto	SendPot0_Mode2
	goto	SendPot0_Mode3
	goto	SendPot0_Mode4
	goto	SendPot0_Mode5

SendPot1Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot1_Mode0
	goto	SendPot1_Mode1
	goto	SendPot1_Mode2
	goto	SendPot1_Mode3
	goto	SendPot1_Mode4
	goto	SendPot1_Mode5

SendPot2Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot2_Mode0
	goto	SendPot2_Mode1
	goto	SendPot2_Mode2
	goto	SendPot2_Mode3
	goto	SendPot2_Mode4
	goto	SendPot2_Mode5

SendPot3Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot3_Mode0
	goto	SendPot3_Mode1
	goto	SendPot3_Mode2
	goto	SendPot3_Mode3
	goto	SendPot3_Mode4
	goto	SendPot3_Mode5

SendPot4Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot4_Mode0
	goto	SendPot4_Mode1
	goto	SendPot4_Mode2
	goto	SendPot4_Mode3
	goto	SendPot4_Mode4
	goto	SendPot4_Mode5

SendPot5Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot5_Mode0
	goto	SendPot5_Mode1
	goto	SendPot5_Mode2
	goto	SendPot5_Mode3
	goto	SendPot5_Mode4
	goto	SendPot5_Mode5

SendPot6Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot6_Mode0
	goto	SendPot6_Mode1
	goto	SendPot6_Mode2
	goto	SendPot6_Mode3
	goto	SendPot6_Mode4
	goto	SendPot6_Mode5

SendPot7Value
	movf	BoxMode, W
	addwf	PCL, 1
	goto	SendPot7_Mode0
	goto	SendPot7_Mode1
	goto	SendPot7_Mode2
	goto	SendPot7_Mode3
	goto	SendPot7_Mode4
	goto	SendPot7_Mode5

GetResetValue
	movwf	W_TMP
	movf	BoxMode, W
	addwf	PCL, 1
	goto	Layer1_ResetTable
	goto	Layer2_ResetTable
	goto	AWE1_ResetTable
	goto	AWE2_ResetTable
	goto	AWE3_ResetTable
	goto	AWE4_ResetTable

Layer1_ResetTable
Layer2_ResetTable
	retlw	0x00

AWE1_ResetTable
	movf	W_TMP, W
	addwf	PCL, 1
	retlw	0x7f
	retlw	0x40
	retlw	0x7f
	retlw	0x00
	retlw	0x7f
	retlw	0x00
	retlw	0x00
	retlw	0x00
AWE2_ResetTable
	movf	W_TMP, W
	addwf	PCL, 1
	retlw	0x00
	retlw	0x00
	retlw	0x40
	retlw	0x00
	retlw	0x40
	retlw	0x00
	retlw	0x00
	retlw	0x40
AWE3_ResetTable
	movf	W_TMP, W
	addwf	PCL, 1
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x40
	retlw	0x40
AWE4_ResetTable
	movf	W_TMP, W
	addwf	PCL, 1
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00


;; ==========================================================================
;;  MIDI Queue Handling
;; ==========================================================================

Get_Next_MIDI_Byte
	clrf	SerData0
	clrf	SerData1

	bsf	STATUS, C		; push Stop Bit
	rrf	SerData0, 1

	rlf	SerByte0, 1		; push Data (MSB first)
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1

	rlf	SerByte0, 1
	rrf	SerData0, 1
	rrf	SerData1, 1

	bcf	STATUS, C		; push Start Bit
	rrf	SerData0, 1
	rrf	SerData1, 1

	movf	SerByte1, W		; bytes one to the left
	movwf	SerByte0
	movf	SerByte2, W
	movwf	SerByte1
	movf	SerByte3, W
	movwf	SerByte2
	movf	SerByte4, W
	movwf	SerByte3
	movf	SerByte5, W
	movwf	SerByte4
	movf	SerByte6, W
	movwf	SerByte5

	decf	SerBytes, 1		; one byte prepared

	movlw	0x0a+1			; ten bits to send
	movwf	SerBits
	
	return


;; ==========================================================================
;;  Send out MIDI Messages, if MIDI-Queue initialized
;; ==========================================================================

FlushMIDIQueue:
	call	Get_Next_MIDI_Byte	; prepare first byte in queue

	bsf	INTCON, GIE		; enable IRQs
SendLoop
	tstf	SerBits			; anything to send?
	bnz	SendLoop

	bcf	INTCON, GIE		; disable IRQs

	return

;; ==========================================================================
;;  Some Util Functions for shorter Code
;; ==========================================================================

Init_MIDI_Send_Control_Cmd
	movf	MIDI_Chn, W		; get control instr. with midi channel
	addlw	0xb0
	movwf	SerByte0
	return

Init_MIDI_NRPN_mult_2
	clrc
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1
	return

Init_MIDI_NRPN_mult_16
	clrc
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1
	return

Init_MIDI_NRPN_split_40
	movlw	0x40
	subwf	NRPN_Data_Lo, 1
	btfss	STATUS, C
	decf	NRPN_Data_Hi, 1
	return

Init_MIDI_NRPN_split_80
	clrc
	rlf	NRPN_Data_Lo, 1
	rlf	NRPN_Data_Hi, 1

	movlw	0x80
	subwf	NRPN_Data_Lo, 1
	btfss	STATUS, C
	decf	NRPN_Data_Hi, 1
	return


;; ==========================================================================
;;  Main Program
;; ==========================================================================

Start

	; Initialize Ports
	movlw	0xff			; PullUP PortB
	movwf	PORTB
	movwf	MAX_PORT
	bsf	STATUS, RP0 		; switch to bank 1
	movlw	0x6f			; Pin B.7 and B.4 as permanent output
	movwf	TRISB
	movlw	MAX_INIT_TRIS
	movwf	MAX_TRIS

	; Initialize Timer0
	bcf	OPTION_REG, T0CS	; start T0
	bsf	INTCON, T0IE		; T0 overflow IRQ enable

	bcf	STATUS, RP0 		; switch to bank 0

	; Initialize Ser
	clrf	SerBits

	; Initialize Status Bytes
	clrf	Last_Pot0
	clrf	Last_Pot1
	clrf	Last_Pot2
	clrf	Last_Pot3
	clrf	Last_Pot4
	clrf	Last_Pot5
	clrf	Last_Pot6
	clrf	Last_Pot7

	clrf	_Last_Pot0
	clrf	_Last_Pot1
	clrf	_Last_Pot2
	clrf	_Last_Pot3
	clrf	_Last_Pot4
	clrf	_Last_Pot5
	clrf	_Last_Pot6
	clrf	_Last_Pot7

	clrf	Last_KeyStatus

	; Init Box Mode
	movlw	MODE_LAYER1
	movwf	BoxMode

	; reload T0
	movlw	RELOAD_VALUE
	movwf	TMR0

	; switch of Control LED
	bsf	PORTB, PIN_LED

Loop

	call	GetKeys

	movf	KeyStatus, W
	movwf	Act_KeyStatus			; save actual KeyStatus for rrf's
	xorwf	Last_KeyStatus, 1		; get changes from KeyStatus

	CHECKKEY SetKeyMode0, Last_Key0
	CHECKKEY SetKeyMode1, Last_Key1
	CHECKKEY SetKeyMode2, Last_Key2
	CHECKKEY SetKeyMode3, Last_Key3
	CHECKKEY SetKeyMode4, Last_Key4
	CHECKKEY SetKeyMode5, Last_Key5
	CHECKKEY SetKeyMode6, Last_Key6
	CHECKKEY SetKeyMode7, Last_Key7

	movf	KeyStatus, W			; store KeyStatus
	movwf	Last_KeyStatus
	
	CHECKPOT B'10001110', SendPot0Value, Last_Pot0, _Last_Pot0, 0x00
	CHECKPOT B'11001110', SendPot1Value, Last_Pot1, _Last_Pot1, 0x01
	CHECKPOT B'10011110', SendPot2Value, Last_Pot2, _Last_Pot2, 0x02
	CHECKPOT B'11011110', SendPot3Value, Last_Pot3, _Last_Pot3, 0x03
	CHECKPOT B'10101110', SendPot4Value, Last_Pot4, _Last_Pot4, 0x04
	CHECKPOT B'11101110', SendPot5Value, Last_Pot5, _Last_Pot5, 0x05
	CHECKPOT B'10111110', SendPot6Value, Last_Pot6, _Last_Pot6, 0x06
	CHECKPOT B'11111110', SendPot7Value, Last_Pot7, _Last_Pot7, 0x07


CheckKeys
	goto	Loop


;; ==========================================================================
;;  Conversion Routine fuer MAX186
;; ==========================================================================

Conversion
	movwf	ADC_Mode

	bcf	MAX_PORT, MAX_CLK	; Clock to zero
	nop
	bcf	MAX_PORT, MAX_CS	; Chip Select

	movlw	0x08			; 8 bits to send
	movwf	ADC_Counter

Con_SendControlByte
	bcf	MAX_PORT, MAX_CLK	; clock to zero
	rlf	ADC_Mode, 1
	bc	Con_SetControlBit
Con_ClearControlBit
	bcf	MAX_PORT, MAX_DIN
	goto	Con_ContrContinue
Con_SetControlBit
	bsf	MAX_PORT, MAX_DIN
Con_ContrContinue
	nop
	bsf	MAX_PORT, MAX_CLK	; clock to one
	decf	ADC_Counter, 1
	bnz	Con_SendControlByte

	bcf	MAX_PORT, MAX_DIN	; clear DIN
	nop

	;	wait until stobe is zero
Con_WaitStrobe
	btfss	MAX_PORT, MAX_STROBE
	goto	Con_WaitStrobe

	;	get data bytes
	
	movlw	0x10			; 16 bits to receive
	movwf	ADC_Counter

	bsf	MAX_PORT, MAX_CLK	; clock to one
	nop
	nop
	nop
Con_GetData
	bcf	MAX_PORT, MAX_CLK	; clock to zero
	nop
	btfsc	MAX_PORT, MAX_DOUT
	goto	Max_DataBitSet
	clrc
	goto	Max_DataContinue
Max_DataBitSet
	setc
Max_DataContinue
	rlf	ADC_ResultLo, 1
	rlf	ADC_ResultHi, 1

	bsf	MAX_PORT, MAX_CLK	; clock to one
	decf	ADC_Counter, 1
	bnz	Con_GetData
	
	bcf	MAX_PORT, MAX_CLK	; clock to zero
	nop
	bsf	MAX_PORT, MAX_CS	; Chip DeSelect
	
	return


;; ==========================================================================
;;  Get Status of all Keys and Switches
;; ==========================================================================

GetKeys
	clrf	KeyStatus			; new Key Status

	; select keys 0-3
	bsf	STATUS, RP0 			; switch to bank 1
	bsf	TRISB, 0x06
	bsf	TRISA, 0x02
	bcf	TRISB, 0x05
	bcf	STATUS, RP0 			; switch to bank 0
	bcf	PORTB, 0x06
	bcf	PORTA, 0x02
	bsf	PORTB, 0x05
	nop
	nop
	nop
	nop

	movf	KEY_INPUT_PORT, W		; get key data
	movwf	STATUS_TMP			; store it into tmp register

	rrf	STATUS_TMP, 1			; shift data out
	rlf	KeyStatus, 1
	rrf	STATUS_TMP, 1
	rlf	KeyStatus, 1
	rrf	STATUS_TMP, 1
	rlf	KeyStatus, 1
	rrf	STATUS_TMP, 1
	rlf	KeyStatus, 1


	; select keys 4-7
	bsf	STATUS, RP0 			; switch to bank 1
	bsf	TRISB, 0x06
	bcf	TRISA, 0x02
	bsf	TRISB, 0x05
	bcf	STATUS, RP0 			; switch to bank 0
	bcf	PORTB, 0x06
	bsf	PORTA, 0x02
	bcf	PORTB, 0x05
	nop
	nop
	nop
	nop

	movf	KEY_INPUT_PORT, W		; get key data
	movwf	STATUS_TMP			; store it into tmp register

	rrf	STATUS_TMP, 1			; shift data out
	rlf	KeyStatus, 1
	rrf	STATUS_TMP, 1
	rlf	KeyStatus, 1
	rrf	STATUS_TMP, 1
	rlf	KeyStatus, 1
	rrf	STATUS_TMP, 1
	rlf	KeyStatus, 1

	clrf	MIDI_Chn				; new Program Mode
	; select Program Switches
	bsf	STATUS, RP0 			; switch to bank 1
	bcf	TRISB, 0x06
	bsf	TRISA, 0x02
	bsf	TRISB, 0x05
	bcf	STATUS, RP0 			; switch to bank 0
	bsf	PORTB, 0x06
	bcf	PORTA, 0x02
	bcf	PORTB, 0x05
	nop
	nop
	nop
	nop

	movf	KEY_INPUT_PORT, W		; get key data
	movwf	STATUS_TMP			; store it into tmp register
	clrf	MIDI_Chn			; Clear MIDI Channel Mask

	rrf	STATUS_TMP, 1			; shift data into MIDI Channel Mask
	rlf	MIDI_Chn, 1
	rrf	STATUS_TMP, 1
	rlf	MIDI_Chn, 1
	rrf	STATUS_TMP, 1
	rlf	MIDI_Chn, 1
	rrf	STATUS_TMP, 1
	rlf	MIDI_Chn, 1

	; Reset Tristate Bits
	bsf	STATUS, RP0 			; switch to bank 1
	bsf	TRISB, 0x06
	bcf	TRISA, 0x02
	bsf	TRISB, 0x05
	bcf	STATUS, RP0 			; switch to bank 0

	return


;; ==========================================================================
;;  Key Mode Handling
;; ==========================================================================

SetKeyMode0
	movlw	MODE_LAYER1
	movwf	BoxMode
	goto	LoadPotStatus
SetKeyMode1
	movlw	MODE_LAYER2
	movwf	BoxMode
	goto	LoadPotStatus
SetKeyMode2
	movlw	MODE_AWE1
	movwf	BoxMode
	call	InitNRPN
	goto	LoadPotStatus
SetKeyMode3
	movlw	MODE_AWE2
	movwf	BoxMode
	call	InitNRPN
	goto	LoadPotStatus
SetKeyMode4
	movlw	MODE_AWE3
	movwf	BoxMode
	call	InitNRPN
	goto	LoadPotStatus
SetKeyMode5
	movlw	MODE_AWE4
	movwf	BoxMode
	call	InitNRPN
	goto	LoadPotStatus
SetKeyMode6
	goto	StorePotStatus
SetKeyMode7
	goto	ResetPotSettings

InitNRPN
	movlw	0x03			; 3 bytes to send
	movwf	SerBytes
	call	Init_MIDI_Send_Control_Cmd
	movlw	0x63			; set NRPN MSB to 0x7f
	movwf	SerByte1
	movlw	0x7f
	movwf	SerByte2
	goto	FlushMIDIQueue		; flush queue and return from subroutine

;; ==========================================================================
;;  Fader Handling
;; ==========================================================================


SendPot0_Mode0
	movlw	0x10
	goto	Send_MIDI_Controller
SendPot0_Mode1
	movlw	0x18
	goto	Send_MIDI_Controller
SendPot0_Mode2
	movlw	0x07
	goto	Send_MIDI_Controller
SendPot0_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x00
	goto	Send_MIDI_NRPN
SendPot0_Mode4
	call	Init_MIDI_NRPN_mult_16
	movlw	0x04
	goto	Send_MIDI_NRPN
SendPot0_Mode5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0a
	goto	Send_MIDI_NRPN


SendPot1_Mode0
	movlw	0x11
	goto	Send_MIDI_Controller
SendPot1_Mode1
	movlw	0x19
	goto	Send_MIDI_Controller
SendPot1_Mode2
	movlw	0x0a
	goto	Send_MIDI_Controller
SendPot1_Mode3
	movlw	0x01
	goto	Send_MIDI_NRPN
SendPot1_Mode4
	call	Init_MIDI_NRPN_mult_16
	movlw	0x05
	goto	Send_MIDI_NRPN
SendPot1_Mode5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0b
	goto	Send_MIDI_NRPN


SendPot2_Mode0
	movlw	0x12
	goto	Send_MIDI_Controller
SendPot2_Mode1
	movlw	0x1a
	goto	Send_MIDI_Controller
SendPot2_Mode2
	movlw	0x0b
	goto	Send_MIDI_Controller
SendPot2_Mode3
	call	Init_MIDI_NRPN_split_80
	movlw	0x11
	goto	Send_MIDI_NRPN
SendPot2_Mode4
	call	Init_MIDI_NRPN_mult_16
	movlw	0x06
	goto	Send_MIDI_NRPN
SendPot2_Mode5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0c
	goto	Send_MIDI_NRPN


SendPot3_Mode0
	movlw	0x13
	goto	Send_MIDI_Controller
SendPot3_Mode1
	movlw	0x1b
	goto	Send_MIDI_Controller
SendPot3_Mode2
	movlw	0x01
	goto	Send_MIDI_Controller
SendPot3_Mode3
	movlw	0x14
	goto	Send_MIDI_NRPN
SendPot3_Mode4
	call	Init_MIDI_NRPN_mult_16
	movlw	0x07
	goto	Send_MIDI_NRPN
SendPot3_Mode5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0d
	goto	Send_MIDI_NRPN


SendPot4_Mode0
	movlw	0x14
	goto	Send_MIDI_Controller
SendPot4_Mode1
	movlw	0x1c
	goto	Send_MIDI_Controller
SendPot4_Mode2
	movlw	0x15
	goto	Send_MIDI_NRPN
SendPot4_Mode3
	call	Init_MIDI_NRPN_split_40
	movlw	0x17
	goto	Send_MIDI_NRPN
SendPot4_Mode4
	movlw	0x08
	goto	Send_MIDI_NRPN
SendPot4_Mode5
	movlw	0x0e
	goto	Send_MIDI_NRPN


SendPot5_Mode0
	movlw	0x15
	goto	Send_MIDI_Controller
SendPot5_Mode1
	movlw	0x1d
	goto	Send_MIDI_Controller
SendPot5_Mode2
	movlw	0x16
	goto	Send_MIDI_NRPN
SendPot5_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x02
	goto	Send_MIDI_NRPN
SendPot5_Mode4
	call	Init_MIDI_NRPN_mult_16
	movlw	0x09
	goto	Send_MIDI_NRPN
SendPot5_Mode5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0f
	goto	Send_MIDI_NRPN


SendPot6_Mode0
	movlw	0x16
	goto	Send_MIDI_Controller
SendPot6_Mode1
	movlw	0x1e
	goto	Send_MIDI_Controller
SendPot6_Mode2
	call	Init_MIDI_NRPN_mult_2
	movlw	0x19
	goto	Send_MIDI_NRPN
SendPot6_Mode3
	movlw	0x03
	goto	Send_MIDI_NRPN
SendPot6_Mode4
	call	Init_MIDI_NRPN_split_80
	movlw	0x13
	goto	Send_MIDI_NRPN
SendPot6_Mode5
	return


SendPot7_Mode0
	movlw	0x17
	goto	Send_MIDI_Controller
SendPot7_Mode1
	movlw	0x1f
	goto	Send_MIDI_Controller
SendPot7_Mode2
	call	Init_MIDI_NRPN_mult_2
	movlw	0x1a
	goto	Send_MIDI_NRPN
SendPot7_Mode3
	call	Init_MIDI_NRPN_split_80
	movlw	0x12
	goto	Send_MIDI_NRPN
SendPot7_Mode4
	call	Init_MIDI_NRPN_split_80
	movlw	0x18
	goto	Send_MIDI_NRPN
SendPot7_Mode5
	return




Send_MIDI_Controller
	movwf	SerByte1		; store Controller Number
	movlw	0x03			; 3 bytes to send
	movwf	SerBytes

	call	Init_MIDI_Send_Control_Cmd

	movf	NRPN_Data_Lo, W		; send pot value
	movwf	SerByte2

	goto	FlushMIDIQueue		; flush MIDI queue and return from subroutine

Send_MIDI_NRPN
	movwf	SerByte2		; store NRPN-Number (only LSB is needed)
	movlw	0x07			; 7 bytes to send
	movwf	SerBytes

	call	Init_MIDI_Send_Control_Cmd

	movlw	0x62			; send NRPN LSB
	movwf	SerByte1

	movlw	0x26			; send NRPN Data LSB
	movwf	SerByte3

	movlw	0x20			; add 0x2000 to NRPN
	addwf	NRPN_Data_Hi, 1
	movf	NRPN_Data_Lo, W		; make LSB
	andlw	0x7f
	movwf	SerByte4

	movlw	0x06			; send NRPN Data MSB
	movwf	SerByte5

	clrc				; make MSB
	rlf	NRPN_Data_Lo, W		; roll in bit 7 of LSB
	rlf	NRPN_Data_Hi, W
	andlw	0x7f
	movwf	SerByte6

	goto	FlushMIDIQueue		; flush MIDI queue and return from subroutine


;; ==========================================================================
;;  EEPROM Handling
;; ==========================================================================

PrepareForEEPROM
	movf	BoxMode, W		; get EEPROM Adr.
	movwf	W_TMP
	clrc
	rlf	W_TMP, 1
	rlf	W_TMP, 1
	rlf	W_TMP, W
	movwf	EEADR

	movlw	Last_Pot0		; set data-pointer
	movwf	FSR

	return

StorePotStatus
	call	PrepareForEEPROM

	movlw	0x08
	movwf	STATUS_TMP
StoreLoop
	bcf	PORTB, PIN_LED		; "access signal" for user

	call	_StorePotStatus
	decf	STATUS_TMP, 1
	bnz	StoreLoop

	bsf	PORTB, PIN_LED
	return

_StorePotStatus
	movf	INDF, W
	movwf	EEDATA

	bsf	STATUS, RP0
	bsf	EECON1, WREN
	movlw	0x55
	movwf	EECON2
	movlw	0xaa
	movwf	EECON2
	bsf	EECON1, WR
Write_EEPROM_Loop
	btfsc	EECON1, 1
	goto	Write_EEPROM_Loop
	bcf	STATUS, RP0

	incf	EEADR, 1
	incf	FSR, 1
	return

LoadPotStatus

	call	PrepareForEEPROM

	movlw	0x08
	movwf	STATUS_TMP
LoadLoop
	call	_LoadPotStatus
	decf	STATUS_TMP, 1
	bnz	LoadLoop

	goto FlushPotData

_LoadPotStatus
	bsf	STATUS, RP0
	bsf	EECON1, RD
	bcf	STATUS, RP0

	movf	EEDATA, W
	movwf	INDF

	incf	EEADR, 1
	incf	FSR, 1
	return

;; ==========================================================================
;;  Output Pot Settings
;; ==========================================================================

FlushPotData
	movf	Last_Pot0, W
	call	PrepareFlushPotData
	call	SendPot0Value

	movf	Last_Pot1, W
	call	PrepareFlushPotData
	call	SendPot1Value

	movf	Last_Pot2, W
	call	PrepareFlushPotData
	call	SendPot2Value

	movf	Last_Pot3, W
	call	PrepareFlushPotData
	call	SendPot3Value

	movf	Last_Pot4, W
	call	PrepareFlushPotData
	call	SendPot4Value

	movf	Last_Pot5, W
	call	PrepareFlushPotData
	call	SendPot5Value

	movf	Last_Pot6, W
	call	PrepareFlushPotData
	call	SendPot6Value

	movf	Last_Pot7, W
	call	PrepareFlushPotData
	goto	SendPot7Value

PrepareFlushPotData
	movwf	NRPN_Data_Lo
	clrf	NRPN_Data_Hi
	return
	
;; ==========================================================================
;;  Reset Pot Settings
;; ==========================================================================

ResetPotSettings
	movlw	0x00
	call	GetResetValue
	movwf	Last_Pot0
	movlw	0x01
	call	GetResetValue
	movwf	Last_Pot1
	movlw	0x02
	call	GetResetValue
	movwf	Last_Pot2
	movlw	0x03
	call	GetResetValue
	movwf	Last_Pot3
	movlw	0x04
	call	GetResetValue
	movwf	Last_Pot4
	movlw	0x05
	call	GetResetValue
	movwf	Last_Pot5
	movlw	0x06
	call	GetResetValue
	movwf	Last_Pot6
	movlw	0x07
	call	GetResetValue
	movwf	Last_Pot7
	goto	FlushPotData

;; ==========================================================================
;; Check if pot reached the reset value
;; ==========================================================================

CheckIndicator
	call	GetResetValue
	subwf	NRPN_Data_Lo, W
	bz	IndicatorOn
IndicatorOff
	bsf	PORTB, PIN_LED
	return
IndicatorOn
	bcf	PORTB, PIN_LED
	return

	END

