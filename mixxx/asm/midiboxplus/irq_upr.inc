;
; IRQ Vector (Upper Part in Last Page)
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;


IRQCheck
	; which IRQ received?
	IFSET	PIR1, RCIF, 	goto IRQ_UART_Receive	; UART Receive IRQ?
	IFSET	INTCON, T0IF, 	goto IRQ_Timer0		; Timer0 IRQ?
	lgoto	IRQEnd

;; ==========================================================================
;;  IRQ Handler for Timer0
;; ==========================================================================

IRQ_Timer0
	
	; reload T0
	clrf	TMR0


TimerIRQ_NoHandler
	bcf	INTCON, T0IF
	goto	IRQCheck


;; ==========================================================================
;;  IRQ Handler for UART Receiver
;; ==========================================================================

IRQ_UART_Receive
	movf	RCREG, W
	movwf	MIDI_RECEIVE

	IFSET	MIDI_RECEIVE, 7, goto	IRQ_UART_Command

	movf	MIDI_EXPBYTES, W
	bnz	IRQ_UART_RunningData

	movf	MIDI_RUNSTATUS, W
	andlw	0xf0
	xorlw	0xf0
	bz	IRQ_UART_NoRunningData

	movf	MIDI_RUNSTATUS, W
	call	MIDI_GetCMax
	movwf	MIDI_EXPBYTES
	movf	MIDI_RUNSTATUS, W
	call	MIDI_SendByte

IRQ_UART_RunningData
	decf	MIDI_EXPBYTES, F
IRQ_UART_NoRunningData
	movf	MIDI_RECEIVE, W
	call	MIDI_SendByte
	goto	IRQ_UART_End
			
IRQ_UART_Command
	movwf	MIDI_RUNSTATUS
	call	MIDI_SendByte
	movf	MIDI_RUNSTATUS, W
	call	MIDI_GetCMax
	movwf	MIDI_EXPBYTES
			
IRQ_UART_End	
	bcf	PIR1, RCIF
	goto	IRQCheck


;; ==========================================================================
;;  Initialization Routine for IRQ Handler
;; ==========================================================================
Init_IRQ
	SWITCHBANK_0_1
	
	movlw	(1 << RCIE)		; enable UART Receive IRQ
	movwf	PIE1
	clrf	PIE2			; clear all other IRQs

	SWITCHBANK_1_0

	bcf	RCSTA, CREN
	movf	RCREG, W
	bsf	RCSTA, CREN
		
	; enable maskable IRQs
	movlw	(1 << GIE) | (1 << PEIE)
	movwf	INTCON

	return


;; ==========================================================================
;;  Special IRQ enable routine
;;  make sure that a IRQ is really disabled
;;  (can loop some times if IRQ occurs concurrently w/ clearing GIE)
;; ==========================================================================
IRQ_Disable
	bcf	INTCON, GIE
	IFSET	INTCON, GIE, goto IRQ_Disable
	return

;; ==========================================================================
;;  Enables IRQs
;; ==========================================================================
IRQ_Enable
	bsf	INTCON, GIE
	return



