format PE console

; Romanyuk Andrey BSE-194
; HOMEWORK #1
; ==== Plus Calculator ====
; Example: 10 + 20 = 30
; Example: -10 + 5 = -5
; Example: str + 10 = 0 etc.

entry start


include '..\..\INCLUDE\win32a.inc'

section '.data' data readable writable

        messageA db 'Please, input A: ', 0
        messageB db 'Please, input B: ', 0

        formatNumber db '%d', 0
        formatEnd db 'A + B = %d', 0

        A dd ?
        B dd ?
        Sum dd ?

        NULL = 0

section '.code' code readable executable

        start:
                push messageA
                call [printf]

                push A
                push formatNumber
                call [scanf]

                push messageB
                call [printf]

                push B
                push formatNumber
                call [scanf]

                mov ecx, [A]
                add ecx, [B]

                push ecx
                push formatEnd
                call [printf]

                call [getch]

                push NULL
                call [ExitProcess]

section '.idata' import data readable

        library kernel, 'kernel32.dll',\
                msvcrt, 'msvcrt.dll'

        import kernel,\
               ExitProcess, 'ExitProcess'

        import msvcrt,\
               printf, 'printf',\
               scanf, 'scanf',\
               getch, '_getch'