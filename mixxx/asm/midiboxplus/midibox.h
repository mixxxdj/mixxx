;
; General Definitions
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; ==========================================================================
;;  Assembler & PICstart directives
;; ==========================================================================

	LIST P=PIC16F874, R=DEC

#if 0
	__CONFIG _HS_OSC & _WDT_ON & _PWRTE_ON & _CP_HALF & _BODEN_ON & _LVP_OFF
#else
	__CONFIG _HS_OSC & _WDT_ON & _PWRTE_ON & _CP_OFF & _BODEN_ON & _LVP_OFF
#endif

;; ==========================================================================
;;  Some Flags
;; ==========================================================================

#define DEBUG_MODE 0
#define EXT16      1
#define P15_TO_P23 0

;; ==========================================================================
;;  Ports
;; ==========================================================================

; Pins of Pots
PORT_POTS0	EQU	PORTA
PORT_POTS1	EQU	PORTE

; Pins of Buttons
PORT_BUTTONS	EQU	PORTC
#if P15_TO_P23 == 0
PIN_BUTTON_04	EQU	0	; Pin C.0
#else
PIN_BUTTON_04	EQU	4	; Pin C.4
#endif
PIN_BUTTON_15	EQU	1	; Pin C.1
PIN_BUTTON_26	EQU	2	; Pin C.2
PIN_BUTTON_37	EQU	3	; Pin C.3
	
PORT_B_CTRL	EQU	PORTD	; volatge supply control
TRIS_B_CTRL	EQU	TRISD
PIN_BUT_0123	EQU	0	; Pin D.0
PIN_BUT_4567	EQU	1	; Pin D.1

	
; Pins of Super-Layer-Switches
PORT_SL		EQU	PORTB
PIN_SL_0	EQU	0	; Pin B.0
PIN_SL_1	EQU	1	; Pin B.1
PIN_SL_2	EQU	2	; Pin B.2
PIN_SL_3	EQU	3	; Pin B.3

	
; Pins of MIDI Channel Switches
PORT_CHN	EQU	PORTB
PIN_CHN_0	EQU	4	; Pin B.4
PIN_CHN_1	EQU	5	; Pin B.5
PIN_CHN_2	EQU	6	; Pin B.6
PIN_CHN_3	EQU	7	; Pin B.7

;; MUX Pins (only for 16 Pot Extension)
PORT_MUX_0      EQU     PORTA   ; Pin A.5
PIN_MUX_0       EQU     5
PORT_MUX_1      EQU     PORTE   ; Pin E.0
PIN_MUX_1       EQU     0
PORT_MUX_2      EQU     PORTE   ; Pin E.1
PIN_MUX_2       EQU     1

; Pin of Reset Value Indicator LED
PORT_LED	EQU	PORTC
PIN_LED		EQU	5


; Pins of LC-Display
PORT_LCD	EQU	PORTD
DATALINE	EQU	4		; Pin D.7-4
E		EQU	3		; Pin D.3
RS		EQU	2		; Pin D.2


#if EXT16 
TRISA_VALUE	EQU	b'11001111'	; Setup Value for Tristate Drivers of PortA
TRISE_VALUE	EQU	b'00000100'	; Setup Value for Tristate Drivers of PortE
#else
TRISA_VALUE	EQU	b'11101111'	; Setup Value for Tristate Drivers of PortA
TRISE_VALUE	EQU	b'00000111'	; Setup Value for Tristate Drivers of PortE
#endif
TRISB_VALUE	EQU	b'11111111'	; Setup Value for Tristate Drivers of PortB
TRISC_VALUE	EQU	b'10011111'	; Setup Value for Tristate Drivers of PortC
TRISD_VALUE	EQU	b'00000011'	; Setup Value for Tristate Drivers of PortD


;; ==========================================================================
;;  Status Bits and Bytes
;; ==========================================================================

RX_ACTIVE	EQU	0

DATA_LOAD	EQU	0x00
DATA_SAVE	EQU	0x01
DATA_RESET	EQU	0x02

;; ==========================================================================
;;  Register File
;; ==========================================================================

POT_VALUE	EQU	0x20
ADC_VALUE	EQU	0x21

BUTTON_STATUS	EQU	0x22
BUTTON_ACTSTAT	EQU	0x23
BUTTON_LASTSTAT	EQU	0x24

MIDI_CHANNEL	EQU	0x25

NRPN_DATALO	EQU	0x26
NRPN_DATAHI	EQU	0x27

LAYER		EQU	0x28
SUPER_LAYER	EQU	0x29
LAST_SUPER_LAYER EQU	0x2a
ACTIVE_SUPER_LAYER EQU	0x2b
DISPLAY_MODE	EQU	0x2c

SL_COUNTER	EQU	0x2d
DOUBLECLICK	EQU	0x2e
LAST_DC_BUTTON	EQU	0x2f

EEPROM_TASK	EQU	0x30

LCD_DELAY_L	EQU	0x31
LCD_DELAY_H	EQU	0x32
LCD_BUFFER	EQU	0x33
LCD_TEXT_CNT	EQU	0x34
LCD_VALUE	EQU	0x35

BOX_DELAY_L	EQU	0x36
BOX_DELAY_H	EQU	0x37
BOX_COUNT	EQU	0x38

RINGBUFFER_A	EQU	0x39
RINGBUFFER_B	EQU	0x3a

MIDI_CVALUE	EQU	0x3b
MIDI_IDLE	EQU	0x3c
MIDI_RUNSTATUS	EQU	0x3d
MIDI_EXPBYTES	EQU	0x3e
MIDI_FSRTMP	EQU	0x3f
MIDI_RECEIVE	EQU	0x40

; free: 0x41-0x5f

RINGBUFFER	EQU	0x60	;  0x60-0x6f

TMP1		EQU	0x70	; for main programs
MIDI_TMP	EQU	0x71
BAR_TMP		EQU	0x72

; free: 0x73-0x7b

FSR_TMP		EQU	0x7c
PCLATH_TMP	EQU	0x7d
STATUS_TMP	EQU	0x7e
W_TMP		EQU	0x07f
W_TMP2		EQU	0x0ff
W_TMP3		EQU	0x17f
W_TMP4		EQU	0x1ff

;; ==========================================================================

; DONT CHANGE THE ADDRESSES!
POT0_VALUE	EQU	0xa0
POT1_VALUE	EQU	0xa1
POT2_VALUE	EQU	0xa2
POT3_VALUE	EQU	0xa3
POT4_VALUE	EQU	0xa4
POT5_VALUE	EQU	0xa5
POT6_VALUE	EQU	0xa6
POT7_VALUE	EQU	0xa7
POT8_VALUE	EQU	0xa8
POT9_VALUE	EQU	0xa9
POTA_VALUE	EQU	0xaa
POTB_VALUE	EQU	0xab
POTC_VALUE	EQU	0xac
POTD_VALUE	EQU	0xad
POTE_VALUE	EQU	0xae
POTF_VALUE	EQU	0xaf

POT0_RVALUE	EQU	0xb0
POT1_RVALUE	EQU	0xb1
POT2_RVALUE	EQU	0xb2
POT3_RVALUE	EQU	0xb3
POT4_RVALUE	EQU	0xb4
POT5_RVALUE	EQU	0xb5
POT6_RVALUE	EQU	0xb6
POT7_RVALUE	EQU	0xb7
POT8_RVALUE	EQU	0xb8
POT9_RVALUE	EQU	0xb9
POTA_RVALUE	EQU	0xba
POTB_RVALUE	EQU	0xbb
POTC_RVALUE	EQU	0xbc
POTD_RVALUE	EQU	0xbd
POTE_RVALUE	EQU	0xbe
POTF_RVALUE	EQU	0xbf

; free: 0xc0 - 0xe0

POT0_LVALUE	EQU	0xe0
POT1_LVALUE	EQU	0xe1
POT2_LVALUE	EQU	0xe2
POT3_LVALUE	EQU	0xe3
POT4_LVALUE	EQU	0xe4
POT5_LVALUE	EQU	0xe5
POT6_LVALUE	EQU	0xe6
POT7_LVALUE	EQU	0xe7
POT8_LVALUE	EQU	0xe8
POT9_LVALUE	EQU	0xe9
POTA_LVALUE	EQU	0xea
POTB_LVALUE	EQU	0xeb
POTC_LVALUE	EQU	0xec
POTD_LVALUE	EQU	0xed
POTE_LVALUE	EQU	0xee
POTF_LVALUE	EQU	0xef

