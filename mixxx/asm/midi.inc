;
; MIDI Handling
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; --------------------------------------------------------------------------
;;  Init Routine for MIDI
;; --------------------------------------------------------------------------
Init_MIDI
	; initialize Ring-Buffer
	movlw	RINGBUFFER
	movwf	RINGBUFFER_A
	movwf	RINGBUFFER_B

	clrf	MIDI_RUNSTATUS
	clrf	MIDI_EXPBYTES
	bsf	MIDI_IDLE, 0

	return

;; --------------------------------------------------------------------------
;;  MIDI Send Byte: puts byte to send onto ring buffer
;;  In:	Byte to Send in W
;; --------------------------------------------------------------------------
MIDI_SendByte
	movwf	MIDI_TMP		; save byte to send
	incf	RINGBUFFER_A, F		; 4 Bit Ringbuffer Address 
	bcf	RINGBUFFER_A, 4
	movf	RINGBUFFER_A, W
	movwf	FSR
	movf	MIDI_TMP, W
	movwf	INDF
	return

;; --------------------------------------------------------------------------
;;  _MIDI Send Byte: sends byte over MIDI interface
;;  In:	Byte to Send in W
;; --------------------------------------------------------------------------
_MIDI_SendByte
	movwf	TXREG
	SWITCHBANK_0_1
_MIDI_SendBytePoll
	IFCLR	TXSTA, TRMT, goto _MIDI_SendBytePoll
	SWITCHBANK_1_0
	return

;; --------------------------------------------------------------------------
;;  MIDI_Flush:	Flushs Ring Buffer if there is anything to send
;; --------------------------------------------------------------------------
MIDI_Flush
	clrwdt			; clear watchdog timer

;	call	IRQ_Disable		; disable IRQs
	movf	RINGBUFFER_A, W
	subwf	RINGBUFFER_B, W
	bz	MIDI_Flush_CheckIdle
;	call	IRQ_Enable		; enable IRQs

	movf	FSR, W			; save FSR 
	movwf	MIDI_FSRTMP

	incf	RINGBUFFER_B, F
	bcf	RINGBUFFER_B, 4
	movf	RINGBUFFER_B, W
	movwf	FSR

	movf	INDF, W
	call	_MIDI_SendByte

	movf	MIDI_FSRTMP, W		; restore FSR
	movwf	FSR

	goto	MIDI_Flush

MIDI_Flush_CheckIdle
	movlw	0x01
	tstf	MIDI_EXPBYTES
	skpz
	movlw	0x00
	movwf	MIDI_IDLE
;	call	IRQ_Enable		; enable IRQs
	return


;; --------------------------------------------------------------------------
;;  MIDI_Check:	Check if there is something to send, flushs buffer if true
;; --------------------------------------------------------------------------
MIDI_Check
	;; wait until running status complete
MIDI_CheckLoop
	call	MIDI_Flush
	tstf	MIDI_IDLE
	bz	MIDI_CheckLoop	; idle != 0: running status not complete
	return
