;
; IRQ Vector (first part in Page0)
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;


;; ==========================================================================
;;  IRQ Vector
;; ==========================================================================

	ORG	0x0004

IRQ
	movwf	W_TMP
	swapf	STATUS, W
	clrf	STATUS		; switch to bank 0
	movwf	STATUS_TMP
	movf	PCLATH, W
	movwf	PCLATH_TMP
	movf	FSR, W
	movwf	FSR_TMP

	lgoto	IRQCheck
	
IRQEnd
	movf	FSR_TMP, W
	movwf	FSR
	movf	PCLATH_TMP, W
	movwf	PCLATH
	swapf	STATUS_TMP, W
	movwf	STATUS
	swapf	W_TMP, F
	swapf	W_TMP, W
	retfie


