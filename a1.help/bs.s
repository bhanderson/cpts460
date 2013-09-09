!============================ mys.s file ====================================
        BOOTSEG =  0x9000        ! Boot block is loaded here.
        SSP     =    4096        ! Stack pointer at 4KB re to SS
       .globl _main, _myprints   ! IMPORT 
       .globl _getc, _putc       ! EXPORT

        !----------------------------------------------------------------
        ! Only one SECTOR loaded at (0000:7C00). We shall get entire BLOCK in
        !----------------------------------------------------------------
        mov  ax,#BOOTSEG    ! set ES to 0x9000
        mov  es,ax

! call BIOS to load BLOCK 0 of FD0 to (segment, offset)=(0x9000, 0)
        xor  dx,dx          ! dh=head=0,    dl=drive=0
        xor  cx,cx          ! ch=cyl=0,     cl=sector=0
        incb cl             ! sector=1 (stupid BIOS counts sector from 1)
        xor  bx,bx          ! (ES,BX)= core address = (0x9000, 0)
        mov  ax, #0x0202    ! ah=2= READ    al=number of sectors 
        int  0x13           ! call BIOS disk I/O function  

! jump indirect to (0x9000, start) to continue execution there
        jmpi start,BOOTSEG  ! CS=BOOTSEG, IP=start

start:
        mov  ax,cs          ! establish segments 
        mov  ds,ax          ! we know ES,CS=0x8000. Let DS=CS  
        mov  ss,ax          ! SS = CS ===> all point at 0x9000
        mov  sp,#SSP        ! SP = SS + 4KB

! these are optional before calling main() in C
        mov  ax, #0x0012    ! color mode
        int  0x10                    
        mov  ax, #0x0200    ! home cursor        
        xor  bx, bx
        xor  dx, dx
        int  0x10
        
! call main() in C
        call _main          ! call main() in C

        mov   ax,#mes
        push  ax
        call  _myprints
        pop   ax

dead:   jmp   dead          ! fall into an infinite loop

mes:    .asciz "BACK TO ASSEMBLY AGAIN\n\r"

        !---------------------------------------------
        !  char getc()   function: returns a char
        !---------------------------------------------
_getc:
        xorb   ah,ah           ! clear ah
        int    0x16            ! call BIOS to get a char in AX
        ret 

        !----------------------------------------------
        ! void putc(char c)  function: print a char
        !----------------------------------------------
_putc:           
        push   bp
        mov    bp,sp

        movb   al,4[bp]        ! get the char into aL
        movb   ah,#14   
        mov    bx,#0x000B      ! BL=0B => cyan    
        int    0x10            ! call BIOS to display the char

        pop    bp
        ret
!===========================================================================
