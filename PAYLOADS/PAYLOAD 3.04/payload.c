#include <stddef.h>

// Pick one
#define LOAD_FROM_VTS_02_0_IFO
//#define LOAD_FROM_SECTOR_RELATIVE_TO_VIDEO_TS_IFO (151 - 138 - 7)

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define pointToIFO ((void (*)(unsigned int index, unsigned int lba, unsigned int offset))0x23dfc8)
#define getDiscData ((void (*)(unsigned int s, void *d))0x23e138)

#define SifIopReset ((int (*)(char *, int))0x291358)
#define SifIopSync ((int (*)(void))0x2914d8)
#define SifInitRpc ((void (*)(int))0x208260)
#define SifExitRpc ((void (*)(void))0x208400)

#define getBufferInternal ((int (*)(void *filename, int type, int currentSector, void *dest, unsigned int sectorsRemaining, int curReadPos))0x261548)

#define ELF_PT_LOAD 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct {
	u8 ident[16];
	u16 type;
	u16 machine;
	u32 version;
	u32 entry;
	u32 phoff;
	u32 shoff;
	u32 flags;
	u16 ehsize;
	u16 phentsize;
	u16 phnum;
	u16 shentsize;
	u16 shnum;
	u16 shstrndx;
} elf_header_t;

typedef struct {
	u32 type;
	u32 offset;
	void *vaddr;
	u32 paddr;
	u32 filesz;
	u32 memsz;
	u32 flags;
	u32 align;
} elf_pheader_t;

__attribute__((noreturn)) void ExecPS2(void *entry, void *gp, int argc, char **argv) {
	asm volatile("la $v1, 7; syscall 7");
	//__builtin_unreachable();
}

void *memcpy_(void *dest, void *src, size_t n) { 
   int i;
   for(i = 0; i < n; i++) ((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
   return dest;
}

void *memset(void *dest, int c, size_t n) {
	int i;
	for(i = 0; i < n; i++) ((unsigned char *)dest)[i] = c;
	return dest;
}

static void readData(void *dest, unsigned int offset, size_t n) {
	unsigned char buffer[0x800];

	unsigned int copied = 0;
	#define remaining (n - copied)

	if(offset % 0x800) {
		getBufferInternal("", 1, offset / 0x800, buffer, 1, 0);
		memcpy_(dest, buffer + offset % 0x800, min(0x800 - (offset % 0x800), n));
		copied += min(0x800 - (offset % 0x800), n);
	}

	if(remaining >= 0x800) {
		getBufferInternal("", 1, (offset + copied) / 0x800, dest + copied, remaining / 0x800, 0);
		copied += (remaining / 0x800) * 0x800;
	}
	
	if(remaining > 0) {
		getBufferInternal("", 1, (offset + copied) / 0x800, buffer, 1, 0);
		memcpy_(dest + copied, buffer, remaining);
	}
}

__attribute__((noreturn)) void _start(void) {
	//Exit(0);
	//asm volatile("la $v1, 0x04; la $a0, 0; syscall 0x04");

	int i;

	#ifdef LOAD_FROM_VTS_02_0_IFO
		// point to VTS_02_0.IFO
		pointToIFO(2, 0, 0);
		
		// Force a read from VTS_02_0.IFO
		char head[64];
		getDiscData(64, &head);

		#define RELATIVE_SECTOR 0
	#else
		#define RELATIVE_SECTOR LOAD_FROM_SECTOR_RELATIVE_TO_VIDEO_TS_IFO
	#endif

	// Based on https://github.com/AKuHAK/uLaunchELF/blob/master/loader/loader.c
	elf_header_t eh;
	readData(&eh, RELATIVE_SECTOR * 0x800, sizeof(elf_header_t));

	elf_pheader_t eph[eh.phnum];
	readData(&eph, RELATIVE_SECTOR * 0x800 + eh.phoff, sizeof(elf_pheader_t) * eh.phnum);
	
	for (i = 0; i < eh.phnum; i++) {
		if (eph[i].type != ELF_PT_LOAD)
			continue;

		readData(eph[i].vaddr, RELATIVE_SECTOR * 0x800 + eph[i].offset, eph[i].filesz);
		if(eph[i].memsz > eph[i].filesz) memset(eph[i].vaddr + eph[i].filesz, 0, eph[i].memsz - eph[i].filesz);
	}

	asm volatile("la $v1, 0x64; la $a0, 0; syscall 0x64"); // FlushCache data writeback
	asm volatile("la $v1, 0x64; la $a0, 2; syscall 0x64"); // FlushCache instruction invalidate

	//while(!SifIopReset("", 0));
	//while(!SifIopSync());

	//while(!SifIopReset("rom0:UDNL rom0:EELOADCNF", 0));
	SifIopReset("rom0:UDNL rom0:EELOADCNF", 0);
	while(!SifIopSync());

	SifInitRpc(0);
	SifExitRpc();

	ExecPS2((void *)eh.entry, 0, 0, 0);
}
