;
; LCD Stuff
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; ==========================================================================
;;  LCD-Stuff
;; ==========================================================================

Init_LCD
	; (Initialization of Ports: done in Boot-Routine)

	call	LCD_LDelay ; Initial Delay

	; Display is in 8 Bit Mode - Set Data Length
	movlw	0x20
	movwf	PORT_LCD
	call	LCD_Enable

	call	LCD_LDelay

	; Now Display is in 4 Bit Mode - Set Data Length once more
	movlw	0x20
	movwf	PORT_LCD
	call	LCD_Enable

	movlw	0x80
	movwf	PORT_LCD
	call	LCD_Enable

	call	LCD_LDelay

	call	LCD_ClearDisplay
	call	LCD_DisplayOn
	call	LCD_EntryMode

	movlw	0x00
	call	LCD_CursorPos

	return

LCD_SendChar
	movwf	LCD_BUFFER
	andlw	0xf0
	xorlw	1 << RS
	movwf	PORT_LCD
	call	LCD_Enable
	movf	LCD_BUFFER, W
	swapf	LCD_BUFFER, W
	andlw	0xf0
	xorlw	1 << RS
	movwf	PORT_LCD
	call	LCD_Enable
	goto	LCD_Delay

LCD_SendCmd
	movwf	LCD_BUFFER
	andlw	0xf0
	movwf	PORT_LCD
	call	LCD_Enable
	movf	LCD_BUFFER, W
	swapf	LCD_BUFFER, W
	andlw	0xf0
	movwf	PORT_LCD
	call	LCD_Enable
	goto	LCD_Delay

LCD_ClearDisplay
	movlw	0x01
	call	LCD_SendCmd
	goto	LCD_LDelay

LCD_CursorPos
	iorlw	0x80
	goto	LCD_SendCmd

LCD_CharRAMAddr
	iorlw	0x40
	goto	LCD_SendCmd

LCD_ShiftLeft
	movlw	0x18
	goto	LCD_SendCmd

LCD_ShiftRight
	movlw	0x1c
	goto	LCD_SendCmd

LCD_DisplayOn
	movlw	0x0c
	goto	LCD_SendCmd

LCD_DisplayOnCursor
	movlw	0x0f		; (cursor on with blink)
	goto	LCD_SendCmd

LCD_EntryMode
	movlw	0x06
	goto	LCD_SendCmd

LCD_Delay
#if DEBUG_MODE == 1
	return
#endif
	movlw	0x02	; Normal Delay (appr. 300 us)
	goto	LCD__LDelay

LCD_LDelay
#if DEBUG_MODE == 1
	return
#endif
	; Long Delay (ca. 50 ms)
	movlw	0xff
LCD__LDelay
	movwf	LCD_DELAY_H
	clrf	LCD_DELAY_L
LCD_LDelay_Loop
	decfsz	LCD_DELAY_L, F
	goto	LCD_LDelay_Loop
	decfsz	LCD_DELAY_H, F
	goto	LCD_LDelay_Loop
END_DELAY
	return

LCD_Enable
	bsf	PORT_LCD, E
	nop
	nop
	nop
	nop
	bcf	PORT_LCD, E
	return

LCD_PrintValue
	movwf	LCD_VALUE
	swapf	LCD_VALUE, W
	call	LCD_PrintDigit
	movf	LCD_VALUE, W
	goto	LCD_PrintDigit

LCD_PrintDigit
	andlw	0x0f
	addlw	-0x0a
	skpnc
	addlw	0x07
	addlw	0x3a
	goto	LCD_SendChar

LCD_Debug
	movlw	0x08
	goto	LCD_PrintValue

LCD_PrintText
LCD_PrintTextLoop
	bsf	STATUS, RP0	; switch to Bank 3
	bsf	STATUS, RP1	
	bsf	EECON1, EEPGD	; point to program memory
	bsf	EECON1, RD	; EEPROM Read
	nop
	nop
	bcf	STATUS, RP0	; switch to Bank 2

	clrc			; load high byte
	IFSET	EEDATA, 7, setc
	rlf	EEDATH, W
	bcf	STATUS, RP1	; switch to Bank 0
	movwf	LCD_BUFFER	; only for testing
	tstf	LCD_BUFFER
	bz	LCD_PrintTextEnd
	call	LCD_SendChar	; print out first char
	bsf	STATUS, RP1	; switch to Bank 2
	
	movf	EEDATA, W	; load low byte
	andlw	0x7f
	bcf	STATUS, RP1	; switch to Bank 0
	bz	LCD_PrintTextEnd
	call	LCD_SendChar	; print out second char
	bsf	STATUS, RP1	; switch to Bank 2
	
	incf	EEADR, F	; increment memory pointer
	skpnz
	incf	EEADRH, F
	
	bcf	STATUS, RP1	; switch to Bank 0

	goto	LCD_PrintTextLoop

LCD_PrintTextEnd

	return

LCD_PrintText16
	movlw	0x00
	call	LCD_CursorPos
	
LCD_PrintText16Loop
	bsf	STATUS, RP0	; switch to Bank 3
	bsf	STATUS, RP1	
	bsf	EECON1, EEPGD	; point to program memory
	bsf	EECON1, RD	; EEPROM Read
	nop
	nop
	bcf	STATUS, RP0	; switch to Bank 2

	clrc			; load high byte
	IFSET	EEDATA, 7, setc
	rlf	EEDATH, W
	bcf	STATUS, RP1	; switch to Bank 0
	call	LCD_SendChar	; print out first char
	bsf	STATUS, RP1	; switch to Bank 2
	
	movf	EEDATA, W	; load low byte
	andlw	0x7f
	bcf	STATUS, RP1	; switch to Bank 0
	call	LCD_SendChar	; print out second char
	bsf	STATUS, RP1	; switch to Bank 2

	incf	EEADR, F	; increment memory pointer

	movf	EEADR, W	; End reached?	
	bcf	STATUS, RP1	; switch to Bank 0
	andlw	0x07
	bz	LCD_PrintText16End
	IFSET	DISPLAY_MODE, 0, goto LCD_PrintText16Loop

	xorlw	0x04
	bnz	LCD_PrintText16Loop
	movlw	0x40
	call	LCD_CursorPos
	goto	LCD_PrintText16Loop

LCD_PrintText16End
	return

LCD_InitBars
	movlw	0x00
	call	LCD_CharRAMAddr

	movlw	0x01
	movwf	BOX_COUNT
LCD_InitBarsOuterLoop
	movlw	0x07
	movwf	BOX_DELAY_L
LCD_InitBarsLoop
	movf	BOX_COUNT, W
	subwf	BOX_DELAY_L, W
	bc	LCD_InitBarsFF
LCD_InitBars00
	movlw	0xff
	call	LCD_SendChar
	goto	LCD_InitBarsCont
LCD_InitBarsFF
	movlw	0x00
	call	LCD_SendChar
LCD_InitBarsCont
	decf	BOX_DELAY_L, F
	IFCLR	BOX_DELAY_L, 7, goto LCD_InitBarsLoop

	incf	BOX_COUNT, F
	movf	BOX_COUNT, W
	xorlw	0x09
	bnz	LCD_InitBarsOuterLoop

	movlw	0x00
	call	LCD_CursorPos

	return
	







