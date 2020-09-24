format PE console
entry start

; ============================================
; Name: Andrew Romanyuk
; Group: 194
; Number in list: 21
; My option: 1
; Desctiption: input array A with keyboard
;              and select only positive numbers
;              and put it in array B
;
; Examples and more information on README.md
; =============================================

include '../../INCLUDE/win32a.inc'
include '../../INCLUDE/API/user32.inc'
include '../../INCLUDE/API/kernel32.inc'

section '.data' data readable writable

        strInputN       db       'Input count of elements in array ([%d;%d]): ', 0
        strArrayA       db       'Array A: ', 0
        strArrayB       db       'Array B: ', 0
        strEnterStart   db       'Enter the %d element: ', 0
        strPositive     db       'Count of positive numbers equals %d ', 10, 13, 0
        strInput        db       'Invalid input. Try again :(', 10, 13, 0
        strPositiveNum  db       7, '(%d is a positive number)', 10, 13, 0
        digit           db       '%d', 0
        endl            db       10, 13, 0
        strOutStart     db       '[ ', 0
        strOutEnd       db       ' ]', 10, 13, 0
        strOutMiddle    db       ', ', 0

;        Heap with arrays:
        _heap           dd      ?
        arrayA          dd      ?
;       Array only with positive numbers:
        arrayB          dd      ?
;       Temp data:
        tempArray       dd      ?
        tempDigit       dd      ?
        count           dd      ?

;       Iterator
        i               dd      ?
;       Count of input array (A)
        N               dd      ?
;       Count of output array (B) with rule (only positive numbers)
        M               dd      0
;       Min and max values of input array's length
        minValue        dd      1
        maxValue        dd      100
        NULL = 0
                
section '.code' code readable executable
        start:
                call InputData

;               Create heap:
                invoke HeapCreate, HEAP_NO_SERIALIZE, 0, 0
                mov [_heap], eax

;               Allocating space in the heap for the array:
                invoke HeapAlloc,[_heap], HEAP_NO_SERIALIZE, N*8
                mov [arrayA], eax

                call FillArrayA
                call PrintArrayA

                invoke HeapAlloc,[_heap], HEAP_NO_SERIALIZE, M*8
                mov [arrayB], eax

                cinvoke printf, strPositive, [M]

                call FillArrayB
                call PrintArrayB

                call FinishProgram
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Entering the number of values in array A with a check for the correctness of the value
; Input: -
; Output: N or Error
InputData:
         cinvoke printf, strInputN, [minValue], [maxValue]
;        Input N:
         cinvoke scanf, digit, N
         mov ecx, [minValue]
         cmp ecx, [N]
         jg invalidData

         mov ecx, [maxValue]
         cmp ecx, [N]
         jl invalidData
         ret 0

invalidData:
         cinvoke printf, strInput
         jmp FinishProgram


; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to print array B
; Input: arrayB
; Output: void
PrintArrayB:
                cinvoke printf, strArrayB
                mov eax, [arrayB]
                mov [tempArray], eax

                mov ecx, [M]
                mov [count], ecx

                call PrintArray
                ret 0
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to fill array B
; Input: -
; Output: arrayB
FillArrayB:
                mov [count], 0
                mov [i], -1
                jmp loopB
        loopB:
                add [i], 1

                mov eax, [arrayA]
                call GetNumber

                cmp ecx, 0
                jg _fillNum

        loopBEnd:
                mov ecx, [i]
                add ecx, 1
                cmp ecx, [N]
                jne loopB
         ret 0

 _fillNum:
        mov [tempDigit], ecx

        mov ecx, [count]
        imul ecx, dword 4
        mov eax, [arrayB]
        add eax, ecx

        mov ecx, [tempDigit]
        mov [eax], ecx

        inc [count]
        jmp loopBEnd
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to fill array A
; Input: -
; Output: arrayA
FillArrayA:
        mov [i], -1
        loopA:
                add [i], 1

;               Increment counter for print
                mov eax, [i]
                add eax, 1

                cinvoke printf, strEnterStart, eax
                mov ecx, [i]
                imul ecx, dword 4
                mov eax, [arrayA]
                add eax, ecx
                mov ecx, [eax]
                cinvoke scanf, digit, eax
                mov ecx, [i]
                imul ecx, dword 4
                mov eax, [arrayA]
                add eax, ecx
                mov ecx, [eax]

                cmp ecx, 0
                jg incrementM
         loopAEnd:
                mov ecx, [i]
                add ecx, 1
                cmp ecx, [N]
                jne loopA
         ret 0

; Find the number of positive values in array A
incrementM:
        cinvoke printf, strPositiveNum, ecx
        add [M], 1
        jmp loopAEnd
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to print array A
; Input: arrayA
; Output: void
PrintArrayA:
        cinvoke printf, strArrayA
        mov eax, [arrayA]
        mov [tempArray], eax
        mov ecx, [N]
        mov [count], ecx

        call PrintArray
        ret 0
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to get number of array
; Input: i - index, eax - array
; Output: ecx - value, eax - reference
GetNumber:
        mov [tempArray], eax
        mov ecx, [i]
        imul ecx, dword 4
        mov eax, [tempArray]
        add eax, ecx
        mov ecx, [eax]
        ret 0
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to call clear memory and end program
; Input: -
; Output: void
FinishProgram:
        pop     edi
        pop     es
        pop     ebp
        pop     edx
        pop     ecx
        pop     bx

        invoke HeapFree, _heap, 0, arrayA
        invoke HeapFree, _heap, 0, arrayB

        cinvoke getch

        push NULL
        call [ExitProcess]
        ret 0
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Name: Function to call print array
; Description: tempArray - our array, count - size of array
; Input: tempArray, count
; Output: void
PrintArray:
        ; reset counter
        mov [i], -1
        cinvoke printf,strOutStart
        cmp [count], 0
        jle _printerEnd

        _printer:
                add [i], 1

                mov ecx, [i]
                imul ecx, dword 4
                mov eax, [tempArray]
                add eax, ecx

                cinvoke printf, digit, dword [eax]

                mov ecx, [i]
                add ecx, 1

                cmp ecx, [count]
                jne printDot
                jmp loop3cont

                printDot:
                        cinvoke printf,strOutMiddle
                loop3cont:
                        mov ecx, [i]
                        add ecx, 1

                        cmp ecx, [count]
                        jne _printer
        _printerEnd:
                cinvoke printf,strOutEnd
        ret 0
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
section '.idata' import data readable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll',\
            user32,'USER32.DLL'

    import msvcrt,\
           printf, 'printf',\
           scanf, 'scanf',\
           getch, '_getch'

    import kernel,\
           ExitProcess, 'ExitProcess',\
           HeapCreate, 'HeapCreate',\
           HeapFree, 'HeapFree',\
           HeapAlloc,'HeapAlloc'

