; T5L OS application marker and alternate entry point.
; The T5L loader expects FF FF "DWINT5" at 0x00F8 and an LJMP at 0x0100.

        .module startup_sdcc
        .globl __sdcc_gsinit_startup

        .area T5LSIG (ABS, CODE)
        .org 0x00F8
        .db 0xFF, 0xFF
        .ascii "DWINT5"
        ljmp __sdcc_gsinit_startup
