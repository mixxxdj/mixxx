;
; Layer Routines
;
; Copyright 1999, 2000 by Thorsten Klose
; Commercial use without my permission is strictly forbidden!
;

;; --------------------------------------------------------------------------
;;  Special Sending Routines
;; --------------------------------------------------------------------------

;; #### Layer 1.AWE ###

BOX_SendAWE0_0
	movlw	0x07
	goto	Send_MIDI_Controller
BOX_SendAWE0_1
	movlw	0x0a
	goto	Send_MIDI_Controller
BOX_SendAWE0_2
	movlw	0x0b
	goto	Send_MIDI_Controller
BOX_SendAWE0_3
	movlw	0x01
	goto	Send_MIDI_Controller
BOX_SendAWE0_4
	movlw	0x15
	goto	Send_MIDI_NRPN
BOX_SendAWE0_5
	movlw	0x16
	goto	Send_MIDI_NRPN
BOX_SendAWE0_6
	call	Init_MIDI_NRPN_mult_2
	movlw	0x19
	goto	Send_MIDI_NRPN
BOX_SendAWE0_7
	call	Init_MIDI_NRPN_mult_2
	movlw	0x1a
	goto	Send_MIDI_NRPN

;; #### Layer 2.AWE ###

BOX_SendAWE1_0
	call	Init_MIDI_NRPN_mult_16
	movlw	0x00
	goto	Send_MIDI_NRPN
BOX_SendAWE1_1
	movlw	0x01
	goto	Send_MIDI_NRPN
BOX_SendAWE1_2
	call	Init_MIDI_NRPN_split_80
	movlw	0x11
	goto	Send_MIDI_NRPN
BOX_SendAWE1_3
	call	Init_MIDI_NRPN_bidir
	movlw	0x14
	goto	Send_MIDI_NRPN
BOX_SendAWE1_4
	call	Init_MIDI_NRPN_split_40
	movlw	0x17
	goto	Send_MIDI_NRPN
BOX_SendAWE1_5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x02
	goto	Send_MIDI_NRPN
BOX_SendAWE1_6
	movlw	0x03
	goto	Send_MIDI_NRPN
BOX_SendAWE1_7
	call	Init_MIDI_NRPN_split_80
	movlw	0x12
	goto	Send_MIDI_NRPN

;; #### Layer 3.AWE ###

BOX_SendAWE2_0
	call	Init_MIDI_NRPN_mult_16
	movlw	0x04
	goto	Send_MIDI_NRPN
BOX_SendAWE2_1
	call	Init_MIDI_NRPN_mult_16
	movlw	0x05
	goto	Send_MIDI_NRPN
BOX_SendAWE2_2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x06
	goto	Send_MIDI_NRPN
BOX_SendAWE2_3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x07
	goto	Send_MIDI_NRPN
BOX_SendAWE2_4
	movlw	0x08
	goto	Send_MIDI_NRPN
BOX_SendAWE2_5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x09
	goto	Send_MIDI_NRPN
BOX_SendAWE2_6
	call	Init_MIDI_NRPN_split_80
	movlw	0x13
	goto	Send_MIDI_NRPN
BOX_SendAWE2_7
	call	Init_MIDI_NRPN_split_40
	movlw	0x18
	goto	Send_MIDI_NRPN

;; #### Layer 4.AWE ###

BOX_SendAWE3_0
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0a
	goto	Send_MIDI_NRPN
BOX_SendAWE3_1
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0b
	goto	Send_MIDI_NRPN
BOX_SendAWE3_2
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0c
	goto	Send_MIDI_NRPN
BOX_SendAWE3_3
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0d
	goto	Send_MIDI_NRPN
BOX_SendAWE3_4
	movlw	0x0e
	goto	Send_MIDI_NRPN
BOX_SendAWE3_5
	call	Init_MIDI_NRPN_mult_16
	movlw	0x0f
	goto	Send_MIDI_NRPN
BOX_SendAWE3_6
BOX_SendAWE3_7
	return

;; #### Layer 1.GM ###

BOX_SendGM0_0
	movlw	0x07
	goto	Send_MIDI_Controller
BOX_SendGM0_1
	movlw	0x0a
	goto	Send_MIDI_Controller
BOX_SendGM0_2
	movlw	0x0b
	goto	Send_MIDI_Controller
BOX_SendGM0_3
	movlw	0x01
	goto	Send_MIDI_Controller
BOX_SendGM0_4
	goto	Send_MIDI_Portamento
BOX_SendGM0_5
	movlw	0x5d
	goto	Send_MIDI_Controller
BOX_SendGM0_6
	movlw	0x5b
	goto	Send_MIDI_Controller
BOX_SendGM0_7
	return
	
;; #### Layer 2.GM ###

BOX_SendGM1_0
	movlw	0x63
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_1
	movlw	0x64
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_2
	movlw	0x66
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_3
	movlw	0x0a
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_4
	movlw	0x08
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_5
	movlw	0x09
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_6
	movlw	0x20
	goto	Send_MIDI_NRPN_GM
BOX_SendGM1_7
	movlw	0x21
	goto	Send_MIDI_NRPN_GM


;; #### Layer 1.XG ###

BOX_SendXM0_0
	movlw	0x07
	goto	Send_MIDI_Controller
BOX_SendXM0_1
	movlw	0x0a
	goto	Send_MIDI_Controller
BOX_SendXM0_2
	movlw	0x0b
	goto	Send_MIDI_Controller
BOX_SendXM0_3
	movlw	0x01
	goto	Send_MIDI_Controller
BOX_SendXM0_4
	goto	Send_MIDI_Portamento
BOX_SendXM0_5
	movlw	0x5b
	goto	Send_MIDI_Controller
BOX_SendXM0_6
	movlw	0x5d
	goto	Send_MIDI_Controller
BOX_SendXM0_7
	movlw	0x5e
	goto	Send_MIDI_Controller

;; #### Layer 2.XG ###

BOX_SendXM1_0
	movlw	0x49
	goto	Send_MIDI_Controller
BOX_SendXM1_1
	movlw	0x1b
	goto	Send_MIDI_SysEx
BOX_SendXM1_2
	movlw	0x48
	goto	Send_MIDI_Controller
BOX_SendXM1_3
	movlw	0x17
	goto	Send_MIDI_SysEx
BOX_SendXM1_4
	movlw	0x15
	goto	Send_MIDI_SysEx
BOX_SendXM1_5
	movlw	0x16
	goto	Send_MIDI_SysEx
BOX_SendXM1_6
	movlw	0x4a
	goto	Send_MIDI_Controller
BOX_SendXM1_7
	movlw	0x47
	goto	Send_MIDI_Controller

;; #### Layer 3.XG ###

BOX_SendXM2_0
	movlw	0x69
	goto	Send_MIDI_SysEx
BOX_SendXM2_1
	movlw	0x6a
	goto	Send_MIDI_SysEx
BOX_SendXM2_2
	movlw	0x6b
	goto	Send_MIDI_SysEx
BOX_SendXM2_3
	movlw	0x6c
	goto	Send_MIDI_SysEx
BOX_SendXM2_4
	movlw	0x6d
	goto	Send_MIDI_SysEx
BOX_SendXM2_5
	movlw	0x6e
	goto	Send_MIDI_SysEx
BOX_SendXM2_6
BOX_SendXM2_7
	return

;; #### Layer 4.XG ###

BOX_SendXM3_0
	movlw	0x23
	goto	Send_MIDI_SysEx
BOX_SendXM3_1
	movlw	0x24
	goto	Send_MIDI_SysEx
BOX_SendXM3_2
	movlw	0x25
	goto	Send_MIDI_SysEx
BOX_SendXM3_3
	movlw	0x26
	goto	Send_MIDI_SysEx
BOX_SendXM3_4
	movlw	0x27
	goto	Send_MIDI_SysEx
BOX_SendXM3_5
	movlw	0x28
	goto	Send_MIDI_SysEx
BOX_SendXM3_6
BOX_SendXM3_7
	return

;; #### Layer 5.XG ###

BOX_SendXM4_0
	movlw	0x1d
	goto	Send_MIDI_SysEx
BOX_SendXM4_1
	movlw	0x1e
	goto	Send_MIDI_SysEx
BOX_SendXM4_2
	movlw	0x1f
	goto	Send_MIDI_SysEx
BOX_SendXM4_3
	movlw	0x20
	goto	Send_MIDI_SysEx
BOX_SendXM4_4
	movlw	0x21
	goto	Send_MIDI_SysEx
BOX_SendXM4_5
	movlw	0x22
	goto	Send_MIDI_SysEx
BOX_SendXM4_6
BOX_SendXM4_7
	return

;; #### Layer 6.XG ###

BOX_SendXM5_0
	movlw	0x53
	goto	Send_MIDI_SysEx
BOX_SendXM5_1
	movlw	0x54
	goto	Send_MIDI_SysEx
BOX_SendXM5_2
	movlw	0x55
	goto	Send_MIDI_SysEx
BOX_SendXM5_3
	movlw	0x56
	goto	Send_MIDI_SysEx
BOX_SendXM5_4
	movlw	0x57
	goto	Send_MIDI_SysEx
BOX_SendXM5_5
	movlw	0x58
	goto	Send_MIDI_SysEx
BOX_SendXM5_6
BOX_SendXM5_7
	return


;; --------------------------------------------------------------------------
;;  General Purpose Sending Routine
;;  Ctrler = (SUPER_LAYER & 7 + LAYER & 1) << 3 + POT_number
;;  or with Pot16: Ctrler = (SUPER_LAYER & F + LAYER & 1) << 4 + POT_number
;; --------------------------------------------------------------------------
BOX_SendPotGen
	call	MIDI_Send_Control_Cmd

	movf	ACTIVE_SUPER_LAYER, W
#if EXT16
	andlw	0x03
#else
	andlw	0x07
#endif
	movwf	TMP1
	rlf	TMP1, F

	movf	LAYER, W
	andlw	0x01
	iorwf	TMP1, F
	rlf	TMP1, F
	rlf	TMP1, F
#if EXT16
	rlf	TMP1, F
	rlf	TMP1, W
	andlw	0x70
#else
	rlf	TMP1, W
	andlw	0x78
#endif
	movwf	TMP1
	movf	FSR, W
#if EXT16
	andlw	0x0f
#else
	andlw	0x07
#endif
	iorwf	TMP1, W
	call	_MIDI_SendByte

	movf	INDF, W
	goto	_MIDI_SendByte

;; --------------------------------------------------------------------------
;;  General Purpose Sending Routine for Mixer
;; --------------------------------------------------------------------------
BOX_SendPotMixer
	movf	FSR, W		; get control instr., depending on Pot and Layer
#if EXT16
	andlw	0x0f
#else
	andlw	0x07
	IFSET	ACTIVE_SUPER_LAYER, 0, iorlw 0x08
#endif
	iorlw	0xb0
	call	_MIDI_SendByte

	call	BOX_GetMixerCntrl
	call	_MIDI_SendByte

	movf	INDF, W
	goto	_MIDI_SendByte

;; --------------------------------------------------------------------------
;;  Utility Funs
;; --------------------------------------------------------------------------

Send_NRPN_MSB
	movwf	TMP1			; store controller value

	call	MIDI_Send_Control_Cmd
	
	movlw	0x63
	call	_MIDI_SendByte
	
	movf	TMP1, W
	goto	_MIDI_SendByte

Send_MIDI_Controller
	movwf	TMP1			; store controller number

	call	MIDI_Send_Control_Cmd
	
	movf	TMP1, W
	call	_MIDI_SendByte
	
	movf	NRPN_DATALO, W
	goto	_MIDI_SendByte

Send_MIDI_Portamento
	call	MIDI_Send_Control_Cmd
	
	movlw	0x41
	call	_MIDI_SendByte
	
	movf	NRPN_DATALO, W
	skpz
	movlw	0x7f
	call	_MIDI_SendByte
	
	movlw	0x05
	call	_MIDI_SendByte
	
	movf	NRPN_DATALO, W
	goto	_MIDI_SendByte

Send_MIDI_NRPN
	movwf	TMP1			; store controller number

	call	MIDI_Send_Control_Cmd
	
	movlw	0x62			; send NRPN LSB
	call	_MIDI_SendByte

	movf	TMP1, W
	call	_MIDI_SendByte

	movlw	0x26			; send NRPN Data LSB
	call	_MIDI_SendByte

	movlw	0x20			; add 0x2000 to NRPN
	addwf	NRPN_DATAHI, F
	movf	NRPN_DATALO, W		; make LSB
	andlw	0x7f
	call	_MIDI_SendByte

	movlw	0x06			; send NRPN Data MSB
	call	_MIDI_SendByte

	clrc				; make MSB
	rlf	NRPN_DATALO, W		; roll in bit 7 of LSB
	rlf	NRPN_DATAHI, W
	andlw	0x7f
	goto	_MIDI_SendByte

Send_MIDI_NRPN_GM
	movwf	TMP1			; store controller number

	call	MIDI_Send_Control_Cmd
	
	movlw	0x62			; send NRPN LSB
	call	_MIDI_SendByte

	movf	TMP1, W
	call	_MIDI_SendByte

	movlw	0x06			; send NRPN Data
	call	_MIDI_SendByte

	movf	NRPN_DATALO, W		; make LSB
	andlw	0x7f
	goto	_MIDI_SendByte

Send_MIDI_SysEx
	movwf	TMP1		; store Controller Number

	movlw	0xf0
	call	_MIDI_SendByte

	movlw	0x43
	call	_MIDI_SendByte

	movlw	0x10
	call	_MIDI_SendByte

	movlw	0x4c
	call	_MIDI_SendByte

	movlw	0x08
	call	_MIDI_SendByte

	movf	MIDI_CHANNEL, W			; get control instr. with midi channel
	call	_MIDI_SendByte

	movf	TMP1, W				; send controller number
	call	_MIDI_SendByte

	movf	NRPN_DATALO, W			; send pot value
	call	_MIDI_SendByte

	movlw	0xf7
	goto	_MIDI_SendByte

MIDI_Send_Control_Cmd
	movf	MIDI_CHANNEL, W			; get control instr. with midi channel
	addlw	0xb0
	goto	_MIDI_SendByte

Init_MIDI_NRPN_mult_2
	clrc
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F
	return

Init_MIDI_NRPN_mult_16
	clrc
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F
	return

Init_MIDI_NRPN_split_40
	movlw	0x40
	subwf	NRPN_DATALO, F
	btfss	STATUS, C
	decf	NRPN_DATAHI, F
	return

Init_MIDI_NRPN_bidir
	movf	NRPN_DATALO, F
	xorlw	0x40
	skpnz
	return
	IFSET	NRPN_DATALO, 6, goto Init_MIDI_NRPN_bidir_higher
	movlw	0x40
	subwf	NRPN_DATALO, F
	return
Init_MIDI_NRPN_bidir_higher
	movlw	0xc0
	subwf	NRPN_DATALO, F
	return

Init_MIDI_NRPN_split_80
	clrc
	rlf	NRPN_DATALO, F
	rlf	NRPN_DATAHI, F

	movlw	0x80
	subwf	NRPN_DATALO, F
	btfss	STATUS, C
	decf	NRPN_DATAHI, F
	return





