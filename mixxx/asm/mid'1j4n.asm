;
; Source-Code for MIDI-Knobbox Plus
; Copyright 1998, 1999 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;
; EMail: Thorsten.Klose@gmx.de
; Homepage: http://go.to/uCApps
;

	LIST P=PIC16F84, R=DEC

#include <p16f84.inc>

#define DEBUG_MODE 0

RELOAD_VALUE	EQU	0xff-0x48	; Reload Value for Timer0 (31.25 kBaud)

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

MODE_AWE1	EQU	0x00
MODE_AWE2	EQU	0x01
MODE_AWE3	EQU	0x02
MODE_AWE4	EQU	0x03
MODE_XG1	EQU	0x04
MODE_XG2	EQU	0x05
MODE_XG3	EQU	0x06
MODE_XG4	EQU	0x07
MODE_GEN1	EQU	0x08
MODE_GEN2	EQU	0x09
MODE_GEN3	EQU	0x0a
MODE_GEN4	EQU	0x0b
MODE_GEN5	EQU	0x0c
MODE_GEN6	EQU	0x0d



W_TMP		EQU	0x0c
STATUS_TMP	EQU	0x0d

ScratchPadRAM	EQU	0x0e

MIDI_Chn	EQU	ScratchPadRAM+0
ActPot		EQU	ScratchPadRAM+1
BoxMode		EQU	ScratchPadRAM+2
SuperBoxMode	EQU	ScratchPadRAM+3

SerBits		EQU	ScratchPadRAM+4
SerData0	EQU	ScratchPadRAM+5
SerData1	EQU	ScratchPadRAM+6

SerBytes	EQU	ScratchPadRAM+7
SerByte0	EQU	ScratchPadRAM+8
SerByte1	EQU	ScratchPadRAM+9
SerByte2	EQU	ScratchPadRAM+10
SerByte3	EQU	ScratchPadRAM+11
SerByte4	EQU	ScratchPadRAM+12
SerByte5	EQU	ScratchPadRAM+13
SerByte6	EQU	ScratchPadRAM+14
SerByte7	EQU	ScratchPadRAM+15
SerByte8	EQU	ScratchPadRAM+16

ADC_Mode	EQU	ScratchPadRAM+17
ADC_ResultLo	EQU	ScratchPadRAM+18
ADC_ResultHi	EQU	ScratchPadRAM+19
ADC_Counter	EQU	ScratchPadRAM+20

Last_Pot0	EQU	ScratchPadRAM+21 	; Stored last Pot Values
Last_Pot1	EQU	ScratchPadRAM+22
Last_Pot2	EQU	ScratchPadRAM+23
Last_Pot3	EQU	ScratchPadRAM+24
Last_Pot4	EQU	ScratchPadRAM+25
Last_Pot5	EQU	ScratchPadRAM+26
Last_Pot6	EQU	ScratchPadRAM+27
Last_Pot7	EQU	ScratchPadRAM+28

_Last_Pot0	EQU	ScratchPadRAM+29	; "Real" last Pot Values
_Last_Pot1	EQU	ScratchPadRAM+30
_Last_Pot2	EQU	ScratchPadRAM+31
_Last_Pot3	EQU	ScratchPadRAM+32
_Last_Pot4	EQU	ScratchPadRAM+33
_Last_Pot5	EQU	ScratchPadRAM+34
_Last_Pot6	EQU	ScratchPadRAM+35
_Last_Pot7	EQU	ScratchPadRAM+36

NRPN_Data_Lo	EQU	ScratchPadRAM+37
NRPN_Data_Hi	EQU	ScratchPadRAM+38

KeyStatus	EQU	ScratchPadRAM+39
Act_KeyStatus	EQU	ScratchPadRAM+40
Last_KeyStatus	EQU	ScratchPadRAM+41

;; ==========================================================================
;;  Macros
;; ==========================================================================

CHECKPOT MACRO adc_mode, last_pot, real_last_pot, pot_nr
	LOCAL	NoNewValue
	
	movlw	adc_mode		; select ADC Mode
	call	Conversion		; do AD conversion

	movf	ADC_ResultHi, W		; check if new value
	subwf	real_last_pot, W
	bz	NoNewValue

	movf	ADC_ResultHi, W		; store new value
	movwf	real_last_pot
	movwf	last_pot
	call    CHECKPOT_hlp

NoNewValue
	ENDM

CHECKKEY MACRO	setkeymode, last_keystatus
	LOCAL	NotifyNewStatus
	LOCAL	NoStatusChange

	rrf	Last_KeyStatus, F	; typed key?
	bc	NotifyNewStatus
	rrf	Act_KeyStatus, F	; we dont need it...
	goto	NoStatusChange

NotifyNewStatus
	rrf	Act_KeyStatus, F
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
	
	decf	SerBits, F		; all bits send?
	bnz	TI_SendBit

	movf	SerBytes, W		; any byte to send?
	bz	TI_End			; no...

	call	Get_Next_MIDI_Byte	; prepare next byte
	goto	TI_End

TI_SendBit
	rlf	SerData1, F
	rlf	SerData0, F
	bc	TI_SendOne
TI_SendZero
	bcf	PORTB, 7
	goto	TI_End

TI_SendOne
	bsf	PORTB, 7

TI_End
	movf	STATUS_TMP, W
	movwf	STATUS
	movf	W_TMP, W
	retfie


;; ==========================================================================
;;  Must be on the first 256-byte-segment because of 8-bit-PCL-Problem
;; ==========================================================================

GetMode
	addwf	PCL, F
	retlw	MODE_GEN3
	retlw	MODE_GEN4
	retlw	MODE_AWE1
	retlw	MODE_AWE2
	retlw	MODE_AWE3
	retlw	MODE_AWE4
	retlw	MODE_GEN3
	retlw	MODE_GEN4
	retlw	MODE_XG1
	retlw	MODE_XG2
	retlw	MODE_XG3
	retlw	MODE_XG4
	retlw	MODE_GEN1
	retlw	MODE_GEN2
	retlw	MODE_GEN3
	retlw	MODE_GEN4
	retlw	MODE_GEN5
	retlw	MODE_GEN6

SendPot
	movf	ActPot, W
	addwf	PCL, F
	goto	SendPot0Value
	goto	SendPot1Value
	goto	SendPot2Value
	goto	SendPot3Value
	goto	SendPot4Value
	goto	SendPot5Value
	goto	SendPot6Value
	goto	SendPot7Value

SendPot0Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot0_Mode0
	goto	SendPot0_Mode1
	goto	SendPot0_Mode2
	goto	SendPot0_Mode3
	goto	SendPot0_Mode4
	goto	SendPot0_Mode5
	goto	SendPot0_Mode6
	goto	SendPot0_Mode7

SendPot1Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot1_Mode0
	goto	SendPot1_Mode1
	goto	SendPot1_Mode2
	goto	SendPot1_Mode3
	goto	SendPot1_Mode4
	goto	SendPot1_Mode5
	goto	SendPot1_Mode6
	goto	SendPot1_Mode7

SendPot2Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot2_Mode0
	goto	SendPot2_Mode1
	goto	SendPot2_Mode2
	goto	SendPot2_Mode3
	goto	SendPot2_Mode4
	goto	SendPot2_Mode5
	goto	SendPot2_Mode6
	goto	SendPot2_Mode7

SendPot3Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot3_Mode0
	goto	SendPot3_Mode1
	goto	SendPot3_Mode2
	goto	SendPot3_Mode3
	goto	SendPot3_Mode4
	goto	SendPot3_Mode5
	goto	SendPot3_Mode6
	goto	SendPot3_Mode7

SendPot4Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot4_Mode0
	goto	SendPot4_Mode1
	goto	SendPot4_Mode2
	goto	SendPot4_Mode3
	goto	SendPot4_Mode4
	goto	SendPot4_Mode5
	goto	SendPot4_Mode6
	goto	SendPot4_Mode7

SendPot5Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot5_Mode0
	goto	SendPot5_Mode1
	goto	SendPot5_Mode2
	goto	SendPot5_Mode3
	goto	SendPot5_Mode4
	goto	SendPot5_Mode5
	goto	SendPot5_Mode6
	goto	SendPot5_Mode7

SendPot6Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot6_Mode0
	goto	SendPot6_Mode1
	goto	SendPot6_Mode2
	goto	SendPot6_Mode3
	goto	SendPot6_Mode4
	goto	SendPot6_Mode5
	goto	SendPot6_Mode6
	goto	SendPot6_Mode7

SendPot7Value
	movf	BoxMode, W
	addwf	PCL, F
	goto	SendPot7_Mode0
	goto	SendPot7_Mode1
	goto	SendPot7_Mode2
	goto	SendPot7_Mode3
	goto	SendPot7_Mode4
	goto	SendPot7_Mode5
	goto	SendPot7_Mode6
	goto	SendPot7_Mode7

GetResetValue
	movf	BoxMode, W
	addwf	PCL, F
	goto	AWE1_ResetTable
	goto	AWE2_ResetTable
	goto	AWE3_ResetTable
	goto	AWE4_ResetTable
	goto	XG1_ResetTable
	goto	XG2_ResetTable
	goto	XG3_ResetTable
	goto	XG4_ResetTable

AWE1_ResetTable
	movf	ActPot, W
	addwf	PCL, F
	retlw	0x7f
	retlw	0x40
	retlw	0x7f
	retlw	0x00
	retlw	0x7f
	retlw	0x00
	retlw	0x00
	retlw	0x00
AWE2_ResetTable
	movf	ActPot, W
	addwf	PCL, F
	retlw	0x00
	retlw	0x00
	retlw	0x40
	retlw	0x00
	retlw	0x40
	retlw	0x00
	retlw	0x00
	retlw	0x40
AWE3_ResetTable
AWE4_ResetTable
	movf	ActPot, W
	addwf	PCL, F
	retlw	0x00
	retlw	0x00
	retlw	0x7f
	retlw	0x7f
	retlw	0x7f
	retlw	0x00
	retlw	0x40
	retlw	0x40
;AWE4_ResetTable
;	movf	ActPot, W
;	addwf	PCL, F
;	retlw	0x00
;	retlw	0x00
;	retlw	0x7f
;	retlw	0x7f
;	retlw	0x7f
;	retlw	0x00
;	retlw	0x00
;	retlw	0x00
XG1_ResetTable
	movf	ActPot, W
	addwf	PCL, F
	retlw	0x7f
	retlw	0x40
	retlw	0x7f
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
	retlw	0x00
XG2_ResetTable
	retlw	0x40
;	movf	ActPot, W
;	addwf	PCL, F
;	retlw	0x40
;	retlw	0x40
;	retlw	0x40
;	retlw	0x40
;	retlw	0x40
;	retlw	0x40
;	retlw	0x40
;	retlw	0x40
XG3_ResetTable
	movf	ActPot, W
	addwf	PCL, F
	retlw	0x40
	retlw	0x40
	retlw	0x40
	retlw	0x40
	retlw	0x01
	retlw	0x7f
	retlw	0x00
	retlw	0x00
XG4_ResetTable
	movf	ActPot, W
	addwf	PCL, F
	retlw	0x42
	retlw	0x40
	retlw	0x40
	retlw	0x40
	retlw	0x40
	retlw	0x40
	retlw	0x00
	retlw	0x00

GetEEPROMAddr
	movf	BoxMode, W
	addwf	PCL, F
	retlw	0x10
	retlw	0x18
	retlw	0x20
	retlw	0x28
	retlw	0x10
	retlw	0x18
	retlw	0x20
	retlw	0x28
	retlw	0x00
	retlw	0x08
	retlw	0x10
	retlw	0x18
	retlw	0x20
	retlw	0x28

;; ==========================================================================
;;  MIDI Queue Handling
;; ==========================================================================

Get_Next_MIDI_Byte
	clrf	SerData0
	clrf	SerData1

	bsf	STATUS, C		; push Stop Bit
	rrf	SerData0, F

	rlf	SerByte0, F		; push Data (MSB first)
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F

	rlf	SerByte0, F
	rrf	SerData0, F
	rrf	SerData1, F

	bcf	STATUS, C		; push Start Bit
	rrf	SerData0, F
	rrf	SerData1, F

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
	movf	SerByte7, W
	movwf	SerByte6
	movf	SerByte8, W
	movwf	SerByte7

	decf	SerBytes, F		; one byte prepared

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

	; Initialize Registers
	movlw	ScratchPadRAM
	movwf	FSR
	movlw	0x50-ScratchPadRAM
	movwf	W_TMP
RegInitLoop
	clrf	INDF
	incf	FSR, F
	decfsz	W_TMP, F
	goto	RegInitLoop

	; reload T0
	movlw	RELOAD_VALUE
	movwf	TMR0


;; ==========================================================================
;;  Preset
;; ==========================================================================

PresetLoop
	; switch off Control LED
	bcf	PORTB, PIN_LED
	call	TestPresetKey
	bsf	PORTB, PIN_LED
	call	TestPresetKey
	goto	PresetLoop

TestPresetKey
	call	Delay
	call	GetKeys

	movf	KeyStatus, W
	movwf	Act_KeyStatus			; save actual KeyStatus for rrf's
	xorwf	Last_KeyStatus, F		; get changes from KeyStatus

	CHECKKEY SuperMode1, Last_Key0
	CHECKKEY SuperMode2, Last_Key1
	CHECKKEY SuperMode3, Last_Key2

	movf	KeyStatus, W			; store KeyStatus
	movwf	Last_KeyStatus

	return

SuperMode1
	movlw	0x00
	goto	SetSuperMode
SuperMode2
	movlw	0x06
	goto	SetSuperMode
SuperMode3
	movlw	0x0c
SetSuperMode
	movwf	SuperBoxMode

	call	GetMode
	movwf	BoxMode

	movf	KeyStatus, W			; store KeyStatus
	movwf	Last_KeyStatus

	call	Delay
	call	Delay
	call	Delay

;; ==========================================================================
;;  Main Loop
;; ==========================================================================

Loop

	call	GetKeys

	movf	KeyStatus, W
	movwf	Act_KeyStatus			; save actual KeyStatus for rrf's
	xorwf	Last_KeyStatus, F		; get changes from KeyStatus

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

	clrf	ActPot	
	CHECKPOT B'10001110', Last_Pot0, _Last_Pot0, 0x00
	incf	ActPot, F
	CHECKPOT B'11001110', Last_Pot1, _Last_Pot1, 0x01
	incf	ActPot, F
	CHECKPOT B'10011110', Last_Pot2, _Last_Pot2, 0x02
	incf	ActPot, F
	CHECKPOT B'11011110', Last_Pot3, _Last_Pot3, 0x03
	incf	ActPot, F
	CHECKPOT B'10101110', Last_Pot4, _Last_Pot4, 0x04
	incf	ActPot, F
	CHECKPOT B'11101110', Last_Pot5, _Last_Pot5, 0x05
	incf	ActPot, F
	CHECKPOT B'10111110', Last_Pot6, _Last_Pot6, 0x06
	incf	ActPot, F
	CHECKPOT B'11111110', Last_Pot7, _Last_Pot7, 0x07

	goto	Loop


CHECKPOT_hlp
	movwf	NRPN_Data_Lo
	clrf	NRPN_Data_Hi
	call	CheckIndicator
SendPotValue
	btfss	BoxMode, 3
	goto	SendPot
	goto	SendPotGen

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
	rlf	ADC_Mode, F
	bc	Con_SetControlBit
Con_ClearControlBit
	bcf	MAX_PORT, MAX_DIN
	goto	Con_ContrContinue
Con_SetControlBit
	bsf	MAX_PORT, MAX_DIN
Con_ContrContinue
	nop
	bsf	MAX_PORT, MAX_CLK	; clock to one
	decf	ADC_Counter, F
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
	rlf	ADC_ResultLo, F
	rlf	ADC_ResultHi, F

	bsf	MAX_PORT, MAX_CLK	; clock to one
	decf	ADC_Counter, F
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

	rrf	STATUS_TMP, F			; shift data out
	rlf	KeyStatus, F
	rrf	STATUS_TMP, F
	rlf	KeyStatus, F
	rrf	STATUS_TMP, F
	rlf	KeyStatus, F
	rrf	STATUS_TMP, F
	rlf	KeyStatus, F


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

	rrf	STATUS_TMP, F			; shift data out
	rlf	KeyStatus, F
	rrf	STATUS_TMP, F
	rlf	KeyStatus, F
	rrf	STATUS_TMP, F
	rlf	KeyStatus, F
	rrf	STATUS_TMP, F
	rlf	KeyStatus, F

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

	rrf	STATUS_TMP, F			; shift data into MIDI Channel Mask
	rlf	MIDI_Chn, F
	rrf	STATUS_TMP, F
	rlf	MIDI_Chn, F
	rrf	STATUS_TMP, F
	rlf	MIDI_Chn, F
	rrf	STATUS_TMP, F
	rlf	MIDI_Chn, F

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
	movlw	0x00
SetKeyMode_hlp
	addwf	SuperBoxMode, W
	call	GetMode
	movwf	BoxMode
	andlw	0xfc
	skpnz
	call	InitNRPN
	movf	BoxMode, W
	goto	LoadPotStatus
SetKeyMode1
	movlw	0x01
	goto	SetKeyMode_hlp
SetKeyMode2
	movlw	0x02
	goto	SetKeyMode_hlp
SetKeyMode3
	movlw	0x03
	goto	SetKeyMode_hlp
SetKeyMode4
	movlw	0x04
	goto	SetKeyMode_hlp
SetKeyMode5
	movlw	0x05
	goto	SetKeyMode_hlp
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


;; #### Layer 1, 2 und Gen3-6 ###

SendPotGen
	movf	BoxMode, W
	andlw	0x07
	movwf	W_TMP
	clrc
	rlf	W_TMP, F
	rlf	W_TMP, F
	rlf	W_TMP, W
	addwf	ActPot, W
	goto	Send_MIDI_Controller

;; #### Layer 3.AWE ###

SendPot0_Mode0
	movlw	0x07
	goto	Send_MIDI_Controller
SendPot1_Mode0
	movlw	0x0a
	goto	Send_MIDI_Controller
SendPot2_Mode0
	movlw	0x0b
	goto	Send_MIDI_Controller
SendPot3_Mode0
	movlw	0x01
	goto	Send_MIDI_Controller
SendPot4_Mode0
	movlw	0x15
	goto	Send_MIDI_NRPN
SendPot5_Mode0
	movlw	0x16
	goto	Send_MIDI_NRPN
SendPot6_Mode0
	call	Init_MIDI_NRPN_mult_2
	movlw	0x19
	goto	Send_MIDI_NRPN
SendPot7_Mode0
	call	Init_MIDI_NRPN_mult_2
	movlw	0x1a
	goto	Send_MIDI_NRPN

;; #### Layer 4.AWE ###

SendPot0_Mode1
	call	Init_MIDI_NRPN_mult_16
	movlw	0x00
	goto	Send_MIDI_NRPN
SendPot1_Mode1
	movlw	0x01
	goto	Send_MIDI_NRPN
SendPot2_Mode1
	call	Init_MIDI_NRPN_split_80
	movlw	0x11
	goto	Send_MIDI_NRPN
SendPot3_Mode1
	movlw	0x14
	goto	Send_MIDI_NRPN
SendPot4_Mode1
	call	Init_MIDI_NRPN_split_40
	movlw	0x17
	goto	Send_MIDI_NRPN
SendPot5_Mode1
	call	Init_MIDI_NRPN_mult_16
	movlw	0x02
	goto	Send_MIDI_NRPN
SendPot6_Mode1
	movlw	0x03
	goto	Send_MIDI_NRPN
SendPot7_Mode1
	call	Init_MIDI_NRPN_split_80
	movlw	0x12
	goto	Send_MIDI_NRPN

;; #### Layer 5.AWE ###

SendPot0_Mode2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x04
	goto	Send_MIDI_NRPN
SendPot1_Mode2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x05
	goto	Send_MIDI_NRPN
SendPot2_Mode2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x06
	goto	Send_MIDI_NRPN
SendPot3_Mode2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x07
	goto	Send_MIDI_NRPN
SendPot4_Mode2
	movlw	0x08
	goto	Send_MIDI_NRPN
SendPot5_Mode2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x09
	goto	Send_MIDI_NRPN
SendPot6_Mode2
	call	Init_MIDI_NRPN_split_80
	movlw	0x13
	goto	Send_MIDI_NRPN
SendPot7_Mode2
	call	Init_MIDI_NRPN_split_80
	movlw	0x18
	goto	Send_MIDI_NRPN

;; #### Layer 6.AWE ###

SendPot0_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0a
	goto	Send_MIDI_NRPN
SendPot1_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0b
	goto	Send_MIDI_NRPN
SendPot2_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0c
	goto	Send_MIDI_NRPN
SendPot3_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0d
	goto	Send_MIDI_NRPN
SendPot4_Mode3
	movlw	0x0e
	goto	Send_MIDI_NRPN
SendPot5_Mode3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0f
	goto	Send_MIDI_NRPN
SendPot6_Mode3
SendPot7_Mode3
	return

;; #### Layer 3.XG ###

SendPot0_Mode4
	movlw	0x07
	goto	Send_MIDI_Controller
SendPot1_Mode4
	movlw	0x0a
	goto	Send_MIDI_Controller
SendPot2_Mode4
	movlw	0x0b
	goto	Send_MIDI_Controller
SendPot3_Mode4
	movlw	0x01
	goto	Send_MIDI_Controller
SendPot4_Mode4
	movlw	0x20
	goto	Send_MIDI_NRPN
SendPot5_Mode4
	movlw	0x21
	goto	Send_MIDI_NRPN
;	goto	Send_MIDI_Portamento
SendPot6_Mode4
	movlw	0x5b
	goto	Send_MIDI_Controller
SendPot7_Mode4
	movlw	0x5d
	goto	Send_MIDI_Controller

;; #### Layer 4.XG ###

SendPot0_Mode5
	movlw	0x49
	goto	Send_MIDI_Controller
SendPot1_Mode5
	movlw	0x1b
	goto	Send_MIDI_SysEx
SendPot2_Mode5
	movlw	0x48
	goto	Send_MIDI_Controller
SendPot3_Mode5
	movlw	0x17
	goto	Send_MIDI_SysEx
SendPot4_Mode5
	movlw	0x15
	goto	Send_MIDI_SysEx
SendPot5_Mode5
	movlw	0x16
	goto	Send_MIDI_SysEx
SendPot6_Mode5
	movlw	0x4a
	goto	Send_MIDI_Controller
SendPot7_Mode5
	movlw	0x47
	goto	Send_MIDI_Controller

;; #### Layer 5.XG ###

SendPot0_Mode6
	movlw	0x69
	goto	Send_MIDI_SysEx
SendPot1_Mode6
	movlw	0x6a
	goto	Send_MIDI_SysEx
SendPot2_Mode6
	movlw	0x6b
	goto	Send_MIDI_SysEx
SendPot3_Mode6
	movlw	0x6c
	goto	Send_MIDI_SysEx
SendPot4_Mode6
	movlw	0x6d
	goto	Send_MIDI_SysEx
SendPot5_Mode6
	movlw	0x6e
	goto	Send_MIDI_SysEx
SendPot6_Mode6
SendPot7_Mode6
	return

;; #### Layer 6.XG ###

SendPot0_Mode7
	movlw	0x23
	goto	Send_MIDI_SysEx
SendPot1_Mode7
	movlw	0x24
	goto	Send_MIDI_SysEx
SendPot2_Mode7
	movlw	0x25
	goto	Send_MIDI_SysEx
SendPot3_Mode7
	movlw	0x26
	goto	Send_MIDI_SysEx
SendPot4_Mode7
	movlw	0x27
	goto	Send_MIDI_SysEx
SendPot5_Mode7
	movlw	0x28
	goto	Send_MIDI_SysEx
SendPot6_Mode7
SendPot7_Mode7
	return

;; ==========================================================================
;;  Fader Utils, Prepare MIDI
;; ==========================================================================


Send_MIDI_Controller
	movwf	SerByte1		; store Controller Number
	movlw	0x03			; 3 bytes to send
	movwf	SerBytes

	call	Init_MIDI_Send_Control_Cmd

	movf	NRPN_Data_Lo, W		; send pot value
	movwf	SerByte2

	goto	FlushMIDIQueue		; flush MIDI queue and return from subroutine

Send_MIDI_Portamento
	movlw	0x06			; store 2 Controller Numbers
	movwf	SerBytes
	call	Init_MIDI_Send_Control_Cmd
	movlw	0x41
	movwf	SerByte1
	movf	NRPN_Data_Lo, W
	movwf	SerByte5
	skpz
	movlw	0x7f
	movwf	SerByte2
	movf	SerByte0, W
	movwf	SerByte3
	movlw	0x05
	movwf	SerByte4
	goto	FlushMIDIQueue

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
	addwf	NRPN_Data_Hi, F
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


Send_MIDI_SysEx
	movwf	SerByte6		; store Controller Number
	movlw	0x09			; 9 bytes to send
	movwf	SerBytes

	movlw	0xf0
	movwf	SerByte0
	movlw	0x43
	movwf	SerByte1
	movlw	0x10
	movwf	SerByte2
	movlw	0x4c
	movwf	SerByte3
	movlw	0x08
	movwf	SerByte4
	movf	MIDI_Chn, W		; get control instr. with midi channel
	movwf	SerByte5
	movf	NRPN_Data_Lo, W		; send pot value
	movwf	SerByte7
	movlw	0xf7
	movwf	SerByte8

	goto	FlushMIDIQueue		; flush MIDI queue and return from subroutine

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
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F
	return

Init_MIDI_NRPN_mult_16
	clrc
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F
	return

Init_MIDI_NRPN_split_40
	movlw	0x40
	subwf	NRPN_Data_Lo, F
	btfss	STATUS, C
	decf	NRPN_Data_Hi, F
	return

Init_MIDI_NRPN_split_80
	clrc
	rlf	NRPN_Data_Lo, F
	rlf	NRPN_Data_Hi, F

	movlw	0x80
	subwf	NRPN_Data_Lo, F
	btfss	STATUS, C
	decf	NRPN_Data_Hi, F
	return


;; ==========================================================================
;;  EEPROM Handling
;; ==========================================================================

PrepareForEEPROM
	call	GetEEPROMAddr
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
	decf	STATUS_TMP, F
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
	btfsc	EECON1, F
	goto	Write_EEPROM_Loop
	bcf	STATUS, RP0

	incf	EEADR, F
	incf	FSR, F
	return

LoadPotStatus

	call	PrepareForEEPROM

	movlw	0x08
	movwf	STATUS_TMP
LoadLoop
	call	_LoadPotStatus
	decf	STATUS_TMP, F
	bnz	LoadLoop

	goto FlushPotData

_LoadPotStatus
	bsf	STATUS, RP0
	bsf	EECON1, RD
	bcf	STATUS, RP0

	movf	EEDATA, W
	andlw	0x7f
	movwf	INDF

	incf	EEADR, F
	incf	FSR, F
	return

;; ==========================================================================
;;  Output Pot Settings
;; ==========================================================================

FlushPotData
	movlw	0xff
	movwf	ActPot

	movf	Last_Pot0, W
	call	FlushPotData_hlp
	movf	Last_Pot1, W
	call	FlushPotData_hlp
	movf	Last_Pot2, W
	call	FlushPotData_hlp
	movf	Last_Pot3, W
	call	FlushPotData_hlp
	movf	Last_Pot4, W
	call	FlushPotData_hlp
	movf	Last_Pot5, W
	call	FlushPotData_hlp
	movf	Last_Pot6, W
	call	FlushPotData_hlp
	movf	Last_Pot7, W

FlushPotData_hlp
	movwf	NRPN_Data_Lo
	clrf	NRPN_Data_Hi
	incf	ActPot, F
	goto	SendPotValue

;; ==========================================================================
;;  Reset Pot Settings
;; ==========================================================================

ResetPotSettings
	movlw	0xff
	movwf	ActPot

	call	ResetPotSettings_hlp
	movwf	Last_Pot0
	call	ResetPotSettings_hlp
	movwf	Last_Pot1
	call	ResetPotSettings_hlp
	movwf	Last_Pot2
	call	ResetPotSettings_hlp
	movwf	Last_Pot3
	call	ResetPotSettings_hlp
	movwf	Last_Pot4
	call	ResetPotSettings_hlp
	movwf	Last_Pot5
	call	ResetPotSettings_hlp
	movwf	Last_Pot6
	call	ResetPotSettings_hlp
	movwf	Last_Pot7
	goto	FlushPotData

ResetPotSettings_hlp
	incf	ActPot, F
	movlw	0x00
	btfss	BoxMode, 3
	goto	GetResetValue
	return

;; ==========================================================================
;; Check if pot reached the reset value
;; ==========================================================================

CheckIndicator
	movlw	0x00
	btfss	BoxMode, 3
	call	GetResetValue
	subwf	NRPN_Data_Lo, W
	bz	IndicatorOn
IndicatorOff
	bsf	PORTB, PIN_LED
	return
IndicatorOn
	bcf	PORTB, PIN_LED
	return

;; ==========================================================================
;; Delay Loop
;; ==========================================================================

Delay
	movlw	0x01
	movwf	W_TMP
	clrf	STATUS_TMP
	clrf	ActPot
DelayLoop
	decfsz	ActPot, F
	goto	DelayLoop
	decfsz	STATUS_TMP, F
	goto	DelayLoop
	decfsz	W_TMP, F
	goto	DelayLoop
	return

	END

