;
; Macro Definitions
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; ==========================================================================
;;  Macros
;; ==========================================================================

IFSET	MACRO	reg, bit, instr
	btfsc	reg, bit
	instr
	ENDM


IFCLR	MACRO	reg, bit, instr
	btfss	reg, bit
	instr
	ENDM

WEQ     MACRO   value, instr    ; do something if W == value
        xorlw   value
        skpnz
        instr
        ENDM

WGT     MACRO  value, instr     ; do something if W > value
        sublw   value
        skpc
        instr
        ENDM

WLTEQ   MACRO  value, instr     ; do something if W <= value
        sublw   value
        skpc
        instr
        ENDM
        
IRQ_DISABLE MACRO
        local   loop
loop:
        bcf     INTCON, GIE
        IFSET   INTCON, GIE, goto loop
        ENDM

IRQ_ENABLE MACRO
        bsf     INTCON, GIE
        ENDM
        
FLASH_ADDR MACRO addr
	bsf	STATUS, RP1	; switch to Bank 2
	movlw	(addr & 0xff)	; store Lo Byte
	movwf	EEADR
	movlw	(addr >> 8)	; store Hi Byte
	movwf	EEADRH
	bcf	STATUS, RP1	; switch to Bank 0
	ENDM

FLASH_H	MACRO	addr
	bsf	STATUS, RP1	; switch to Bank 2
	movlw	(addr >> 8)	; store Hi Byte
	movwf	EEADRH
	bcf	STATUS, RP1	; switch to Bank 0
	ENDM

FLASH_L	MACRO	addr
	bsf	STATUS, RP1	; switch to Bank 2
	movlw	(addr & 0xff)	; store Lo Byte
	movwf	EEADR
	bcf	STATUS, RP1	; switch to Bank 0
	ENDM

SWITCHBANK_0_1 MACRO
        bsf     STATUS, RP0
        ENDM
SWITCHBANK_0_2 MACRO
        bsf     STATUS, RP1
        ENDM
SWITCHBANK_0_3 MACRO
        bsf     STATUS, RP0
        bsf     STATUS, RP1
        ENDM

SWITCHBANK_1_0 MACRO
        bcf     STATUS, RP0
        ENDM
SWITCHBANK_1_2 MACRO
        bcf     STATUS, RP0
        bsf     STATUS, RP1
        ENDM
SWITCHBANK_1_3 MACRO
        bsf     STATUS, RP1
        ENDM

SWITCHBANK_2_0 MACRO
        bcf     STATUS, RP1
        ENDM
SWITCHBANK_2_1 MACRO
        bsf     STATUS, RP0
        bcf     STATUS, RP1
        ENDM
SWITCHBANK_2_3 MACRO
        bsf     STATUS, RP0
        ENDM

SWITCHBANK_3_0 MACRO
        bcf     STATUS, RP0
        bcf     STATUS, RP1
        ENDM
SWITCHBANK_3_1 MACRO
        bcf     STATUS, RP1
        ENDM
SWITCHBANK_3_2 MACRO
        bcf     STATUS, RP0
        ENDM


SWITCH_FSR_LOWER MACRO
        bcf     STATUS, IRP
        ENDM

SWITCH_FSR_UPPER MACRO
        bsf     STATUS, IRP
        ENDM

