        LIST
; P16F877.INC  Standard Header File, Version 1.00    Microchip Technology, Inc.
        NOLIST

; This header file defines configurations, registers, and other useful bits of
; information for the PIC16F877 microcontroller.  These names are taken to match 
; the data sheets as closely as possible.  

; Note that the processor must be selected before this file is 
; included.  The processor may be selected the following ways:

;       1. Command line switch:
;               C:\ MPASM MYFILE.ASM /PIC16F877
;       2. LIST directive in the source file
;               LIST   P=PIC16F877
;       3. Processor Type entry in the MPASM full-screen interface

;==========================================================================
;
;       Revision History
;
;==========================================================================

;Rev:   Date:    Reason:

;1.12   01/12/00 Changed some bit names, a register name, configuration bits
;                 to match datasheet (DS30292B)
;1.00   08/07/98 Initial Release

;==========================================================================
;
;       Verify Processor
;
;==========================================================================

        IFNDEF __16F877
           MESSG "Processor-header file mismatch.  Verify selected processor."
        ENDIF

;==========================================================================
;
;       Register Definitions
;
;==========================================================================

W                            EQU     H'0000'
F                            EQU     H'0001'

;----- Register Files------------------------------------------------------

INDF                         EQU     H'0000'
TMR0                         EQU     H'0001'
PCL                          EQU     H'0002'
STATUS                       EQU     H'0003'
FSR                          EQU     H'0004'
PORTA                        EQU     H'0005'
PORTB                        EQU     H'0006'
PORTC                        EQU     H'0007'
PORTD                        EQU     H'0008'
PORTE                        EQU     H'0009'
PCLATH                       EQU     H'000A'
INTCON                       EQU     H'000B'
PIR1                         EQU     H'000C'
PIR2                         EQU     H'000D'
TMR1L                        EQU     H'000E'
TMR1H                        EQU     H'000F'
T1CON                        EQU     H'0010'
TMR2                         EQU     H'0011'
T2CON                        EQU     H'0012'
SSPBUF                       EQU     H'0013'
SSPCON                       EQU     H'0014'
CCPR1L                       EQU     H'0015'
CCPR1H                       EQU     H'0016'
CCP1CON                      EQU     H'0017'
RCSTA                        EQU     H'0018'
TXREG                        EQU     H'0019'
RCREG                        EQU     H'001A'
CCPR2L                       EQU     H'001B'
CCPR2H                       EQU     H'001C'
CCP2CON                      EQU     H'001D'
ADRESH                       EQU     H'001E'
ADCON0                       EQU     H'001F'

OPTION_REG                   EQU     H'0081'
TRISA                        EQU     H'0085'
TRISB                        EQU     H'0086'
TRISC                        EQU     H'0087'
TRISD                        EQU     H'0088'
TRISE                        EQU     H'0089'
PIE1                         EQU     H'008C'
PIE2                         EQU     H'008D'
PCON                         EQU     H'008E'
SSPCON2                      EQU     H'0091'
PR2                          EQU     H'0092'
SSPADD                       EQU     H'0093'
SSPSTAT                      EQU     H'0094'
TXSTA                        EQU     H'0098'
SPBRG                        EQU     H'0099'
ADRESL                       EQU     H'009E'
ADCON1                       EQU     H'009F'

EEDATA                       EQU     H'010C'
EEADR                        EQU     H'010D'
EEDATH                       EQU     H'010E'
EEADRH                       EQU     H'010F'

EECON1                       EQU     H'018C'
EECON2                       EQU     H'018D'

;----- STATUS Bits --------------------------------------------------------

IRP                          EQU     H'0007'
RP1                          EQU     H'0006'
RP0                          EQU     H'0005'
NOT_TO                       EQU     H'0004'
NOT_PD                       EQU     H'0003'
Z                            EQU     H'0002'
DC                           EQU     H'0001'
C                            EQU     H'0000'

;----- INTCON Bits --------------------------------------------------------

GIE                          EQU     H'0007'
PEIE                         EQU     H'0006'
T0IE                         EQU     H'0005'
INTE                         EQU     H'0004'
RBIE                         EQU     H'0003'
T0IF                         EQU     H'0002'
INTF                         EQU     H'0001'
RBIF                         EQU     H'0000'

;----- PIR1 Bits ----------------------------------------------------------

PSPIF                        EQU     H'0007'
ADIF                         EQU     H'0006'
RCIF                         EQU     H'0005'
TXIF                         EQU     H'0004'
SSPIF                        EQU     H'0003'
CCP1IF                       EQU     H'0002'
TMR2IF                       EQU     H'0001'
TMR1IF                       EQU     H'0000'

;----- PIR2 Bits ----------------------------------------------------------

EEIF                         EQU     H'0004'
BCLIF                        EQU     H'0003'
CCP2IF                       EQU     H'0000'

;----- T1CON Bits ---------------------------------------------------------

T1CKPS1                      EQU     H'0005'
T1CKPS0                      EQU     H'0004'
T1OSCEN                      EQU     H'0003'
NOT_T1SYNC                   EQU     H'0002'
T1INSYNC                     EQU     H'0002'    ; Backward compatibility only
T1SYNC                       EQU     H'0002'
TMR1CS                       EQU     H'0001'
TMR1ON                       EQU     H'0000'

;----- T2CON Bits ---------------------------------------------------------

TOUTPS3                      EQU     H'0006'
TOUTPS2                      EQU     H'0005'
TOUTPS1                      EQU     H'0004'
TOUTPS0                      EQU     H'0003'
TMR2ON                       EQU     H'0002'
T2CKPS1                      EQU     H'0001'
T2CKPS0                      EQU     H'0000'

;----- SSPCON Bits --------------------------------------------------------

WCOL                         EQU     H'0007'
SSPOV                        EQU     H'0006'
SSPEN                        EQU     H'0005'
CKP                          EQU     H'0004'
SSPM3                        EQU     H'0003'
SSPM2                        EQU     H'0002'
SSPM1                        EQU     H'0001'
SSPM0                        EQU     H'0000'

;----- CCP1CON Bits -------------------------------------------------------

CCP1X                        EQU     H'0005'
CCP1Y                        EQU     H'0004'
CCP1M3                       EQU     H'0003'
CCP1M2                       EQU     H'0002'
CCP1M1                       EQU     H'0001'
CCP1M0                       EQU     H'0000'

;----- RCSTA Bits ---------------------------------------------------------

SPEN                         EQU     H'0007'
RX9                          EQU     H'0006'
RC9                          EQU     H'0006'    ; Backward compatibility only
NOT_RC8                      EQU     H'0006'    ; Backward compatibility only
RC8_9                        EQU     H'0006'    ; Backward compatibility only
SREN                         EQU     H'0005'
CREN                         EQU     H'0004'
ADDEN                        EQU     H'0003'
FERR                         EQU     H'0002'
OERR                         EQU     H'0001'
RX9D                         EQU     H'0000'
RCD8                         EQU     H'0000'    ; Backward compatibility only

;----- CCP2CON Bits -------------------------------------------------------

CCP2X                        EQU     H'0005'
CCP2Y                        EQU     H'0004'
CCP2M3                       EQU     H'0003'
CCP2M2                       EQU     H'0002'
CCP2M1                       EQU     H'0001'
CCP2M0                       EQU     H'0000'

;----- ADCON0 Bits --------------------------------------------------------

ADCS1                        EQU     H'0007'
ADCS0                        EQU     H'0006'
CHS2                         EQU     H'0005'
CHS1                         EQU     H'0004'
CHS0                         EQU     H'0003'
GO                           EQU     H'0002'
NOT_DONE                     EQU     H'0002'
GO_DONE                      EQU     H'0002'
ADON                         EQU     H'0000'

;----- OPTION_REG Bits -----------------------------------------------------

NOT_RBPU                     EQU     H'0007'
INTEDG                       EQU     H'0006'
T0CS                         EQU     H'0005'
T0SE                         EQU     H'0004'
PSA                          EQU     H'0003'
PS2                          EQU     H'0002'
PS1                          EQU     H'0001'
PS0                          EQU     H'0000'

;----- TRISE Bits ---------------------------------------------------------

IBF                          EQU     H'0007'
OBF                          EQU     H'0006'
IBOV                         EQU     H'0005'
PSPMODE                      EQU     H'0004'
TRISE2                       EQU     H'0002'
TRISE1                       EQU     H'0001'
TRISE0                       EQU     H'0000'

;----- PIE1 Bits ----------------------------------------------------------

PSPIE                        EQU     H'0007'
ADIE                         EQU     H'0006'
RCIE                         EQU     H'0005'
TXIE                         EQU     H'0004'
SSPIE                        EQU     H'0003'
CCP1IE                       EQU     H'0002'
TMR2IE                       EQU     H'0001'
TMR1IE                       EQU     H'0000'

;----- PIE2 Bits ----------------------------------------------------------

EEIE                         EQU     H'0004'
BCLIE                        EQU     H'0003'
CCP2IE                       EQU     H'0000'

;----- PCON Bits ----------------------------------------------------------

NOT_POR                      EQU     H'0001'
NOT_BO                       EQU     H'0000'
NOT_BOR                      EQU     H'0000'

;----- SSPCON2 Bits --------------------------------------------------------

GCEN                         EQU     H'0007'
ACKSTAT                      EQU     H'0006'
ACKDT                        EQU     H'0005'
ACKEN                        EQU     H'0004'
RCEN                         EQU     H'0003'
PEN                          EQU     H'0002'
RSEN                         EQU     H'0001'   
SEN                          EQU     H'0000'   

;----- SSPSTAT Bits -------------------------------------------------------

SMP                          EQU     H'0007'
CKE                          EQU     H'0006'
D                            EQU     H'0005'
I2C_DATA                     EQU     H'0005'
NOT_A                        EQU     H'0005'
NOT_ADDRESS                  EQU     H'0005'
D_A                          EQU     H'0005'
DATA_ADDRESS                 EQU     H'0005'
P                            EQU     H'0004'
I2C_STOP                     EQU     H'0004'
S                            EQU     H'0003'
I2C_START                    EQU     H'0003'
R                            EQU     H'0002'
I2C_READ                     EQU     H'0002'
NOT_W                        EQU     H'0002'
NOT_WRITE                    EQU     H'0002'
R_W                          EQU     H'0002'
READ_WRITE                   EQU     H'0002'
UA                           EQU     H'0001'
BF                           EQU     H'0000'

;----- TXSTA Bits ---------------------------------------------------------

CSRC                         EQU     H'0007'
TX9                          EQU     H'0006'
NOT_TX8                      EQU     H'0006'    ; Backward compatibility only
TX8_9                        EQU     H'0006'    ; Backward compatibility only
TXEN                         EQU     H'0005'
SYNC                         EQU     H'0004'
BRGH                         EQU     H'0002'
TRMT                         EQU     H'0001'
TX9D                         EQU     H'0000'
TXD8                         EQU     H'0000'    ; Backward compatibility only

;----- ADCON1 Bits --------------------------------------------------------

ADFM                         EQU     H'0007'
PCFG3                        EQU     H'0003'
PCFG2                        EQU     H'0002'
PCFG1                        EQU     H'0001'
PCFG0                        EQU     H'0000'

;----- EECON1 Bits --------------------------------------------------------

EEPGD                        EQU     H'0007'
WRERR                        EQU     H'0003'
WREN                         EQU     H'0002'
WR                           EQU     H'0001'
RD                           EQU     H'0000'

;==========================================================================
;
;       RAM Definition
;
;==========================================================================

        __MAXRAM H'1FF'
        __BADRAM H'8F'-H'90', H'95'-H'97', H'9A'-H'9D'
        __BADRAM H'105', H'107'-H'109'
        __BADRAM H'185', H'187'-H'189', H'18E'-H'18F'

;==========================================================================
;
;       Configuration Bits
;
;==========================================================================

_CP_ALL                      EQU     H'0FCF'
_CP_HALF                     EQU     H'1FDF'
_CP_UPPER_256                EQU     H'2FEF'
_CP_OFF                      EQU     H'3FFF'
_DEBUG_ON                    EQU     H'37FF'
_DEBUG_OFF                   EQU     H'3FFF'
_WRT_ENABLE_ON               EQU     H'3FFF'
_WRT_ENABLE_OFF              EQU     H'3DFF'
_CPD_ON                      EQU     H'3EFF'
_CPD_OFF                     EQU     H'3FFF'
_LVP_ON                      EQU     H'3FFF'
_LVP_OFF                     EQU     H'3F7F'
_BODEN_ON                    EQU     H'3FFF'
_BODEN_OFF                   EQU     H'3FBF'
_PWRTE_OFF                   EQU     H'3FFF'
_PWRTE_ON                    EQU     H'3FF7'
_WDT_ON                      EQU     H'3FFF'
_WDT_OFF                     EQU     H'3FFB'
_LP_OSC                      EQU     H'3FFC'
_XT_OSC                      EQU     H'3FFD'
_HS_OSC                      EQU     H'3FFE'
_RC_OSC                      EQU     H'3FFF'

        LIST
