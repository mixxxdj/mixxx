; 
; Mixer
;

#include <p16f874.inc>
 	ERRORLEVEL	-302 ; suppress bank selection warnings

Baudrate	EQU	0x09
Dataarea	EQU	0x20
TEMPQ		EQU	Dataarea
PORTB_init	EQU	B'00000001'
PORTD_init	EQU	B'11111111'
;
; Midi queue
;
SerBytes	EQU	Dataarea+4
SerByte0	EQU	Dataarea+5
SerByte1	EQU	Dataarea+6
SerByte2	EQU	Dataarea+7
;
; ADC
;
ADC_delaycount	EQU	Dataarea+8
ADC_0		EQU	Dataarea+9   ; These hold the last 
ADC_1		EQU	Dataarea+10  ; values read from the
ADC_2		EQU	Dataarea+11  ; analog ports.
ADC_3		EQU	Dataarea+12
ADC_4		EQU	Dataarea+13
ADC_5		EQU	Dataarea+14
ADC_6		EQU	Dataarea+15
ADC_7		EQU	Dataarea+16
ADC_curr_no	EQU	Dataarea+17
ADC_curr_value	EQU	Dataarea+18
ADC_new_value	EQU	Dataarea+19
ADC_potvalue	EQU	Dataarea+20
;
; Input ports
;
PORTB_curr_value EQU	Dataarea+21  ; this is the current value
PORTB_new_value	EQU	Dataarea+22  ; temporary storage for new read.
PORTD_curr_value EQU	Dataarea+23  ; this is the current value
PORTD_new_value	EQU	Dataarea+24  ; temporary storage for new read.

;; ==========================================================================
;;  Macros
;; ==========================================================================
BANK1 MACRO
	bsf	STATUS, RP0
	ENDM

BANK0 MACRO
	bcf	STATUS, RP0
	ENDM

IFCLR	MACRO	reg, bit, instr
	btfss	reg, bit
	instr
	ENDM

IFSET	MACRO	reg, bit, instr
	btfsc	reg, bit
	instr
	ENDM

	ORG	0x0000
	lgoto 	Start
	ORG	0x800

#include "midi.asm"
#include "adc.asm"

;; ==========================================================================
;;  Control the init LED
;; ==========================================================================

InitLED	EQU	5		; The portc bit for initLED
	; Set port C to output
InitInit
	BANK1
	bcf	TRISC, InitLED
	return

InitOn
	BANK0
	bcf	PORTC, InitLED
	return

InitOff
	BANK0
	bsf	PORTC, InitLED
	return

;; ==========================================================================
;;  Check any new inputs on port b
;; ==========================================================================
Portb_check
	movf	PORTB, W
	andlw	PORTB_init 	; take only the used pins
	movwf	PORTB_new_value ; save it temporarily
	; Check if the value has changed from last time:
	xorwf	PORTB_curr_value
	bnz	Portb_send
	movf	PORTB_new_value, W
	movwf	PORTB_curr_value  ; restore the current value.
	return

Portb_send
	; Save the new value:
	movf	PORTB_new_value, W
	movwf	PORTB_curr_value

	; Send it as a control change on 1
	andlw	B'11111111'
	movwf	SerByte2
	movlw	1	; midi channel
	addlw	0xb0
	movwf 	SerByte0
	movlw	1	; controller no.
	movwf	SerByte1
	call 	MIDIflushQueue
	return

;; ==========================================================================
;;  Check any new inputs on port d
;; ==========================================================================
Portd_check
	movf	PORTD, W
	andlw	PORTD_init 	; take only the used pins
	movwf	PORTD_new_value ; save it temporarily
	; Check if the value has changed from last time:
	xorwf	PORTD_curr_value
	bnz	Portd_send
	movf	PORTD_new_value, W
	movwf	PORTD_curr_value  ; restore the current value.
	return

Portd_send
	; Save the new value:
	movf	PORTD_new_value, W
	movwf	PORTD_curr_value

	; Send it as a controller 3 change 
	;andlw	B'01111111'	
	movwf	SerByte2
	movlw	1	; midi channel
	addlw	0xb0
	movwf 	SerByte0
	movlw	3	; controller no.
	movwf	SerByte1
	call 	MIDIflushQueue
	return

; ======================================================
; Main program
; ======================================================
Start
	; Switch on the init LED
	call	InitInit
	call	InitOn

	call	ADC_init
	call 	MIDIinit

	; Initialize port B
	clrf	PORTB	; Set all pins to low.
	BANK1
	movlw	PORTB_init
	movwf	TRISB
	;bcf	OPTION_REG, NOT_RBPU	; enable weak pull-ups
	BANK0

	; Initialize port D
	clrf	PORTD	; Set all pins to low.
	BANK1
	movlw	PORTD_init
	movwf	TRISD
	BANK0
	
Loop
	; Check potmeters:
	call	ADC_check_0
	call	ADC_check_1
	call	ADC_check_2
	call	ADC_check_3
	call	ADC_check_4
	call	ADC_check_5
	call	ADC_check_6
	call	ADC_check_7

	; Check contacts:
	call 	Portb_check		
	call	Portd_check
	
	goto Loop

	end
