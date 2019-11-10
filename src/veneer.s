;   File        : veneer.s
;   Date        : 19-Sep-02
;   Author      : © A.Thoukydides, 1998-2002, 2019
;   Description : Filing system entry point veneers for the PsiFS module.
;
;   License     : PsiFS is free software: you can redistribute it and/or
;                 modify it under the terms of the GNU General Public License
;                 as published by the Free Software Foundation, either
;                 version 3 of the License, or (at your option) any later
;                 version.
;   
;                 PsiFS is distributed in the hope that it will be useful,
;                 but WITHOUT ANY WARRANTY; without even the implied warranty
;                 of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
;                 the GNU General Public License for more details.
;   
;                 You should have received a copy of the GNU General Public
;                 License along with PsiFS. If not, see
;                 <http://www.gnu.org/licenses/>.

; Include system header files

        GET     OS:Hdr.OS
        GET     OS:Hdr.Types

; Include my header files

        GET     AT:Hdr.macros

; Imported symbols

        IMPORT  |Image$$RO$$Base|
        IMPORT  |_Lib$Reloc$Off$DP|

; Place in a code area

        AREA    |C$$code|, REL, CODE, READONLY

        ;   Syntax      : <label> Veneer
        ;   Parameters  : label - The symbol to use for the veneer. The
        ;                         corresponding C function should have the same
        ;                         name but with "_handler" appended.
        ;   Description : Generate veneer code to a C function for use as a
        ;                 filing system entry point.
        MACRO
$label  Veneer
        EXPORT  $label
        IMPORT  $label._handler
$label  LDR     r8, = $label._handler   ; Address of the C handler function
        B       veneer_common           ; Pass on to the common code
        MEND

        ;   Parameters  : r8    - Address of C handler function.
        ;                 r12   - Pointer to private word.
        ;   Returns     : -
        ;   Description : Call the specified C handler function.
veneer_common
        LocalLabels
        STMFD   sp!, {r0-r12, lr}       ; Stack registers
        MOV     sl, sp, LSR #20         ; Get top 12 bits of stack pointer
        MOV     sl, sl, LSL #20         ; Round down to megabyte boundary
        LDMIA   sl, {v1, v2}            ; Save any previous reloc modifiers
        LDR     r12, [r12]              ; Get value of private word
        LDMIB   r12, {fp, r12}          ; Read our reloc modifiers
        STMIA   sl,  {fp, r12}          ; Store at the base of the stack
        MOV     fp, #0                  ; Halt the C backtrace here
        &       |_Lib$Reloc$Off$DP| + &E28AA000; 'ADD sl, sl, #0' + reloc
        MOV     a1, sp                  ; Pointer to the registers on the stack
        MOV     lr, pc                  ; Return address after function call
        MOV     pc, r8                  ; Call the C function
        &       |_Lib$Reloc$Off$DP| + &E24AA000; 'SUB sl, sl, #0' + reloc
        STMIA   sl, {v1, v2}            ; Restore previous reloc modifiers
        MOVS    lr, r0                  ; Temporary copy of return value
        LDMFD   sp!, {r0-r12}           ; Registers returned by function
        MOVNE   r0, lr                  ; Overwrite with any error pointer
        MOV     lr, pc                  ; Copy of status flags
        BIC     lr, lr, #Z + C + V      ; Clear the status flags
        ORRNE   lr, lr, #V              ; Set overflow flag if error returned
        TEQ     r1, #0                  ; Is the returned r1 value 0
        ORREQ   lr, lr, #C              ; Set carry flag if r1 is 0
        TEQ     r8, #0                  ; Is the returned r8 value 0
        ORRNE   lr, lr, #Z              ; Set zero flag if r8 is not 0
        TEQP    lr, #0                  ; Set the modified flags
        NOP                             ; No-op to prevent contention
        LDMFD   sp!, {pc}               ; Return to the original caller

; Veneers for the filing system entry points

fsentry_open        Veneer
fsentry_getbytes    Veneer
fsentry_putbytes    Veneer
fsentry_args        Veneer
fsentry_close       Veneer
fsentry_file        Veneer
fsentry_func        Veneer
fsentry_gbpb        Veneer

; fsentry_free also requires a change from USR to SVC mode

        EXPORT  fsentry_free
        IMPORT  fsentry_free_handler

        ;   Parameters  : r0    - Reason code.
        ;                 r1    - Filing system number.
        ;                 r2    - Pointer to buffer or filename.
        ;                 r3    - Pointer to device name or ID.
        ;                 r6    - Pointer to special field.
        ;                 r12   - Pointer to private word.
        ;   Returns     : r0    - Length of name (reason code 1 only).
        ;   Description : Veneer for the C handler function to handle
        ;                 interactive free space requests.
fsentry_free
        STMFD   sp!, {r8}               ; Return address is already on the stack
        LDR     r8, = fsentry_free_handler; Address of the C handler function
        MOV     lr, pc                  ; Copy the program counter for the flags
        TST     lr, #3                  ; Check if in USR mode
        BNE     fsentry_free_svc        ; Pass on to the common code if not USR
        SWI     XOS_EnterOS             ; Enter SVC mode
        BL      veneer_common           ; Pass on to the common code
        MOV     lr, pc                  ; Copy the program counter for the flags
        TEQP    lr, #3                  ; Return to USR mode
        NOP                             ; Wait for the mode change to complete
        LDMFD   sp!, {r8, pc}           ; Return to the original caller
fsentry_free_svc
        BL      veneer_common           ; Pass on to the common code if not USR
        LDMFD   sp!, {r8, pc}           ; Return to the original caller

        END
