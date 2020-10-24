format PE Console
entry start
                    
include '../../INCLUDE/win32a.inc'

; |==================================================|
; |�������: ������� ������                           |
; |������: 194                                       |
; |����� � ������ � �������: 21                      |
; |                                                  |
; |������:                                           |
; |����������� ���������, ����������� �              |
; |������� ���������� ���� � ��������� ��            |
; |���� 0,05% �������� ������� arccos(�)             |
; |��� ��������� ��������� x (������������ FPU)      |
; |                                                  |
; |������������� ������� ������������ � ����� ��.docx|
; |==================================================|

section '.data' data readable writeable
x1      dq ? ; �������� ������������� ��������:
eps1    dd 0.0005 ; �������� 0.05%
; ���������:
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
        ccall [printf],msg1                 ; ������� ��������� � �������
        ccall [gets],buf                    ; ������ � ������� ��������
        ccall [sscanf],buf,fmt1,x1      ; ������ �������� ������� � �����
        
        ; ���� �������������� �������, �� ����������:
        cmp eax,1               
        jz m1                   

        
        ; �����: ������� ��������� �� ������ � �������� ������.
        ccall [printf],msg2
        jmp start

m1:     fld [x1]                  ; ��������� ��������
        fabs                      ; ������ ���������� ��������
        fld1                      ; 1
        fcompp                    ; ���������� 1 � ������� ���������� �����
        fstsw   ax                ; �������� ����� ������������ � ��
        sahf                      ; ��������� �� � ����� ����������
        jb start                  ; 1<x, ������ ������
        fld [eps1]                ; �������� ����������
        
        sub esp, 8                ; �������� � ����� ����� ��� double
        fstp qword [esp]          ; �������� � ���� double �����     
        fld qword [x1]            ; ��������� ��������
        sub esp, 8                ; �������� � ����� ����� ��� double
        fstp qword [esp]          ; �������� � ���� double �����     
        call myarccos             ; ��������� myarccos(x,eps)
        add esp, 16               ; ������� ���������� ���������

        sub esp, 8                ; �������� ����� ����
        fstp qword [esp]          ; ������� ����� ����
        push msg3                 ; ������ ���������
        call [printf]             ; ������������ ���������
        add esp, 12               ; ��������� �����


        fld qword [x1]            ; ��������� ��������
        sub esp, 8                ; �������� � ����� ����� ��� double
        fstp qword [esp]          ; �������� � ���� double �����     
        call arccos               ; ��������� arccos(x,eps)
        add esp, 16               ; ������� ���������� ���������

        sub esp, 8                ; �������� ������ �������� arccos
        fstp qword [esp]          ;������� ����� ����
        push msg4                 ; ������ ���������
        call [printf]             ; ������������ ���������
        add esp, 12               ; ��������� �����

        ccall [_getch]            ; �������� ������� ����� �������
        
ex:     stdcall [ExitProcess], 0  ; �����


; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; ������� ��� ���������� arccos(x) � �������� eps. 
; ���������� ������ ����� cdecl
; ����: double x, double eps
; �����: double
myarccos:
        push ebp                ; ������� ���� �����
        mov ebp,esp
        sub esp,0ch             ; �������� ��������� ����������
;��������� ����������: 
t       equ ebp-0ch             ; ��������� ����������
a       equ ebp-8h              ; ��������� ��������� ����

;���������� ������� ���������:
x       equ ebp+8h              
eps     equ ebp+10h

;����������� �������� 
        fld qword [x]           ;��������� �
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
        fcomp qword [eps]       ; �������� |a| c eps
        fstsw ax;               ; ��������� ����� ��������� � ��
        sahf;                   ; ������� ah � ����� ����������
        jnb m11;                ; ���� |a|>=e, ���������� ����
        fsubp st1,st            ;pi/2-���������� �����
        leave              
        ret

; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
; ������� ��� ������� ���������� arccos(double x)
; ���������� ������ ����� cdecl
; ����: double x
; �����: double
arccos:
        push ebp                 ; ������� ���� �����
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
        pop ebp                  ; ������ �������
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