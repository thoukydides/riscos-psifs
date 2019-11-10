;   File        : sema.s
;   Date        : 19-Sep-02
;   Author      : © A.Thoukydides, 1998-2002, 2019
;   Description : Semaphore handling for the PsiFS module.
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

; Include my header files

        GET     AT:Hdr.macros

; Exported symbols

        EXPORT  sema_claim
        EXPORT  sema_release

; Constants

I                   * 1 << 27           ; I bit in PC

; Place in a code area

        AREA    |C$$code|, REL, CODE, READONLY

; bool sema_claim(semaphore *sema)
sema_claim
        LocalLabels
        ; Original version of routine not safe on ARM2:
        ;   MOV     a2, #1              ; Value to claim semaphore with
        ;   SWP     a2, a2, [a1]        ; Swap with the semaphore contents
        ;   RSB     a1, a2, #1          ; Successful if not previously claimed
        ;   MOVS    pc, lr              ; Return from subroutine
        ; New version is slightly slower and ineligant but safe:
        TEQP    pc, #I + 3              ; Disable interrupts
        LDR     a2, [a1]                ; Read current value of semaphore
        RSBS    a2, a2, #1              ; Successful if not previously claimed
        STRNE   a2, [a1]                ; Claim the semaphore if possible
        MOV     a1, a2                  ; Copy the result
        MOVS    pc, lr                  ; Return from subroutine

; void sema_release(semaphore *sema)
sema_release
        LocalLabels
        MOV     a2, #0                  ; Value to release semaphore with
        STR     a2, [a1]                ; Clear the semaphore
        MOVS    pc, lr                  ; Return from subroutine

        END
