format PE console

; Romanyuk Andrey BSE-194
; HOMEWORK #1
; ==== Calculator++ ====
; Example: 10 * 20 = 200
; Example: -10 + 5 = -5
; Example: 10 - 10 = 0
; Example: 1000 / 10 = 100

entry start


include '..\..\INCLUDE\win32a.inc'

section '.data' data readable writable

        messageA db 'Please, input A: ', 0
        messageB db 'Please, input B: ', 0
        ; INFORMATION LINE:
        messageI db 'Supported operations: +, -, *, /', 10, 13, 0
        messageO db 'Please, input operation: ',0

        formatChar db '%s', 0
        formatNumber db '%d', 0
        formatEnd db 'A %s B = %d', 0

        A dd ?
        B dd ?
        C dd ?
        O db ?


        ERROR_MESSAGE db 'Incorrect operation', 0


        NULL = 0

section '.code' code readable executable

        start:
                push messageA
                call [printf]

                ; INPUT FIRST NUMBER
                push A
                push formatNumber
                call [scanf]

                push messageB
                call [printf]

                ; INPUT SECOND NUMBER
                push B
                push formatNumber
                call [scanf]

                push messageI
                call [printf]

                push messageO
                call [printf]

                ; INPUT OPERATION SYMBOL
                push O
                push formatChar
                call [scanf]

                mov esi, O
                mov al, [esi]
                ;// FORK PLUS
                mov ah, '+'
                cmp ah, al
                JE plus

                ;// Fork subtraction
                mov ah, '-'
                cmp ah, al
                JE subtraction

                ;// Fork multiplication
                mov ah, '*'
                cmp ah, al
                JE multiplication

                ;// Fork multiplication
                mov ah, '/'
                cmp ah, al
                JE division

                mov ecx, ERROR_MESSAGE
                jmp Exit

                ;// CALCULATE START\\
                mov eax, 0
                mov ecx, 0



                plus:
                        mov ecx, [A]
                        add ecx, [B]
                        jmp SuccessMessage
                subtraction:
                        mov ecx, [A]
                        sub ecx, [B]
                        jmp SuccessMessage
                multiplication:
                        mov ecx, [A]
                        imul ecx, [B]
                        jmp SuccessMessage
                division:
                        mov eax, [A]
                        mov ecx, [B]
                        mov edx, 0

                        div ecx
                        mov ecx, eax

                        jmp SuccessMessage

                ;// CALCULATE  END\\
                SuccessMessage:
                         push ecx
                         push O
                         push formatEnd
                         call [printf]

                Exit:
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