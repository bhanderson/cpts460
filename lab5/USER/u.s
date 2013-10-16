	.globl _syscall,_main,_exit
        .globl _getcs

         call _main


	
! if ever return, exit(0)
	push  #0
        call  _exit

_syscall:
        int    80
        ret
_getcs:
	mov   ax,cs
	ret
	
