format PE Console
entry start
                    
include '../../INCLUDE/win32a.inc'

; |==================================================|
; |Выпонил: Романюк Андрей                           |
; |Группа: 194                                       |
; |Номер в списке и вариант: 21                      |
; |                                                  |
; |Задача:                                           |
; |Разработать программу, вычисляющую с              |
; |помощью степенного ряда с точностью не            |
; |хуже 0,05% значение функции arccos(х)             |
; |для заданного параметра x (использовать FPU)      |
; |                                                  |
; |Пояснительная записка пресдтавлена в файле ПЗ.docx|
; |==================================================|

section '.data' data readable writeable
x1      dq ? ; Введённое пользователем значение:
eps1    dd 0.0005 ; Точность 0.05%
; Константы:
c2      dq 2.0
c4      dq 4.0
msg1    db 'Enter x (|x|<=1): ',0
msg2    db 'Wrong number.',13,10,0
fmt1    db '%lf',0
msg3    db 'Teylor row = %lg',13,10,0
msg4    db 'Calculated arccos = %lg',13,10,0
buf     db 256 dup(0)

section '.code' code readable executable
start:
        ccall [printf],msg1                 ; Выводим сообщение в консоль
        ccall [gets],buf                    ; Вводим с консоли значение
        ccall [sscanf],buf,fmt1,x1      ; Парсим введённую строчку в число
        
        ; Если преобразование удалось, то продолжить:
        cmp eax,1               
        jz m1                   

        
        ; Иначе: выводим сообщение об ошибке и начинаем заново.
        ccall [printf],msg2
        jmp start

m1:     fld [x1]                  ; Введенное значение
        fabs                      ; Модуль введенного значения
        fld1                      ; 1
        fcompp                    ; Сравниваем 1 с модулем введенного числа
        fstsw   ax                ; Записать флаги сопроцессора в ах
        sahf                      ; Переносим их в флаги процессора
        jb start                  ; 1<x, начать заново
        fld [eps1]                ; Точность вычисления
        
        sub esp, 8                ; Выделяем в стеке место под double
        fstp qword [esp]          ; Записать в стек double число     
        fld qword [x1]            ; Введенное значение
        sub esp, 8                ; Выделить в стеке место под double
        fstp qword [esp]          ; Записать в стек double число     
        call myarccos             ; Вычислить myarccos(x,eps)
        add esp, 16               ; Удалить переданные параметры

        sub esp, 8                ; Передать сумму ряда
        fstp qword [esp]          ; Функции через стек
        push msg3                 ; Формат сообщения
        call [printf]             ; Сформировать результат
        add esp, 12               ; Коррекция стека


        fld qword [x1]            ; Введенное значение
        sub esp, 8                ; Выделить в стеке место под double
        fstp qword [esp]          ; Записать в стек double число     
        call arccos               ; Вычислить arccos(x,eps)
        add esp, 16               ; Удалить переданные параметры

        sub esp, 8                ; Передать точное значение arccos
        fstp qword [esp]          ;Функции через стек
        push msg4                 ; Формат сообщения
        call [printf]             ; Сформировать результат
        add esp, 12               ; Коррекция стека

        ccall [_getch]            ; Ожидание нажатия любой клавиши
        
ex:     stdcall [ExitProcess], 0  ; Выход


; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Функция для вычисления arccos(x) с точность eps. 
; Соглашение вызова через cdecl
; Вход: double x, double eps
; Вывод: double
myarccos:
        push ebp                ; Создать кадр стека
        mov ebp,esp
        sub esp,0ch             ; Создание локальных переменных
;Локальные переменные: 
t       equ ebp-0ch             ; Временная переменная
a       equ ebp-8h              ; Очередное слагаемое ряда

;Переданные функции параметры:
x       equ ebp+8h              
eps     equ ebp+10h

;Вычисленное значение 
        fld qword [x]           ;Загрузить х
        fstp qword [a]          ;a = x
        fldpi                   ;pi
        fdiv [c2]               ;pi/2
        fldz                    ;s=0
        mov ecx,0               ;//n=0
m11:    fadd qword [a]          ;s += a;
        inc ecx                 ;n++;
        fld qword [a]           ;a
        fmul qword [x]          ;a*x
        fmul qword [x]          ;a*x*x
        lea eax,[2*ecx-1]       ;2n-1
        mov [t],eax             ;t=2n-1
        fimul dword [t]         ;a*x*x*(2n-1)
        lea eax,[2*ecx]         ;2n
        mov [t],eax             ;t=2n
        fimul dword [t]         ;a*x*x*(2n-1)*2n
        fdiv [c4]               ;a*x*x*(2n-1)*2n/2
        mov [t],ecx             ;n
        fidiv dword [t] 
        fidiv dword [t]         ;a*x*x*(2n-1)*2n/2/(n*n)
        lea edx,[ecx-1]         ;n-1
        lea eax,[2*edx+1]       ;(2 * (n - 1) + 1)
        mov [t],eax             ;t=(2 * (n - 1) + 1)
        fimul dword [t]         ;a*x*x*(2n-1)*2n/2/(n*n)*(2 * (n - 1) + 1)
        lea eax,[2*ecx+1]       ;(2 * n + 1)
        mov [t],eax             ;t=(2 * n + 1)
        fidiv dword [t]         ;a*x*x*(2n-1)*2n/2/(n*n)*(2 * (n - 1) + 1)/(2 * n + 1)
        fst qword [a]           ;a = a*x*x*(2 * n - 1)*(2 * n) / 4.0 / (n*n)*(2 * (n - 1) + 1) / (2 * n + 1);
        fabs                    ;|a|
        fcomp qword [eps]       ; сравнить |a| c eps
        fstsw ax;               ; перенести флаги сравнения в ах
        sahf;                   ; занести ah в флаги процессора
        jnb m11;                ; Если |a|>=e, продолжить цикл
        fsubp st1,st            ;pi/2-полученная сумма
        leave              
        ret

; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; Функция для точного вычисления arccos(double x)
; Соглашение вызова через cdecl
; Вход: double x
; Вывод: double
arccos:
        push ebp                 ; Создать кадр стека
        mov ebp,esp
        fldpi                    ;pi
        fdiv [c2]                ;pi/2
        fld qword [ebp+8];x
        fld1                     ;1
        fld qword [ebp+8];x
        fmul st,st               ; x^2
        fsubp st1,st             ; 1-x^2
        fsqrt                    ; sqrt(1-x^2)
        fpatan                   ; arctg(x/sqrt(1-x^2))
        fsubp st1,st             ; pi/2-arctg(x/sqrt(1-x^2))
        pop ebp                  ; Эпилог функции
        ret
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

section '.idata' import data readable

library kernel,'kernel32.dll',\
        user,'user32.dll',\
        msvcrt,'msvcrt.dll'

import  kernel,\
        ExitProcess,'ExitProcess'

import  msvcrt,\
        sscanf,'sscanf',\
        gets,'gets',\
        _getch,'_getch',\
        printf,'printf'