DATA32 SEGMENT
ASSUME CS:FLAT, DS:FLAT, ES:FLAT
PUBLIC __code_segments
__code_segments:
EXTRN __SEGEND__init_text : BYTE
dd OFFSET FLAT:__SEGSTRT__init_text
dd OFFSET FLAT:__SEGEND__init_text
EXTRN __SEGEND__sched_text : BYTE
dd OFFSET FLAT:__SEGSTRT__sched_text
dd OFFSET FLAT:__SEGEND__sched_text
EXTRN __SEGEND__exit_text : BYTE
dd OFFSET FLAT:__SEGSTRT__exit_text
dd OFFSET FLAT:__SEGEND__exit_text
dd 0
dd 0
PUBLIC __data_segments
__data_segments:
EXTRN __SEGEND__init_data : BYTE
dd OFFSET FLAT:__SEGSTRT__init_data
dd OFFSET FLAT:__SEGEND__init_data
EXTRN __SEGEND__init_setup : BYTE
dd OFFSET FLAT:__SEGSTRT__init_setup
dd OFFSET FLAT:__SEGEND__init_setup
EXTRN __SEGEND__initcall1_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall1_init
dd OFFSET FLAT:__SEGEND__initcall1_init
EXTRN __SEGEND__initcall2_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall2_init
dd OFFSET FLAT:__SEGEND__initcall2_init
EXTRN __SEGEND__initcall3_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall3_init
dd OFFSET FLAT:__SEGEND__initcall3_init
EXTRN __SEGEND__initcall4_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall4_init
dd OFFSET FLAT:__SEGEND__initcall4_init
EXTRN __SEGEND__initcall5_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall5_init
dd OFFSET FLAT:__SEGEND__initcall5_init
EXTRN __SEGEND__initcall6_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall6_init
dd OFFSET FLAT:__SEGEND__initcall6_init
EXTRN __SEGEND__initcall7_init : BYTE
dd OFFSET FLAT:__SEGSTRT__initcall7_init
dd OFFSET FLAT:__SEGEND__initcall7_init
EXTRN __SEGEND__exitcall_exit : BYTE
dd OFFSET FLAT:__SEGSTRT__exitcall_exit
dd OFFSET FLAT:__SEGEND__exitcall_exit
EXTRN __SEGEND___ksymtab_strings : BYTE
dd OFFSET FLAT:__SEGSTRT___ksymtab_strings
dd OFFSET FLAT:__SEGEND___ksymtab_strings
EXTRN __SEGEND___ksymtab : BYTE
dd OFFSET FLAT:__SEGSTRT___ksymtab
dd OFFSET FLAT:__SEGEND___ksymtab
EXTRN __SEGEND___ksymtab_gpl : BYTE
dd OFFSET FLAT:__SEGSTRT___ksymtab_gpl
dd OFFSET FLAT:__SEGEND___ksymtab_gpl
dd 0
dd 0
DATA32 ENDS
