#ifndef T5L_COMPILER_COMPAT_H
#define T5L_COMPILER_COMPAT_H

/*
 * Keep the application source shared between Keil C51 and SDCC.
 * Hardware registers use the absolute bit address for T5L SBITs.
 */
#if defined(SDCC) || defined(__SDCC)
  #define T5L_XDATA __xdata
  #define T5L_IDATA __idata
  #define T5L_CODE  __code
  #define T5L_SFR(name, address) __sfr __at(address) name
  #define T5L_SBIT(name, address) __sbit __at(address) name
  #define T5L_ISR(vector) __interrupt(vector)
#else
  #define T5L_XDATA xdata
  #define T5L_IDATA idata
  #define T5L_CODE  code
  #define T5L_SFR(name, address) sfr name = address
  #define T5L_SBIT(name, address) sbit name = address
  #define T5L_ISR(vector) interrupt vector
#endif

#endif
