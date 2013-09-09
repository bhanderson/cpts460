!======================= bs.s file  ==============================
        BOOTSEG =  0x9000        ! Boot block is loaded here.
        SSP     =    8192        ! 8 KB stack
        NTXSEG  =  0x1000        ! OS is loaded here
			
.globl _main, _prints                                 ! IMPORT 
.globl _getc,_putc,_readfd,_setes,_inces,_error       ! EXPORT

        !----------------------------------------------------------------
        ! Only one SECTOR loaded at (0000:7C00). load entire BLOCK to 0x9E00
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
        jmpi    start, BOOTSEG           ! CS=BOOTSEG, IP=start

start:
        mov     ax,cs                   ! establish segments 
        mov     ds,ax                   ! we know ES,CS=0x9E00. Let DS=CS  
        mov     ss,ax                   ! SS = CS ===> all point at 0x9E00
        mov     sp,#SSP                 ! SP = SS + 8KB

	mov    ax,#0x0012               ! set color mode
	int    0x10
	
        call _main                      ! call main() in C

        jmpi  0, 0x1000

!======================== I/O functions =================================
		
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
        movb   ah,#14          ! aH = 14
        movb   bl,#0x0B        ! bL = cyan color 
        int    0x10            ! call BIOS to display the char

        pop    bp
	ret
        
       !---------------------------------------
       ! readfd(cyl, head, sector, buf)
       !        4     6     8      10
       !---------------------------------------
_readfd:                             
        push  bp
	mov   bp,sp            ! bp = stack frame pointer

        movb  dl, #0x00        ! drive 0=FD0
        movb  dh, 6[bp]        ! head
        movb  cl, 8[bp]        ! sector, count from 0
        incb  cl               ! BIOS sector : count from 1
        movb  ch, 4[bp]        ! cyl
        mov   bx, 10[bp]       ! BX=buf ==> memory addr=(ES,BX)
        mov   ax, #0x0202      ! READ 2 sectors to (EX, BX)

        int  0x13              ! call BIOS to read the block 
        jb   _error            ! to error if CarryBit is on [read failed]

        pop  bp                
	ret

        
_setes:  push  bp
	 mov   bp,sp

         mov   ax,4[bp]        
         mov   es,ax

	 pop   bp
	 ret
	
_inces:                         ! inces() inc ES by 0x40, or 1K
         mov   ax,es
         add   ax,#0x40         ! inc ES by 1KB
         mov   es,ax
         ret

        !------------------------------
        !       error & reboot
        !------------------------------
_error:
        mov  bx, #bad
        push bx
        call _prints
call _getc
        int  0x19               ! reboot

bad:    .asciz  "Error!\n\r"
