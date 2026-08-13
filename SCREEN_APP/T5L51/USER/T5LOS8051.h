#ifndef T5LOS8051_H
#define T5LOS8051_H

#include "compiler_compat.h"

/* Standard 8051 core registers used by the application. */
T5L_SFR(P0,        0x80);
T5L_SFR(SP,        0x81);
T5L_SFR(DPL,       0x82);
T5L_SFR(DPH,       0x83);
T5L_SFR(PCON,      0x87);
T5L_SFR(TCON,      0x88);
T5L_SBIT(TF1,      0x8F);
T5L_SBIT(TR1,      0x8E);
T5L_SBIT(TF0,      0x8D);
T5L_SBIT(TR0,      0x8C);
T5L_SBIT(IE1,      0x8B);
T5L_SBIT(IT1,      0x8A);
T5L_SBIT(IE0,      0x89);
T5L_SBIT(IT0,      0x88);
T5L_SFR(TMOD,      0x89);
T5L_SFR(TL0,       0x8A);
T5L_SFR(TL1,       0x8B);
T5L_SFR(TH0,       0x8C);
T5L_SFR(TH1,       0x8D);
T5L_SFR(CKCON,     0x8E);
T5L_SFR(P1,        0x90);
T5L_SFR(DPC,       0x93);
T5L_SFR(PAGESEL,   0x94);
T5L_SFR(D_PAGESEL, 0x95);

/* UART2 (8051 UART0 register set in DWIN naming). */
T5L_SFR(SCON0,     0x98);
T5L_SBIT(TI0,      0x99);
T5L_SBIT(RI0,      0x98);
T5L_SFR(SBUF0,     0x99);
T5L_SFR(SREL0L,    0xAA);
T5L_SFR(SREL0H,    0xBA);

T5L_SFR(SCON1,     0x9B);
T5L_SFR(SBUF1,     0x9C);
T5L_SFR(SREL1L,    0x9D);
T5L_SFR(SREL1H,    0xBB);

/* Interrupt control. */
T5L_SFR(IEN2,      0x9A);
T5L_SFR(P2,        0xA0);
T5L_SFR(IEN0,      0xA8);
T5L_SBIT(EA,       0xAF);
T5L_SBIT(ET2,      0xAD);
T5L_SBIT(ES0,      0xAC);
T5L_SBIT(ET1,      0xAB);
T5L_SBIT(EX1,      0xAA);
T5L_SBIT(ET0,      0xA9);
T5L_SBIT(EX0,      0xA8);
T5L_SFR(IP0,       0xA9);
T5L_SFR(P3,        0xB0);
T5L_SFR(IEN1,      0xB8);
T5L_SBIT(ES3R,     0xBD);
T5L_SBIT(ES3T,     0xBC);
T5L_SBIT(ES2R,     0xBB);
T5L_SBIT(ES2T,     0xBA);
T5L_SBIT(ECAN,     0xB9);
T5L_SFR(IP1,       0xB9);
T5L_SFR(IRCON2,    0xBF);
T5L_SFR(IRCON,     0xC0);
T5L_SBIT(TF2,      0xC6);

/* Timer 2. */
T5L_SFR(T2CON,     0xC8);
T5L_SBIT(TR2,      0xC8);
T5L_SFR(TRL2L,     0xCA);
T5L_SFR(TRL2H,     0xCB);
T5L_SFR(TL2,       0xCC);
T5L_SFR(TH2,       0xCD);

T5L_SFR(PSW,       0xD0);
T5L_SBIT(CY,       0xD7);
T5L_SBIT(AC,       0xD6);
T5L_SBIT(F0,       0xD5);
T5L_SBIT(RS1,      0xD4);
T5L_SBIT(RS0,      0xD3);
T5L_SBIT(OV,       0xD2);
T5L_SBIT(F1,       0xD1);
T5L_SBIT(P,        0xD0);
T5L_SFR(ADCON,     0xD8);
T5L_SFR(ACC,       0xE0);
T5L_SFR(B,         0xF0);

/* DGUS variable-memory access interface. */
T5L_SFR(RAMMODE,   0xF8);
T5L_SBIT(APP_REQ,  0xFF);
T5L_SBIT(APP_EN,   0xFE);
T5L_SBIT(APP_RW,   0xFD);
T5L_SBIT(APP_ACK,  0xFC);
T5L_SFR(ADR_H,     0xF1);
T5L_SFR(ADR_M,     0xF2);
T5L_SFR(ADR_L,     0xF3);
T5L_SFR(ADR_INC,   0xF4);
T5L_SFR(DATA3,     0xFA);
T5L_SFR(DATA2,     0xFB);
T5L_SFR(DATA1,     0xFC);
T5L_SFR(DATA0,     0xFD);

/* UART4 / UART5 extensions. */
T5L_SFR(SCON2T,      0x96);
T5L_SFR(SCON2R,      0x97);
T5L_SFR(BODE2_DIV_H, 0xD9);
T5L_SFR(BODE2_DIV_L, 0xE7);
T5L_SFR(SBUF2_TX,    0x9E);
T5L_SFR(SBUF2_RX,    0x9F);
T5L_SFR(SCON3T,      0xA7);
T5L_SFR(SCON3R,      0xAB);
T5L_SFR(BODE3_DIV_H, 0xAE);
T5L_SFR(BODE3_DIV_L, 0xAF);
T5L_SFR(SBUF3_TX,    0xAC);
T5L_SFR(SBUF3_RX,    0xAD);

/* CAN / GPIO / arithmetic extensions. */
T5L_SFR(CAN_CR,    0x8F);
T5L_SFR(CAN_IR,    0x91);
T5L_SFR(CAN_ET,    0xE8);
T5L_SFR(P0MDOUT,   0xB7);
T5L_SFR(P1MDOUT,   0xBC);
T5L_SFR(P2MDOUT,   0xBD);
T5L_SFR(P3MDOUT,   0xBE);
T5L_SFR(MUX_SEL,   0xC9);
T5L_SFR(PORTDRV,   0xF9);
T5L_SFR(MAC_MODE,  0xE5);
T5L_SFR(DIV_MODE,  0xE6);
T5L_SFR(EXADR,     0xFE);
T5L_SFR(EXDATA,    0xFF);

#endif
