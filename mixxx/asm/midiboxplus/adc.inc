;
; ADC Handling
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; ==========================================================================
;;  ADC Handling
;; ==========================================================================


;; --------------------------------------------------------------------------
;;  ADC Delay
;; --------------------------------------------------------------------------
ADC_Delay
	movlw	0x40
	movwf	BOX_DELAY_L
ADC_DelayLoop
	decfsz	BOX_DELAY_L, F
	goto	ADC_DelayLoop

	return

;; --------------------------------------------------------------------------
;;  ADC Conversion
;;  In:	 ADCON0 Configuration in W Register
;;  Out: Conversion Result in W
;; --------------------------------------------------------------------------
ADC_Conversion
	movwf	ADCON0
	call	ADC_Delay
	bsf	ADCON0, GO
ADC_ConversionLoop
	IFSET	ADCON0, NOT_DONE, goto ADC_ConversionLoop

	rrf	ADRESH, W
	andlw	0x7f

	return
	

