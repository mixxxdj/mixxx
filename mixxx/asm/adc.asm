;
; ADC Handling
;

;; ==========================================================================
;;  ADC Handling
;; ==========================================================================

ADC_init
	clrf	PORTA	; reset port A
	
	BANK1
	movlw	0xff
	movwf	TRISA	; set all A lines to input.
	
	clrf	ADCON1	; Set all A lines to analog input
	BANK0
	return

;; --------------------------------------------------------------------------
;; Read one of the ports. If the value is
;; changed, send the value via midi.
;; --------------------------------------------------------------------------
ADC_check_0
 	movf	ADC_0, W	; shift the last value over to curr_value
	movwf	ADC_curr_value
	movlw	0x00		; the port no.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_0
	return

ADC_check_1
 	movf	ADC_1,W
	movwf	ADC_curr_value
	movlw	0x08		; port 1.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_1
	return

ADC_check_2
 	movf	ADC_2,W
	movwf	ADC_curr_value
	movlw	0x10		; port 2.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_2
	return

ADC_check_3
 	movf	ADC_3,W
	movwf	ADC_curr_value
	movlw	B'00011000'	; port 3.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_3
	return

ADC_check_4
 	movf	ADC_4,W
	movwf	ADC_curr_value
	movlw	B'00100000'	; port 4.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_4
	return

ADC_check_5
 	movf	ADC_5,W
	movwf	ADC_curr_value
	movlw	B'00101000'	; port 5.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_5
	return

ADC_check_6
 	movf	ADC_6,W
	movwf	ADC_curr_value
	movlw	0x30		; port 6.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_6
	return

ADC_check_7
 	movf	ADC_7,W
	movwf	ADC_curr_value
	movlw	0x38		; port 7.
	call 	ADC_checkchange
	; store the new (maybe unchanged) value:
	movf	ADC_new_value, W
	movwf	ADC_7
	return

;	See if there is a change in the value:
ADC_checkchange
	movwf	ADC_curr_no
	call	ADC_Conversion
	movwf	ADC_potvalue	; current pot-value
	; is the value changed?
	xorwf	ADC_curr_value,W
	bz	ADC_forgetit
	;
	; Accept the value only if three subsequent reads give the same
	;
	call	ADC_Conversion
	xorwf	ADC_potvalue, W
	bnz	ADC_forgetit

	call	ADC_Conversion
	xorwf	ADC_potvalue, W
	bnz	ADC_forgetit

	call	ADC_Conversion
	xorwf	ADC_potvalue, W
	bnz	ADC_forgetit
	;
	; Value accepted; save it
	;		
	movf	ADC_potvalue, W
	movwf	ADC_new_value	
 
	; Send it via midi.
ADC_send_value
	; Send a MIDI controller change
	movlw	1	; midi channel
	addlw	0xb0
	movwf 	SerByte0
	movf	ADC_curr_no,W	; controller no.
	movwf	SerByte1
	movf	ADC_new_value,W
	movwf	SerByte2
	goto 	MIDIflushQueue

ADC_forgetit
	movf	ADC_curr_value, W	; restore the old value
	movwf	ADC_new_value
	return
 
;; --------------------------------------------------------------------------
;;  ADC Conversion
;;  In:  Channel no. (in bits. 6,5,4) in ADC_curr_no.
;;  Out: Conversion Result in W
;; --------------------------------------------------------------------------
ADC_Conversion
	movfw	ADC_curr_no
	addlw	0x81	; add configurations bits to channel	
	movwf	ADCON0
	; Make a delay:
	movlw	0x40
	movwf	ADC_delaycount
ADC_DelayLoop
	decfsz	ADC_delaycount, F
	goto	ADC_DelayLoop
	; Wait until the conversion is finished:
	bsf	ADCON0, GO
ADC_ConversionLoop		
	;IFSET	ADCON0, NOT_DONE, goto ADC_ConversionLoop
	btfsc	ADCON0, NOT_DONE
	goto ADC_ConversionLoop
	
	rrf	ADRESH, W
	andlw	0x7f

	return
	
