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
	; IFCLR	TXSTA, TRMT, goto MIDIwaitloop;
	btfss TXSTA, TRMT
	goto MIDIwaitloop
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


