;
; Condition Tables
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;


	ORG	0xf00

MIDI_GetCMax
	movwf	MIDI_TMP
	rrf	MIDI_TMP, F
	rrf	MIDI_TMP, F
	rrf	MIDI_TMP, F
	rrf	MIDI_TMP, W
	andlw	0x07
	addwf	PCL, F
	retlw	0x02
	retlw	0x02
	retlw	0x02
	retlw	0x02
	retlw	0x01
	retlw	0x01
	retlw	0x02
	retlw	0x00

;; ==========================================================================

EEPROM_TaskTable
	movf	EEPROM_TASK, W
	addwf	PCL, F
	goto	EEPROM_Task_Load
	goto	EEPROM_Task_Save
	goto	EEPROM_Task_Reset

;; ==========================================================================

BOX_GetSuperLayerIndex
	addwf	PCL, F
	retlw	0x00
	retlw	0x06
	retlw	0x0c
	retlw	0x12
	retlw	0x18
	retlw	0x1e
	retlw	0x24
	retlw	0x2a
	retlw	0x30
	retlw	0x36
	retlw	0x3c
	retlw	0x42
	retlw	0x48
	retlw	0x4e
	retlw	0x54
	retlw	0x5a

BOX_GetMixerCntrl
	movf	LAYER, W
	addwf	PCL, F
	retlw	0x07
	retlw	0x0a
	retlw	0x01
	retlw	0x5b
	retlw	0x5d
	retlw	0x5e

;; ==========================================================================

#if EXT16
BOX_GetADCValue
	movf	FSR, W
	andlw	0x08
	bnz	BOX_SetADC_Chn1
BOX_SetADC_Chn0
	retlw	0x81 | (0 << 3)
BOX_SetADC_Chn1
	retlw	0x81 | (1 << 3)

ADC_GetMUXValue
	movf	FSR, W
	andlw	0x07
	addwf	PCL, F
	retlw	3
	retlw	0
	retlw	1
	retlw	2
	retlw	4
	retlw	6
	retlw	7
	retlw	5
#else
BOX_GetADCValue
	movf	FSR, W
	andlw	0x07
	addwf	PCL, F
	retlw	0x81 | (0 << 3)
	retlw	0x81 | (1 << 3)
	retlw	0x81 | (2 << 3)
	retlw	0x81 | (3 << 3)
	retlw	0x81 | (4 << 3)
	retlw	0x81 | (5 << 3)
	retlw	0x81 | (6 << 3)
	retlw	0x81 | (7 << 3)
#endif

;; ==========================================================================

BOX_SendPotTable
	addwf	PCL, F

#if EXT16
	;; Layer 04-1
	goto	BOX_SendAWE0_0
	goto	BOX_SendAWE0_1
	goto	BOX_SendAWE0_2
	goto	BOX_SendAWE0_3
	goto	BOX_SendAWE0_4
	goto	BOX_SendAWE0_5
	goto	BOX_SendAWE0_6
	goto	BOX_SendAWE0_7

	goto	BOX_SendAWE1_0
	goto	BOX_SendAWE1_1
	goto	BOX_SendAWE1_2
	goto	BOX_SendAWE1_3
	goto	BOX_SendAWE1_4
	goto	BOX_SendAWE1_5
	goto	BOX_SendAWE1_6
	goto	BOX_SendAWE1_7

	;; Layer 04-2
	goto	BOX_SendAWE2_0
	goto	BOX_SendAWE2_1
	goto	BOX_SendAWE2_2
	goto	BOX_SendAWE2_3
	goto	BOX_SendAWE2_4
	goto	BOX_SendAWE2_5
	goto	BOX_SendAWE2_6
	goto	BOX_SendAWE2_7

	goto	BOX_SendAWE3_0
	goto	BOX_SendAWE3_1
	goto	BOX_SendAWE3_2
	goto	BOX_SendAWE3_3
	goto	BOX_SendAWE3_4
	goto	BOX_SendAWE3_5
	goto	BOX_SendAWE3_6
	goto	BOX_SendAWE3_7

	;; Layer 04-3
	goto	BOX_SendAWE0_0
	goto	BOX_SendAWE0_1
	goto	BOX_SendAWE0_2
	goto	BOX_SendAWE0_3
	goto	BOX_SendAWE0_4
	goto	BOX_SendAWE0_5
	goto	BOX_SendAWE0_6
	goto	BOX_SendAWE0_7

	goto	BOX_SendAWE1_0
	goto	BOX_SendAWE1_1
	goto	BOX_SendAWE1_2
	goto	BOX_SendAWE1_3
	goto	BOX_SendAWE1_4
	goto	BOX_SendAWE1_5
	goto	BOX_SendAWE1_6
	goto	BOX_SendAWE1_7

	;; Layer 04-4
	goto	BOX_SendAWE2_0
	goto	BOX_SendAWE2_1
	goto	BOX_SendAWE2_2
	goto	BOX_SendAWE2_3
	goto	BOX_SendAWE2_4
	goto	BOX_SendAWE2_5
	goto	BOX_SendAWE2_6
	goto	BOX_SendAWE2_7

	goto	BOX_SendAWE3_0
	goto	BOX_SendAWE3_1
	goto	BOX_SendAWE3_2
	goto	BOX_SendAWE3_3
	goto	BOX_SendAWE3_4
	goto	BOX_SendAWE3_5
	goto	BOX_SendAWE3_6
	goto	BOX_SendAWE3_7

	;; Layer 04-5
	goto	BOX_SendGM0_0
	goto	BOX_SendGM0_1
	goto	BOX_SendGM0_2
	goto	BOX_SendGM0_3
	goto	BOX_SendGM0_4
	goto	BOX_SendGM0_5
	goto	BOX_SendGM0_6
	goto	BOX_SendGM0_7

	goto	BOX_SendGM1_0
	goto	BOX_SendGM1_1
	goto	BOX_SendGM1_2
	goto	BOX_SendGM1_3
	goto	BOX_SendGM1_4
	goto	BOX_SendGM1_5
	goto	BOX_SendGM1_6
	goto	BOX_SendGM1_7

	;; Layer 04-6
	goto	BOX_SendGM0_0
	goto	BOX_SendGM0_1
	goto	BOX_SendGM0_2
	goto	BOX_SendGM0_3
	goto	BOX_SendGM0_4
	goto	BOX_SendGM0_5
	goto	BOX_SendGM0_6
	goto	BOX_SendGM0_7

	goto	BOX_SendGM1_0
	goto	BOX_SendGM1_1
	goto	BOX_SendGM1_2
	goto	BOX_SendGM1_3
	goto	BOX_SendGM1_4
	goto	BOX_SendGM1_5
	goto	BOX_SendGM1_6
	goto	BOX_SendGM1_7

#else
	;; Layer 09-1
	goto	BOX_SendAWE0_0
	goto	BOX_SendAWE0_1
	goto	BOX_SendAWE0_2
	goto	BOX_SendAWE0_3
	goto	BOX_SendAWE0_4
	goto	BOX_SendAWE0_5
	goto	BOX_SendAWE0_6
	goto	BOX_SendAWE0_7

	;; Layer 09-2
	goto	BOX_SendAWE1_0
	goto	BOX_SendAWE1_1
	goto	BOX_SendAWE1_2
	goto	BOX_SendAWE1_3
	goto	BOX_SendAWE1_4
	goto	BOX_SendAWE1_5
	goto	BOX_SendAWE1_6
	goto	BOX_SendAWE1_7

	;; Layer 09-3
	goto	BOX_SendAWE2_0
	goto	BOX_SendAWE2_1
	goto	BOX_SendAWE2_2
	goto	BOX_SendAWE2_3
	goto	BOX_SendAWE2_4
	goto	BOX_SendAWE2_5
	goto	BOX_SendAWE2_6
	goto	BOX_SendAWE2_7

	;; Layer 09-4
	goto	BOX_SendAWE3_0
	goto	BOX_SendAWE3_1
	goto	BOX_SendAWE3_2
	goto	BOX_SendAWE3_3
	goto	BOX_SendAWE3_4
	goto	BOX_SendAWE3_5
	goto	BOX_SendAWE3_6
	goto	BOX_SendAWE3_7

	;; Layer 09-5
	goto	BOX_SendGM0_0
	goto	BOX_SendGM0_1
	goto	BOX_SendGM0_2
	goto	BOX_SendGM0_3
	goto	BOX_SendGM0_4
	goto	BOX_SendGM0_5
	goto	BOX_SendGM0_6
	goto	BOX_SendGM0_7

	;; Layer 09-6
	goto	BOX_SendGM1_0
	goto	BOX_SendGM1_1
	goto	BOX_SendGM1_2
	goto	BOX_SendGM1_3
	goto	BOX_SendGM1_4
	goto	BOX_SendGM1_5
	goto	BOX_SendGM1_6
	goto	BOX_SendGM1_7


	;; Layer 10-1
	goto	BOX_SendAWE0_0
	goto	BOX_SendAWE0_1
	goto	BOX_SendAWE0_2
	goto	BOX_SendAWE0_3
	goto	BOX_SendAWE0_4
	goto	BOX_SendAWE0_5
	goto	BOX_SendAWE0_6
	goto	BOX_SendAWE0_7

	;; Layer 10-2
	goto	BOX_SendAWE1_0
	goto	BOX_SendAWE1_1
	goto	BOX_SendAWE1_2
	goto	BOX_SendAWE1_3
	goto	BOX_SendAWE1_4
	goto	BOX_SendAWE1_5
	goto	BOX_SendAWE1_6
	goto	BOX_SendAWE1_7

	;; Layer 10-3
	goto	BOX_SendAWE2_0
	goto	BOX_SendAWE2_1
	goto	BOX_SendAWE2_2
	goto	BOX_SendAWE2_3
	goto	BOX_SendAWE2_4
	goto	BOX_SendAWE2_5
	goto	BOX_SendAWE2_6
	goto	BOX_SendAWE2_7

	;; Layer 10-4
	goto	BOX_SendAWE3_0
	goto	BOX_SendAWE3_1
	goto	BOX_SendAWE3_2
	goto	BOX_SendAWE3_3
	goto	BOX_SendAWE3_4
	goto	BOX_SendAWE3_5
	goto	BOX_SendAWE3_6
	goto	BOX_SendAWE3_7

	;; Layer 10-5
	goto	BOX_SendGM0_0
	goto	BOX_SendGM0_1
	goto	BOX_SendGM0_2
	goto	BOX_SendGM0_3
	goto	BOX_SendGM0_4
	goto	BOX_SendGM0_5
	goto	BOX_SendGM0_6
	goto	BOX_SendGM0_7

	;; Layer 10-6
	goto	BOX_SendGM1_0
	goto	BOX_SendGM1_1
	goto	BOX_SendGM1_2
	goto	BOX_SendGM1_3
	goto	BOX_SendGM1_4
	goto	BOX_SendGM1_5
	goto	BOX_SendGM1_6
	goto	BOX_SendGM1_7

#endif

	;; Layer 11-1
	goto	BOX_SendXM0_0
	goto	BOX_SendXM0_1
	goto	BOX_SendXM0_2
	goto	BOX_SendXM0_3
	goto	BOX_SendXM0_4
	goto	BOX_SendXM0_5
	goto	BOX_SendXM0_6
	goto	BOX_SendXM0_7

	;; Layer 11-2
	goto	BOX_SendXM1_0
	goto	BOX_SendXM1_1
	goto	BOX_SendXM1_2
	goto	BOX_SendXM1_3
	goto	BOX_SendXM1_4
	goto	BOX_SendXM1_5
	goto	BOX_SendXM1_6
	goto	BOX_SendXM1_7

	;; Layer 11-3
	goto	BOX_SendXM2_0
	goto	BOX_SendXM2_1
	goto	BOX_SendXM2_2
	goto	BOX_SendXM2_3
	goto	BOX_SendXM2_4
	goto	BOX_SendXM2_5
	goto	BOX_SendXM2_6
	goto	BOX_SendXM2_7

	;; Layer 11-4
	goto	BOX_SendXM3_0
	goto	BOX_SendXM3_1
	goto	BOX_SendXM3_2
	goto	BOX_SendXM3_3
	goto	BOX_SendXM3_4
	goto	BOX_SendXM3_5
	goto	BOX_SendXM3_6
	goto	BOX_SendXM3_7

	;; Layer 11-5
	goto	BOX_SendXM4_0
	goto	BOX_SendXM4_1
	goto	BOX_SendXM4_2
	goto	BOX_SendXM4_3
	goto	BOX_SendXM4_4
	goto	BOX_SendXM4_5
	goto	BOX_SendXM4_6
	goto	BOX_SendXM4_7

	;; Layer 11-6
	goto	BOX_SendXM5_0
	goto	BOX_SendXM5_1
	goto	BOX_SendXM5_2
	goto	BOX_SendXM5_3
	goto	BOX_SendXM5_4
	goto	BOX_SendXM5_5
	goto	BOX_SendXM5_6
	goto	BOX_SendXM5_7


	;; Layer 12-1
	goto	BOX_SendXM0_0
	goto	BOX_SendXM0_1
	goto	BOX_SendXM0_2
	goto	BOX_SendXM0_3
	goto	BOX_SendXM0_4
	goto	BOX_SendXM0_5
	goto	BOX_SendXM0_6
	goto	BOX_SendXM0_7

	;; Layer 12-2
	goto	BOX_SendXM1_0
	goto	BOX_SendXM1_1
	goto	BOX_SendXM1_2
	goto	BOX_SendXM1_3
	goto	BOX_SendXM1_4
	goto	BOX_SendXM1_5
	goto	BOX_SendXM1_6
	goto	BOX_SendXM1_7

	;; Layer 12-3
	goto	BOX_SendXM2_0
	goto	BOX_SendXM2_1
	goto	BOX_SendXM2_2
	goto	BOX_SendXM2_3
	goto	BOX_SendXM2_4
	goto	BOX_SendXM2_5
	goto	BOX_SendXM2_6
	goto	BOX_SendXM2_7

	;; Layer 12-4
	goto	BOX_SendXM3_0
	goto	BOX_SendXM3_1
	goto	BOX_SendXM3_2
	goto	BOX_SendXM3_3
	goto	BOX_SendXM3_4
	goto	BOX_SendXM3_5
	goto	BOX_SendXM3_6
	goto	BOX_SendXM3_7

	;; Layer 12-5
	goto	BOX_SendXM4_0
	goto	BOX_SendXM4_1
	goto	BOX_SendXM4_2
	goto	BOX_SendXM4_3
	goto	BOX_SendXM4_4
	goto	BOX_SendXM4_5
	goto	BOX_SendXM4_6
	goto	BOX_SendXM4_7

	;; Layer 12-6
	goto	BOX_SendXM5_0
	goto	BOX_SendXM5_1
	goto	BOX_SendXM5_2
	goto	BOX_SendXM5_3
	goto	BOX_SendXM5_4
	goto	BOX_SendXM5_5
	goto	BOX_SendXM5_6
	goto	BOX_SendXM5_7



	END




