;
; Source-Code for MIDI-Box V1.500 (MBox Plus)
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;
; EMail: Thorsten.Klose@gmx.de
; Homepage: http://go.to/uCApps
;

#include <p16f874.inc>

	ERRORLEVEL      -302    ; SUPPRESS BANK SELECTION MESSAGES
	ERRORLEVEL      -307    ; SUPPRESS PCLATH SELECTION MESSAGES

#include "midibox.h"
#include "macros.h"
#include "reset.inc"
#include "irq.inc"

#include "data.inc"

	org	0x800

#include "irq_upr.inc"
#include "midi.inc"
#include "adc.inc"
#include "lcd.inc"
#include "boxmode.inc"
#include "layers.inc"
       ; condition tables included at bottom
	
;; ==========================================================================
;;  Main Program
;; ==========================================================================

Start
	movlw	0x0f		; store pointer to condition tables
	movwf	PCLATH
	movlw	0x7f		; Prescaler for Watchdog, Max Prescaling, Enable PullUps on PortB
	movwf	OPTION_REG	; (Enable PullDowns: also done in Init_Ports)
#if 1
	call	Init_Ports
#endif
	call	Init_USART
	call	Init_MIDI
#if 1
	call	Init_Layers
	IFSET	STATUS, NOT_TO, call Init_LCD		; Init LCD only if no WDT Reset
	IFCLR	STATUS, NOT_TO, call Malfunction_Message ; On WDT Reset print Malfunction Message
	call	LCD_InitBars

	call	Copyright_Message

	movlw	0x00		; initial layer: 01-1
	call	BOX_Button012345
#endif

	call	Init_IRQ



;; ==========================================================================

Loop
	clrwdt			; clear watchdog timer

	call	BOX_Mode

	movf	BOX_COUNT, W
	bz	Loop

Copyright_MessageLoop2
	call	LCD_ShiftRight
	movlw	0x10
	call	Startup_Delay
	decfsz	BOX_COUNT, F
	goto	Copyright_MessageLoop2

	call	LCD_ClearDisplay
	call	BOX_PrintOutLayer

	goto	Loop


;; --------------------------------------------------------------------------
;;  Init Routine for Ports
;; --------------------------------------------------------------------------
Init_Ports
	clrf	PORTA			; Reset PortA
	clrf	PORTB			; Reset PortB
	movlw	0x40			; Reset PortC (TX Pin = 1)
	movwf	PORTC
	clrf	PORTD			; Reset PortD
	clrf	PORTE			; Reset PortE
	
	SWITCHBANK_0_1

	bcf	OPTION_REG, NOT_RBPU	; Enable Pull-Ups of PortB
	movlw	TRISA_VALUE
	movwf	TRISA
	movlw	TRISB_VALUE
	movwf	TRISB
	movlw	TRISC_VALUE
	movwf	TRISC
	movlw	TRISD_VALUE
	movwf	TRISD
	movlw	TRISE_VALUE
	movwf	TRISE

#if EXT16
	movlw	0x05			; PortA and E: Pin 0, 1 analog inputs, Pin 3 = VRef+, all other digital pins
#else
	movlw	0x00			; PortA and E: All Pins are analog pins
#endif
	movwf	ADCON1

	SWITCHBANK_1_0

	return

;; --------------------------------------------------------------------------
;;  Init Routine for USART
;; --------------------------------------------------------------------------
Init_USART
	SWITCHBANK_0_1

	movlw	1 << TXEN
	movwf	TXSTA
  	movlw	0x09
	movwf	SPBRG

	SWITCHBANK_1_0

	; Configure Receive Status and Control Register
	movlw	(1 << SPEN) | (1 << CREN)
	movwf	RCSTA

	return

;; --------------------------------------------------------------------------
;;  Init Routine for Layers
;; --------------------------------------------------------------------------
Init_Layers
	;; Initialize Layers
	clrf	LAYER
	clrf	SL_COUNTER
	clrf	MIDI_CHANNEL

	movf	PORT_SL, W
	andlw	0x0f
	movwf	SUPER_LAYER
	movwf	LAST_SUPER_LAYER
	movwf	ACTIVE_SUPER_LAYER

	clrf	DISPLAY_MODE
	clrf	DOUBLECLICK
	clrf	LAST_DC_BUTTON

	call	BOX_CheckTestMode	; can change the DISPLAY_MODE if reset button pressed

        movlw   POT0_LVALUE		; init Last Pot Value with 0xff (invalid value)
        movwf   FSR
        movlw   0x10
        movwf   TMP1
Init_Layers_Loop
        clrf    INDF
        incf    FSR, F
        decfsz  TMP1, F
        goto    Init_Layers_Loop

	return

;; --------------------------------------------------------------------------
;;  Copyright Message
;; --------------------------------------------------------------------------
Copyright_Message
	call	LCD_DisplayOn
	call	LCD_ClearDisplay
	call	Copyright_MessageText

	movlw	0x40
	call	Startup_Delay

	movlw	0x10
	movwf	BOX_COUNT
Copyright_MessageLoop
	call	LCD_ShiftLeft
	movlw	0x10
	call	Startup_Delay
	decfsz	BOX_COUNT, F
	goto	Copyright_MessageLoop

	movlw	0xff
	call	Startup_Delay

	movlw	0x10
	movwf	BOX_COUNT
	return

;; --------------------------------------------------------------------------

Copyright_MessageText
	movlw	0x10
	call	LCD_CursorPos
	FLASH_ADDR TEXT_COPY_0
	call	LCD_PrintText
Copyright_MessageText1
	movlw	0x50
	call	LCD_CursorPos
	IFSET	DISPLAY_MODE, 1, goto Copyright_MessageText1_TestMode

	FLASH_ADDR TEXT_COPY_1
	goto	LCD_PrintText

Copyright_MessageText1_TestMode
	FLASH_ADDR TEXT_TEST_1
	goto	LCD_PrintText

;; --------------------------------------------------------------------------
;;  Malfunction Message
;; --------------------------------------------------------------------------
Malfunction_Message
	clrwdt			; clear watchdog timer
	call	LCD_DisplayOn
	call	LCD_ClearDisplay
	movlw	0x00
	call	LCD_CursorPos
	FLASH_ADDR MALFUNCTION_0
	call	LCD_PrintText
	movlw	0x40
	call	LCD_CursorPos
	FLASH_ADDR MALFUNCTION_1
	call	LCD_PrintText

	movlw	0xff
	goto	Startup_Delay

;; --------------------------------------------------------------------------
;;  Startup Delay (appr. 1 s)
;; --------------------------------------------------------------------------
Startup_Delay
	movwf	BOX_DELAY_H
	clrf	BOX_DELAY_L
Startup_DelayLoop
	decfsz	BOX_DELAY_L, F
	goto	Startup_DelayLoop

	clrwdt			; clear watchdog timer

	call	Copyright_MessageText1

	decfsz	BOX_DELAY_H, F
	goto	Startup_DelayLoop
	return

;; --------------------------------------------------------------------------
;;  ShiftDelay (appr. 100 ms)
;; --------------------------------------------------------------------------
Shift_Delay
	movlw	0xff
	movwf	BOX_DELAY_H
	clrf	BOX_DELAY_L
Shift_DelayLoop
	decfsz	BOX_DELAY_L, F
	decfsz	BOX_DELAY_H, F
	goto	Shift_DelayLoop
	return

;; ==========================================================================
;; ==========================================================================

#include "contab.inc"


	END




