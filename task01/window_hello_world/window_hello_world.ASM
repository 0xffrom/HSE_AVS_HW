format PE GUI 4.0


; Romanyuk Andrey BSE-194
; HOMEWORK #1
; ==== Window Hello World ====

include '..\..\INCLUDE\win32ax.inc'


section '.code' code readable executable
  start:
        invoke    MessageBox,HWND_DESKTOP,"Hello, HSE!",invoke GetCommandLine,MB_OK
        invoke    ExitProcess,0
.end start