;
; Boxmode Routines
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; ==========================================================================
;;  This is the main loop of MIDI box
;; ==========================================================================
BOX_Mode
	call	BOX_CheckButtons

	call	BOX_Delay

	IFCLR	DISPLAY_MODE, 1, goto BOX_ModeNoTest
	call	BOX_PrintTestMode
	goto	BOX_ModeTestCont

BOX_ModeNoTest
	call	BOX_Mode_FlashLine

BOX_ModeTestCont

	movlw	0x40
	call	LCD_CursorPos

	movlw	POT0_VALUE
	movwf	FSR

	call	BOX_CheckPot		; ADC conversion and check for Pot0
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot1
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot2
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot3
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot4
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot5
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot6
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot7

#if EXT16
	call	BOX_CheckPot		; ADC conversion and check for Pot8
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for Pot9
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for PotA
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for PotB
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for PotC
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for PotD
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for PotE
	call	MIDI_Check
	call	BOX_CheckPot		; ADC conversion and check for PotF
#endif

	return

;; --------------------------------------------------------------------------
;;  Box Check Pot: Checks a Pot and sends out the value on changes
;;  In:	 Config Value for ADCON0 in W, Address of Pot Buffer in FSR
;;  Out: Increments the FSR Pointer
;; --------------------------------------------------------------------------
BOX_CheckPot
	call	BOX_GetADCValue
	movwf	ADC_VALUE

	;; Sample Value 4 times. Continue only if all values are equal
	call	ADC_Conversion
	movwf	POT_VALUE
	
	movf	ADC_VALUE, W
	call	ADC_Conversion
	xorwf	POT_VALUE, W
	bnz	BOX_CheckPotEnd
	
	movf	ADC_VALUE, W
	call	ADC_Conversion
	xorwf	POT_VALUE, W
	bnz	BOX_CheckPotEnd

	movf	ADC_VALUE, W
	call	ADC_Conversion
	xorwf	POT_VALUE, W
	bnz	BOX_CheckPotEnd

	;; Compare with last send value
	movf	POT_VALUE, W
	bsf	FSR, 6		; switch to "real last value"
	xorwf	INDF, W
	bcf	FSR, 6		; switch to "last value"
	bz	BOX_CheckPotEnd

	;; Send Value if there is any change
	movf	POT_VALUE, W
	movwf	INDF
	bsf	FSR, 6		; switch to "real last value"
	movwf	INDF
	bcf	FSR, 6		; switch to "last value"
	call	BOX_SendPotValue

	;; Check Reset Value
	bsf	FSR, 4		; switch pointer to reset value
	movf	INDF, W
	bcf	FSR, 4		; switch back to pot value
	xorwf	POT_VALUE, W
	bz	BOX_CheckPotResetValue
	bsf	PORT_LED, PIN_LED
	goto	BOX_CheckPotEnd
BOX_CheckPotResetValue
	bcf	PORT_LED, PIN_LED
	
BOX_CheckPotEnd
	;; Print Out Value
	movf	DISPLAY_MODE, W
	bnz	BOX_CheckPotDisp1

	call	BOX_PrintBar
	goto	BOX_CheckPotFinalEnd

BOX_CheckPotDisp1
	movf	INDF, W
	call	LCD_PrintValue

BOX_CheckPotFinalEnd
	incf	FSR, F
	return


;; --------------------------------------------------------------------------
;;  Box Delay: Delay of 1 ms minimum, calls MIDI_Check to make sure that
;;             incoming MIDI Values will be forwarded quickly
;; --------------------------------------------------------------------------
BOX_Delay
	movlw	0x1
	movwf	BOX_DELAY_H
	clrf	BOX_DELAY_L
BOX_DelayLoop
	
	call	MIDI_Check

	decfsz	BOX_DELAY_L, F
	goto	BOX_DelayLoop
	decfsz	BOX_DELAY_H, F
	goto	BOX_DelayLoop
	return


;; --------------------------------------------------------------------------
;;  Box Mode Flash Line: flashes the first line during SL selection
;; --------------------------------------------------------------------------
BOX_Mode_FlashLine
	movf	SUPER_LAYER, W
	xorwf	LAST_SUPER_LAYER, W
	bz	BOX_Mode_FlashLineEnd
	
	movlw	0xff		; make sure that line flashes also after reselection
	movwf	LAST_SUPER_LAYER

	IFSET	SL_COUNTER, 4, goto BOX_Mode_FlashLineOn

	movlw	0x00
	call	LCD_CursorPos
	movlw	0x20
	call	LCD_SendChar
	movlw	0x20
	call	LCD_SendChar
	movlw	0x20
	call	LCD_SendChar
	movlw	0x20
	call	LCD_SendChar
	goto	BOX_Mode_FlashLineCont

BOX_Mode_FlashLineOn
	call	BOX_PrintOutLayer

BOX_Mode_FlashLineCont
	incf	SL_COUNTER, F
	IFSET	SL_COUNTER, 5, clrf SL_COUNTER

BOX_Mode_FlashLineEnd
	return

;; --------------------------------------------------------------------------
;;  Box Check Buttons: Checks all buttons and jumps to the associated routine
;;		       if button is pressed
;; --------------------------------------------------------------------------
BOX_CheckButtons

;; --[ BEGIN MACRO ] --------------------------------------------------------
CHECKBUTTON MACRO set_button_mode
	LOCAL	NotifyNewStatus
	LOCAL	NoStatusChange

	rrf	BUTTON_LASTSTAT, 1	; typed key?
	bc	NotifyNewStatus
	rrf	BUTTON_ACTSTAT, 1	; we dont need it...
	goto	NoStatusChange

NotifyNewStatus
	rrf	BUTTON_ACTSTAT, 1
	btfss	STATUS, C		; no MIDI msg if key released
	call	set_button_mode
NoStatusChange
	ENDM
;; --[ END MACRO ] ----------------------------------------------------------

	;; actualize MIDI Channel
	swapf	PORT_CHN, W
	andlw	0x0f
	movwf	MIDI_CHANNEL

	movf	PORT_SL, W
#if EXT16
	andlw	0x07
#else
	andlw	0x0f
#endif
	movwf	SUPER_LAYER

	;; actualize Functional Buttons
	clrf	PORT_B_CTRL

	; select button 0-3
	SWITCHBANK_0_1
	movlw	1 << PIN_BUT_4567	; 4567 to tristate -> 0123 free
	movwf	TRIS_B_CTRL
	SWITCHBANK_1_0
	nop
	nop
	nop
	nop

	movf	PORT_BUTTONS, W			; get button data
	andlw	0x0f
#if P15_TO_P23 == 0
	andlw	0x0f
#else
	andlw	0x0e
	IFSET	PORT_BUTTONS, PIN_BUTTON_04, iorlw 0x01	
#endif
	movwf	BUTTON_STATUS			; save button 0123 in BUTTON_STATUS


	; select button 4-7
	SWITCHBANK_0_1
	movlw	1 << PIN_BUT_0123	; 0123 to tristate -> 4567 free
	movwf	TRIS_B_CTRL
	SWITCHBANK_1_0
	nop
	nop
	nop
	nop

	swapf	PORT_BUTTONS, W			; get button data
#if P15_TO_P23 == 0
	andlw	0xe0
#else
	andlw	0xe0
	IFSET	PORT_BUTTONS, PIN_BUTTON_04, iorlw 0x10
#endif
	iorwf	BUTTON_STATUS, F		; save button 4567 in BUTTON_STATUS



;; --[ analyze layer buttons ]-----------------------------------------------

	movf	BUTTON_STATUS, W
	movwf	BUTTON_ACTSTAT		; save actual KeyStatus for rrf's
	xorwf	BUTTON_LASTSTAT, F	; get changes from KeyStatus

	CHECKBUTTON BOX_Button0
	CHECKBUTTON BOX_Button1
	CHECKBUTTON BOX_Button2
	CHECKBUTTON BOX_Button3
	CHECKBUTTON BOX_Button4
	CHECKBUTTON BOX_Button5
	CHECKBUTTON BOX_Button6
	CHECKBUTTON BOX_Button7

	movf	BUTTON_STATUS, W	; store Button Status
	movwf	BUTTON_LASTSTAT

	tstf	DOUBLECLICK
	bz	BOX_CheckButtonsNC
	decf	DOUBLECLICK, F

BOX_CheckButtonsNC
	return

;; --------------------------------------------------------------------------
;;  Box Button Routines
;; --------------------------------------------------------------------------
BOX_Button0
	movlw	0x00
BOX_Button012345
	movwf	LAYER
	movf	SUPER_LAYER, W
	movwf	LAST_SUPER_LAYER
	movwf	ACTIVE_SUPER_LAYER

	movf	DOUBLECLICK, W
	bz	BOX_Button012345NoDoubleClick
	WGT	0x28, goto BOX_Button012345NoDoubleClick ; entprellen

	movf	LAYER, W
	xorwf	LAST_DC_BUTTON, W
	bnz	BOX_Button012345NoDoubleClick

	;; (TestMode: dont switch the DISPLAY_MODE)
	IFSET	DISPLAY_MODE, 1, goto BOX_Button012345NoDoubleClick

	incf	DISPLAY_MODE, F
	bcf	DISPLAY_MODE, 1
	clrf	DOUBLECLICK
	goto	BOX_Button012345Cont

BOX_Button012345NoDoubleClick
	movlw	0x30
	movwf	DOUBLECLICK
	movf	LAYER, W
	movwf	LAST_DC_BUTTON

BOX_Button012345Cont
	call	BOX_PrintOutLayer
	call	EEPROM_GetData
	call	BOX_CheckNRPN
	goto	BOX_FlushPotData
	
BOX_Button1
	movlw	0x01
	goto	BOX_Button012345
	
BOX_Button2
	movlw	0x02
	goto	BOX_Button012345

BOX_Button3
	movlw	0x03
	goto	BOX_Button012345

BOX_Button4
	movlw	0x04
	goto	BOX_Button012345

BOX_Button5
	movlw	0x05
	goto	BOX_Button012345

BOX_Button6
	bcf	PORT_LED, PIN_LED
	call	EEPROM_SaveData
	bsf	PORT_LED, PIN_LED
	return

BOX_Button7
	bcf	PORT_LED, PIN_LED
	call	EEPROM_ResetData
	call	BOX_FlushPotData
	bsf	PORT_LED, PIN_LED
	return

;; --------------------------------------------------------------------------
;;  Box Check NRPN (only on deticated layers)
;;  In:	 Layer in LAYER and SUPER_LAYER
;; --------------------------------------------------------------------------
BOX_CheckNRPN
	;; select Superlayer 9-13 for NRPN
	IFCLR	SUPER_LAYER, 3, return
	IFSET	SUPER_LAYER, 2, return

	; select SuperLayer 9 and 10 for GM NRPN (NOTE:	Superlayer is already preselected!)
	IFSET	SUPER_LAYER, 1, goto BOX_CheckNRPN_SendNRPN7F
	; select Layer 5 and 6 for GM NRPN
	IFCLR	LAYER, 2, goto BOX_CheckNRPN_SendNRPN7F
	movlw	0x01
	goto	BOX_CheckNRPN_SendNRPN
BOX_CheckNRPN_SendNRPN7F
	movlw	0x7f
BOX_CheckNRPN_SendNRPN
	goto	Send_NRPN_MSB

;; --------------------------------------------------------------------------
;;  Box Print Out Layer
;;  In:	 Layer in LAYER and SUPER_LAYER
;; --------------------------------------------------------------------------
BOX_PrintOutLayer
	IFSET	DISPLAY_MODE, 1, return ; TestMode

	movf	SUPER_LAYER, W
	call	BOX_GetSuperLayerIndex
	addwf	LAYER, W
	SWITCHBANK_0_2
	movwf	EEADR		; calc address:	LAYER * 8 + Address Offset LAYER_TEXTS
	rlf	EEADR, F
	rlf	EEADRH, F
	rlf	EEADR, F
	rlf	EEADRH, F
	rlf	EEADR, F
	rlf	EEADRH, F
	movf	EEADR, W
	andlw	0xf8
	movwf	EEADR
	movf	EEADRH, W
	andlw	0x07
	addlw	LAYER_TEXTS >> 8
	movwf	EEADRH
	SWITCHBANK_2_0

	goto	LCD_PrintText16

;; --------------------------------------------------------------------------
;;  Box Flush Pot Data: Sends all actual Pot values
;; --------------------------------------------------------------------------
BOX_FlushPotData
	movlw	POT0_VALUE	; Pointer to Pot0
	movwf	FSR

	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F

#if EXT16
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
	call	BOX_SendPotValue
	incf	FSR, F
#endif

	return

;; --------------------------------------------------------------------------
;;  Box Send Pot Value:	Sends Value
;;  In:	Value in INDF, Pot in FSR & 0x07
;; --------------------------------------------------------------------------
BOX_SendPotValue
	movf	INDF, W
	movwf	NRPN_DATALO
	clrf	NRPN_DATAHI

TESTX
#if EXT16
	;; Layers <4 :	General Pot Configuration
	IFCLR	ACTIVE_SUPER_LAYER, 2, goto BOX_SendPotGen
#else
	;; Layers <8 :	General Pot Configuration
	IFCLR	ACTIVE_SUPER_LAYER, 3, goto BOX_SendPotGen
#endif

#if EXT16
	;; Layers >= 6 <= 7 : Mixer Pot Configuration
	IFSET	ACTIVE_SUPER_LAYER, 1, goto BOX_SendPotMixer
#else
	;; Layers >= c <= f : Mixer Pot Configuration
	IFSET	ACTIVE_SUPER_LAYER, 2, goto BOX_SendPotMixer
#endif
	
	movf	ACTIVE_SUPER_LAYER, W
#if EXT16
	andlw	0x01
#else
	andlw	0x03
#endif
	call	BOX_GetSuperLayerIndex
	addwf	LAYER, W
	movwf	TMP1
	rlf	TMP1, F
	rlf	TMP1, F
#if EXT16
	rlf	TMP1, F
	rlf	TMP1, W
	andlw	0xf0
#else
	rlf	TMP1, W
	andlw	0xf8
#endif
	movwf	TMP1
	movf	FSR, W
#if EXT16
	andlw	0x0f
#else
	andlw	0x07
#endif
	iorwf	TMP1, W
	goto	BOX_SendPotTable	; Jumps to a very big condition table ;-)


;; --------------------------------------------------------------------------
;;  Box Print Bar: print nice status bar for pot
;;  In:	Value in INDF, Position in FSR & 0x07 (Pot 16 Extension: FSR & 0x0f)
;; --------------------------------------------------------------------------
#if EXT16
BOX_PrintBar
	movf	FSR, W
	andlw	0x07
	iorlw	0x08
	IFSET	FSR, 3, iorlw 0x40	; +0x40 for Pot 8-16
	call	LCD_CursorPos

	swapf	INDF, W		; Pot Value / 16
	andlw	0x07
	call	LCD_SendChar

	return
#else
BOX_PrintBar
	movf	FSR, W
	andlw	0x07
	iorlw	0x08
	call	LCD_CursorPos

	rrf	INDF, W		; Pot Value / 8
	movwf	BAR_TMP
	rrf	BAR_TMP, F
	rrf	BAR_TMP, W
	andlw	0x0f
	movwf	BAR_TMP

	IFCLR	BAR_TMP, 3, goto BOX_PrintBarUpper20
	andlw	0x07
	goto	BOX_PrintBarUpper	
BOX_PrintBarUpper20
	movlw	0x20
BOX_PrintBarUpper
	call	LCD_SendChar

	movf	FSR, W
	andlw	0x07
	iorlw	0x48
	call	LCD_CursorPos

	IFSET	BAR_TMP, 3, goto BOX_PrintBarLower07
	movf	BAR_TMP, W
	andlw	0x07
	goto	BOX_PrintBarLower
BOX_PrintBarLower07
	movlw	0x07
BOX_PrintBarLower
	call	LCD_SendChar

	return
#endif


;; --------------------------------------------------------------------------
;;  BOX Check Test Mode: Sets DISPLAY_MODE = 2 if reset button is pushed 
;;                       during power up
;; --------------------------------------------------------------------------
BOX_CheckTestMode
	;; check if Testmode selected (Reset-Button during Power On)
	; select button 4-7
	SWITCHBANK_0_1
	movlw	1 << PIN_BUT_0123	; 0123 to tristate -> 4567 free
	movwf	TRIS_B_CTRL
	SWITCHBANK_1_0
	nop
	nop
	nop
	nop

	IFSET	PORT_BUTTONS, 3, return

	movlw	0x02
	movwf	DISPLAY_MODE

	return

;; --------------------------------------------------------------------------
;;  BOX Print Test Mode: prints out some debug values
;; --------------------------------------------------------------------------
BOX_PrintTestMode
	movlw	0x00
	call	LCD_CursorPos

	movf	RINGBUFFER_A, W
	call	LCD_PrintValue
	movf	RINGBUFFER_B, W
	call	LCD_PrintValue

	movf	ACTIVE_SUPER_LAYER, W
	call	LCD_PrintValue
	movf	LAYER, W
	call	LCD_PrintValue

	movf	DOUBLECLICK, W
	call	LCD_PrintValue
	movf	LAST_DC_BUTTON, W
	call	LCD_PrintValue

	movf	BUTTON_STATUS, W
	call	LCD_PrintValue
	movf	BUTTON_ACTSTAT, W
	goto	LCD_PrintValue

	
;; --------------------------------------------------------------------------
;;  EEPROM Get Data
;;  In:	LAYER to calc the EEPROM pointer
;; --------------------------------------------------------------------------
EEPROM_GetData
	movlw	DATA_LOAD
	movwf	EEPROM_TASK
	goto	EEPROM_ProcessData

;; --------------------------------------------------------------------------
;;  EEPROM Save Data
;;  In:	LAYER to calc the EEPROM pointer
;; --------------------------------------------------------------------------
EEPROM_SaveData
	call	IRQ_Disable	; disable IRQs (better as nothing...)
	
	movlw	DATA_SAVE
	movwf	EEPROM_TASK
	call	EEPROM_ProcessData

	goto	IRQ_Enable	; IRQs are welcome again

;; --------------------------------------------------------------------------
;;  EEPROM Reset Data
;;  In:	LAYER to calc the EEPROM pointer
;; --------------------------------------------------------------------------
EEPROM_ResetData
	movlw	DATA_RESET
	movwf	EEPROM_TASK
	goto	EEPROM_ProcessData

;; --------------------------------------------------------------------------
;;  EEPROM Process Data
;;  In:	EEPROM_TASK and LAYER to calc the EEPROM pointer
;; --------------------------------------------------------------------------
EEPROM_ProcessData
	movf	SUPER_LAYER, W
	call	BOX_GetSuperLayerIndex
	addwf	LAYER, W
	SWITCHBANK_0_2
	movwf	EEADR		; calc address:	LAYER * 8 + Address Offset EEPROM_CONTENT
	rlf	EEADR, F
	rlf	EEADRH, F
	rlf	EEADR, F
	rlf	EEADRH, F
	rlf	EEADR, F
	rlf	EEADRH, F
#if EXT16
	rlf	EEADR, F	; EXT16: LAYER * 16!
	rlf	EEADRH, F
#endif
	movf	EEADR, W
	andlw	0xf8
	movwf	EEADR
	movf	EEADRH, W
	andlw	0x07
	addlw	EEPROM_CONTENT >> 8
	movwf	EEADRH
	SWITCHBANK_2_0

	movlw	POT0_VALUE	; Pointer to Pot Table
	movwf	FSR
	
EEPROM_ProcessDataLoop
	call	EEPROM_TaskTable ; do something with the data 

	SWITCHBANK_0_2	
	incf	EEADR, F	; increment memory pointer
	SWITCHBANK_2_0
	incf	FSR, F

#if EXT16
	IFCLR	FSR, 4, goto EEPROM_ProcessDataLoop	; repeat until POTF_VALUE feeded
#else
	IFCLR	FSR, 3, goto EEPROM_ProcessDataLoop	; repeat until POT7_VALUE feeded
#endif
	
	return

;; --------------------------------------------------------------------------
;;  EEPROM Task Load
;;  In:	Some predefines by EEPROM_ProcessDataLoop
;; --------------------------------------------------------------------------
EEPROM_Task_Load
	SWITCHBANK_0_3
	bsf	EECON1, EEPGD	; point to program memory
	bsf	EECON1, RD	; EEPROM Read
	nop
	nop
	SWITCHBANK_3_2

	movf	EEDATA, W	; get and store Pot Value
	andlw	0x7f
	movwf	INDF

	bsf	FSR, 4		; FSR pointer to reset table
	movf	EEDATH, W	; get and store Reset Value
	movwf	INDF
	rlf	EEDATA, W	; Shift in LSB
	rlf	INDF, F
	bcf	FSR, 4		; FSR pointer to Pot Table

	SWITCHBANK_2_0
	return

;; --------------------------------------------------------------------------
;;  EEPROM Task Save
;;  In:	Some predefines by EEPROM_ProcessDataLoop
;; --------------------------------------------------------------------------
EEPROM_Task_Save
	SWITCHBANK_0_3
	bsf	EECON1, EEPGD	; point to program memory
	bsf	EECON1, RD	; EEPROM Read
	nop
	nop
	SWITCHBANK_3_2

	movf	EEDATA, W	; extract the LSB of Reset Value
	andlw	0x80		; mask out the rest
	movwf	EEDATA

	movf	INDF, W		; get Pot Value
	iorwf	EEDATA, F	; put it into EEDATA

	SWITCHBANK_2_3

	bsf	EECON1, EEPGD	; point to program memory
	bsf	EECON1, WREN	; EEPROM write enable
	movlw	0x55		; unlock sequence
	movwf	EECON2
	movlw	0xaa
	movwf	EECON2
	bsf	EECON1, WR	; EEPROM write
EEPROM_Task_SaveLoop
	IFSET	EECON1, WR, goto EEPROM_Task_SaveLoop
	
	SWITCHBANK_3_0
	return

;; --------------------------------------------------------------------------
;;  EEPROM Task Reset
;;  In:	Some predefines by EEPROM_ProcessDataLoop
;; --------------------------------------------------------------------------
EEPROM_Task_Reset
	SWITCHBANK_0_3
	bsf	EECON1, EEPGD	; point to program memory
	bsf	EECON1, RD	; EEPROM Read
	nop
	nop
	SWITCHBANK_3_2

	movf	EEDATH, W	; get Reset Value
	movwf	INDF
	rlf	EEDATA, W	; Shift in LSB
	rlf	INDF, F

	SWITCHBANK_2_0
	return






