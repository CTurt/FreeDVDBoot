// ElReino & CTurt 2020

//int (*SifIopReset)(char *, int) = (void *)0x85360;
//void (*SifInitRpc)(int) = (void *)0x84500;
//void (*SifExitRpc)(void) = (void *)0x84690;

extern void SifWriteBackDCache(void *ptr, int size);
extern int SifSetReg(unsigned int register_num, unsigned int register_value);
extern int SifGetReg(unsigned int register_num);

static int SifIopSync(void) {
	#define SIF_REG_SMFLAG 4
	#define SIF_STAT_BOOTEND 0x40000
	return((SifGetReg(SIF_REG_SMFLAG) & SIF_STAT_BOOTEND) != 0);
}

static void flush(void) {
	asm volatile("la $v1, 0x64; la $a0, 0; syscall 0x64"); // FlushCache data writeback
	asm volatile("la $v1, 0x64; la $a0, 2; syscall 0x64"); // FlushCache instruction invalidate
}

 int GetThreadId(void);
void ChangeThreadPriority(int thread_id, int priority);
int CancelWakeupThread(int thread_id);
void TerminateThread(int thread_id);
void DeleteThread(int thread_id);

static void TerminateAllThreads(void) {
    int i, ThreadID;

    ThreadID=GetThreadId();
    ChangeThreadPriority(ThreadID, 0);
    CancelWakeupThread(ThreadID);
    for(i=1; i<256; i++){    //Skip idle thread.
        if(i!=ThreadID){
            TerminateThread(i);
            DeleteThread(i);
        }
    }
}

int main(void) {
	// ExecPS2 is broken on Phat PS2... manually kill other threads instead
	TerminateAllThreads();

	// Signal IOP that EE is Ready, willing, and fully enabled!
	SifSetReg(3, 1);

	volatile int *waitAddress = (void *)0x21FFF7F0;
	while(!*waitAddress);

	volatile void **entry_point_address = (void *)0x01FFF7E0;

	// cdrom0:
	volatile void **argument = (void *)0x01FFF7D0;
	*(volatile int *)0x01FFF7D0 = 0x01FFF7D8;
	*(volatile int *)0x01FFF7D8 = 0x6F726463;
	*(volatile int *)0x01FFF7DC = 0x003A306D;

	flush();
	
	//SifIopReset("rom0:UDNL rom0:EELOADCNF", 0);
	//while(!SifIopSync());

	//ExecPS2(*entry_point_address, 0, 0, 0);
	ExecPS2(*entry_point_address, 0, 1, argument); // kHn: arg == cdrom0:
}
