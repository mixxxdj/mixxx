; 
; Mixer
;

#include <p16f874.inc>
 

Baudrate	EQU	0x09
Dataarea	EQU	0x20
TEMPQ		EQU	Dataarea

;
; Midi queue
;
SerBytes	EQU	Dataarea+4
SerByte0	EQU	Dataarea+5
SerByte1	EQU	Dataarea+6
SerByte2	EQU	Dataarea+7

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

;	ORG	0
	goto 	Start


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
;;  Send out MIDI Messages, if MIDI-Queue initialized
;; ==========================================================================

MIDIinit:
	BANK1
	movlw	1 << TXEN
	movwf	TXSTA
	movlw	Baudrate		; Set the speed
	movwf	SPBRG
	BANK0
	movlw	(1 << SPEN) | (1 << CREN); activate port
	movwf	RCSTA
	return

;-----------------------
;	Send byte in W
;-----------------------
MIDIsendbyte
	movwf	TXREG
	BANK1
MIDIwaitloop
	IFCLR	TXSTA, TRMT, goto MIDIwaitloop
	BANK0
	return

MIDIflushQueue:
	movf	SerByte0, W
	call 	MIDIsendbyte
 	movf	SerByte1, W		; Move the queue up
	call	MIDIsendbyte
	movf	SerByte2, W
	call	MIDIsendbyte
	return

;
; Main program
;
Start
	; Switch on the init LED
	call	InitInit
	call	InitOff

	BANK1
	bcf	TRISC, 4
	bcf	TRISC, 3
	BANK0
	bcf	PORTC, 4
	bsf	PORTC, 3

;	call 	MIDIinit

	; Set port B7 and B6 to input
;	BANK1
;	bsf	TRISB, 7
;	bsf 	TRISB, 6
;	BANK0
tt	
	call InitOn
;	btfss	PORTB, 7
	goto	Start

	; Send a MIDI controller change
	movlw	1	; midi channel
	addlw	0xb0
	movwf 	SerByte0
	movlw	7	; controller no.
	movwf	SerByte1
	movlw	64	; value
	movwf	SerByte2

	call	InitOff
	call 	MIDIflushQueue

	goto 	Start

	end
