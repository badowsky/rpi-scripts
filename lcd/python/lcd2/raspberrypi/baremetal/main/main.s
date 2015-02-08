.section .init
.global _start

_start:
b init

.section .text
init:
mov sp, #0x8000
bl notmain

hang:
b hang