format PE console
entry start


include '..\..\INCLUDE\win32a.inc'

section '.data' data readable writable

        formatName db '%s', 0
        formatAge db '%d', 0

        name rd 2
        age rd 1

        firstQuestion db 'What is your name? My name is: ', 0
        secondQuestion db 'How old are you? I am: ', 0
        hello db 'Hello, %s. Your age is %d.',0

        NULL = 0

section '.code' code readable executable

        start:
                push firstQuestion
                call [printf]

                push name
                push formatName
                call [scanf]

                push secondQuestion
                call [printf]

                push age
                push formatAge
                call [scanf]

                push [age]
                push name
                push hello
                call [printf]

                call [getch]

                push NULL
                call [ExitProcess]

section '.idata' import data readable

        library kernel, 'kernel32.dll',\
                msvcrt, 'msvcrt.dll'

        import msvcrt,\
               printf, 'printf',\
               scanf, 'scanf',\
               getch, '_getch'

         import kernel,\
               ExitProcess, 'ExitProcess'