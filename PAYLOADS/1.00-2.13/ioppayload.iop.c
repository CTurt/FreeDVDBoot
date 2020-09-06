// ElReino & CTurt

/* Todo: seperate these settings to an include file.
 */
#define EE_CRT0_ADDRESS ((void*)0x21FFF800)
#define EE_WAIT_ADDRESS ((void*)0x01FFF7F0)
#define EE_ENTRYPOINT_ADDRESS ((void *)0x01FFF7E0)
//#define EE_DEBUG_ADDRESS ((void *)0x01FFF7D0)

struct SifDmaTransfer {
   void				*src,
      				*dest;
   int				size;
   int				attr;
} __attribute__ ((aligned(8)));

#define ELF_PT_LOAD 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int size_t;

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

#define SECTOR_SIZE 0x800

#define min(a, b) (((a) < (b)) ? (a) : (b))

int (*readSectors)(int count, int sector, void *destination);

//int (*sceSifSetDma)(struct SifDmaTransfer *, int num) = (void *)0x16fc8;
//int (*sceSifDmaStat)(int trid) = (void *)0x17170;
//void (*flushIcache)(void) = (void*)0x2f40;
//void (*flushDcache)(void) = (void*)0x3044;
//void (*printf)(char *, ...) = (void *)0x1ab84; // 2.10
//void (*printf)(char *, ...) = (void *)0x155f8; // 2.12

int (*sceSifSetDma)(struct SifDmaTransfer *, int num);
int (*sceSifDmaStat)(int trid);

static void transfer_to_ee(void *dest, void *src, unsigned int size);
static void *memcpy(void *dest, void *src, unsigned int n);
static void *memset(void *s, int c, unsigned int n);
static void memset_ee(void *s, int c, unsigned int n);

//#include "iopresolve.h"

#define BD2 (*(volatile int *)0xBD000020) //msflag

static void readData(void *dest, unsigned int offset, size_t n) {
	//unsigned char buffer[SECTOR_SIZE];
	//unsigned char *buffer = (void *)0xfd000;
	unsigned char *buffer = (void *)0xba000; // single

	unsigned int copied = 0;
	#define remaining (n - copied)

	if(offset % SECTOR_SIZE) {
		readSectors(1, offset / SECTOR_SIZE, buffer);
		memcpy(dest, buffer + offset % SECTOR_SIZE, min(SECTOR_SIZE - (offset % SECTOR_SIZE), n));
		copied += min(SECTOR_SIZE - (offset % SECTOR_SIZE), n);
	}

	if(remaining >= SECTOR_SIZE) {
		readSectors(remaining / SECTOR_SIZE, (offset + copied) / SECTOR_SIZE, dest + copied);
		copied += (remaining / SECTOR_SIZE) * SECTOR_SIZE;
	}
	
	if(remaining > 0) {
		readSectors(1, (offset + copied) / SECTOR_SIZE, buffer);
		memcpy(dest + copied, buffer, remaining);
	}

	#undef remaining
}

// Read data but don't care about over/under writing to dest
static void readDataUnsafe(void *dest, unsigned int offset, size_t n) {
	unsigned int sectorAlignedOffset = offset & ~(SECTOR_SIZE - 1);
	unsigned int underflow = offset - sectorAlignedOffset;

	readSectors((n + underflow + SECTOR_SIZE - 1) / SECTOR_SIZE, sectorAlignedOffset / SECTOR_SIZE, dest - underflow);
}

void _start(void) {
	extern unsigned char ee_crt0[];
	extern unsigned int ee_crt0_size;
	void *return_address[4] __attribute__ ((aligned (16))) = { EE_CRT0_ADDRESS, 0, 0, 0 };
	int one __attribute__ ((aligned (16))) = 1;
	int i;

	//sceSifSetDma = resolve("sifman", 7);
	//sceSifDmaStat = resolve("sifman", 8);

	sceSifSetDma = (void *)0x16fc8;
	sceSifDmaStat = (void *)0x17170;

	unsigned int addiu_magic = 0x27bdffc8; // addiu $sp, $sp, -0x38
	//if(*(unsigned int *)READ_SECTORS_110 == addiu_magic) readSectors = (void *)READ_SECTORS_110;
	 if(*(unsigned int *)READ_SECTORS_210 == addiu_magic) readSectors = (void *)READ_SECTORS_210;
	else if(*(unsigned int *)READ_SECTORS_212 == addiu_magic) readSectors = (void *)READ_SECTORS_212;
	else if(*(unsigned int *)READ_SECTORS_213 == addiu_magic) readSectors = (void *)READ_SECTORS_213;

	transfer_to_ee(EE_CRT0_ADDRESS, ee_crt0, ee_crt0_size);
	
	// Corrupt all known return addresses in the stack, there might be a more universal way for IOP to redirect EE...
	transfer_to_ee((void *)0x14A5FF0, &return_address, sizeof(return_address)); // 2.10E/A
	transfer_to_ee((void *)0x10007F0, &return_address, sizeof(return_address)); // 2.10J
	transfer_to_ee((void *)0x12D1C70, &return_address, sizeof(return_address)); // 2.10U
	
	transfer_to_ee((void *)0x12B8CF0, &return_address, sizeof(return_address)); // 2.12U
	transfer_to_ee((void *)0x148D0F0, &return_address, sizeof(return_address)); // 2.12G
	transfer_to_ee((void *)0xFE5FF0, &return_address, sizeof(return_address)); // 2.12J

	transfer_to_ee((void *)0x01477B80, &return_address, sizeof(return_address)); // 2.13E/A

	// Clear bit 0 of 0x208bb710 to make EE exit loop waiting for IOP, and return to our above payload
	unsigned int loopValue = 0x010004;
	//transfer_to_ee((void *)0x208bb710, &loopValue, sizeof(loopValue)); // 2.10E
	transfer_to_ee((void *)0x2087d110, &loopValue, sizeof(loopValue)); // 2.13E

	// We wait for EE side to be ready before sending ELF.
	while(!(SifGetMSFlag() & 1));
	SifSetMSFlag(3);
	
	//unsigned char *buffer = (void *)0xfe000;
	unsigned char *buffer = (void *)0xBB800;
	size_t sizeofbuffer = 2 * SECTOR_SIZE; // todo: find a nice large space 4 sectors maybe

	elf_header_t eh;
	readData(&eh, LOAD_ELF_FROM_OFFSET, sizeof(elf_header_t));

	elf_pheader_t eph[eh.phnum];
	readData(&eph, LOAD_ELF_FROM_OFFSET + eh.phoff, sizeof(elf_pheader_t) * eh.phnum);
	
	for (i = 0; i < eh.phnum; i++) {
		if (eph[i].type != ELF_PT_LOAD)
			continue;

		// TODO: handle non-16byte aligned transfers

		unsigned int copied = 0;
		int remaining = eph[i].filesz;
		while(remaining > 0) {
			unsigned int k = min(remaining, sizeofbuffer);
			k = (k + 0xf) & ~0xf;

			// If offset is not aligned to a sector, start with a smaller transfer to get it aligned for future reads
			if((eph[i].offset + copied) & (SECTOR_SIZE - 1)) k = SECTOR_SIZE - (eph[i].offset + copied) & (SECTOR_SIZE - 1);
			
			//readData(buffer, LOAD_ELF_FROM_OFFSET + eph[i].offset + copied, k);
			readDataUnsafe(buffer, LOAD_ELF_FROM_OFFSET + eph[i].offset + copied, k);

			transfer_to_ee(eph[i].vaddr + copied, buffer, k);
			copied += k;
			remaining -= k;
		}

		copied = 0;
		remaining = eph[i].memsz - eph[i].filesz;

		if(remaining > 0) {
			// First transfer needs to respect if load size isn't multiple of 16 bytes and not memset 0 over the final eph[i].filesz % 16 bytes
			if(eph[i].filesz % 16) {
				readData(buffer, LOAD_ELF_FROM_OFFSET + eph[i].offset + eph[i].filesz - (eph[i].filesz % 16), eph[i].filesz % 16);
				memset(buffer + (eph[i].filesz % 16), 0, 16 - (eph[i].filesz % 16));
				transfer_to_ee(eph[i].vaddr + eph[i].filesz - (eph[i].filesz % 16), buffer, 16);

				copied += 16 - (eph[i].filesz % 16);
				remaining -= 16 - (eph[i].filesz % 16);
			}

			memset(buffer, 0, sizeofbuffer);
		}
		while(remaining > 0) {
			unsigned int k = min(remaining, sizeofbuffer);
			k = (k + 0xf) & ~0xf;
			
			transfer_to_ee(eph[i].vaddr + eph[i].filesz + copied, buffer, k);
			copied += k;
			remaining -= k;
		}
	}
	
	transfer_to_ee(EE_ENTRYPOINT_ADDRESS, &eh.entry, sizeof(one));
	
	// Signal EE that the ELF is loaded and ready to execute.
	transfer_to_ee(EE_WAIT_ADDRESS, &one, sizeof(one));

	//int loopValueJ = 0;
	//transfer_to_ee((void *)0x205ea210, &loopValueJ, sizeof(loopValueJ)); // 2.10J
}

/* dest and src should be aligned to 16 byte boundary
 */
static void transfer_to_ee(void *dest, void *src, unsigned int size)
{
	int trid;
	
	size = size & 0x3FFFFFFF;
	struct SifDmaTransfer t = { src, dest, size, 0 };
	
	/* These could be sent in parallel, but is it really worth it?
	 */
	trid = sceSifSetDma(&t, 1);
	while(sceSifDmaStat(trid) > -1){};
}

static void *memcpy(void *dest, void *src, unsigned int n)
{ 
	int i;
	
	for(i = 0; i < n; i++)
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
	
	return dest;
}

static void *memset(void *s, int c, unsigned int n)
{
	int i;
	
	for(i = 0; i < n; i++)
		((unsigned char *)s)[i] = c;
	
	return s;
}

static int SifGetMSFlag()
{
	int a, b;
	
	b = BD2;
	do {
		a=b;
		b=BD2;
	} while(a != b);
	
	return a;
}

static int SifSetMSFlag(unsigned int value)
{
	int a, b;
	
	BD2 = value;
	
	b = BD2;
	do {
		a=b;
		b=BD2;
	} while(a != b);
	
	return a;
}

asm("\n\
.global ee_crt0\n\
ee_crt0:\n\
.align 8\n\
.incbin \"eepayload.ee.bin\"\n\
ee_crt0_size: .word . - ee_crt0\n\
");
