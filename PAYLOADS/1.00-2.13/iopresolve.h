//typedef unsigned char u8;
//typedef unsigned short u16;
//typedef unsigned int u32;

typedef void *pointer;

#define NULL 0

typedef struct _smod_mod_info {
	//struct _smod_mod_info *next;
	pointer next;
	/** A pointer to the name in IOP RAM, this must be smem_read().  */
	//char	*name;
	pointer name;
	u16	version;
	/** For MODLOAD shipped with games. The old MODLOAD module from boot ROMs do not use a flags field.  */
	u16	newflags;
	u16	id;
	u16	unused;
	/** _start */
	u32	entry;
	u32	gp;
	u32	text_start;
	u32	text_size;
	u32	data_size;
	u32	bss_size;
	u32	unused1;
	u32	unused2;
} smod_mod_info_t;

typedef struct _slib_imp_list {
	u8	magic;
	//struct _slib_imp_list *next;
	pointer next;
	u16	version;
	u16	flags;
	u8	name[8];
	//void	*imports[0];
	pointer imports[0];
} slib_imp_list_t;

typedef struct _slib_exp_lib {
	//struct _slib_exp_lib *prev;
	pointer prev;
	//struct _slib_imp_list *caller;
	pointer caller;
	u16	version;
	u16	flags;
	u8	name[8];
	//void	*exports[0];
	pointer exports[0];
} slib_exp_lib_t;

typedef struct _slib_exp_lib_list {
	//struct _slib_exp_lib *tail;
	pointer tail;
	//struct _slib_exp_lib *head;
	pointer head;
} slib_exp_lib_list_t;

#define SMEM_BUF_SIZE	0x300	//Must be large enough to accommodate all operations.

struct smem_buf {
	union {
		u8 bytes[SMEM_BUF_SIZE / sizeof(u8)];
		u32 words[SMEM_BUF_SIZE / sizeof(u32)];
		smod_mod_info_t mod_info;
		slib_exp_lib_t exp_lib;
	};
};



size_t strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

int memcmp(const char *cs_in, const char *ct_in, size_t n) {
  size_t i;  
  const unsigned char * cs = (const unsigned char*) cs_in;
  const unsigned char * ct = (const unsigned char*) ct_in;

  for (i = 0; i < n; i++, cs++, ct++)
  {
    if (*cs < *ct)
    {
      return -1;
    }
    else if (*cs > *ct)
    {
      return 1;
    }
  }
  return 0;
} 



slib_exp_lib_list_t _slib_cur_exp_lib_list;

struct smem_buf smem_buf;

typedef unsigned int SifRpcReceiveData_t;

size_t SifRpcGetOtherData(void *a, pointer x, void *dest, size_t s, int z) {
	memcpy(dest, x, s);
	return s;
}

slib_exp_lib_list_t *slib_exp_lib_list(void) {
	SifRpcReceiveData_t RData;
	slib_exp_lib_t *core_exps;
	slib_exp_lib_list_t *exp_lib_list = NULL;
	u32 i, addr, core_end, NextMod, *exp_func;
	void *pGetLoadcoreInternalData;
	smod_mod_info_t *ModInfo;

	/* Read the start of the global module table - this is where we will search.  */
	if(SifRpcGetOtherData(&RData, (void*)0x800, &smem_buf, sizeof(smod_mod_info_t), 0)>=0){
		/* The first entry points to LOADCORE's module info.  We then use the
		   module info to determine the end of LOADCORE's .text segment (just
		   past the export library we're trying to find.  */
		NextMod = *smem_buf.words;
		if(SifRpcGetOtherData(&RData, (void*)NextMod, &smem_buf, sizeof(smod_mod_info_t), 0)>=0){
			ModInfo = &smem_buf.mod_info;
			core_end = ModInfo->text_start+ModInfo->text_size;

			/* Back up so we position ourselves infront of where the export
			   library will be.  */
			if(SifRpcGetOtherData(&RData, (void*)(core_end - 512), &smem_buf, 512, 0)>=0){
				/* Search for LOADCORE's export library.  */
				for (i = 0; i < 512; i += 4) {
					/* SYSMEM's export library sits at 0x830, so it should appear in
					   LOADCORE's prev pointer.  */
					if (smem_buf.words[i / sizeof(u32)] == 0x830) {
						if (!memcmp(smem_buf.bytes + i + 12, "loadcore", 8))
						//if(*(unsigned int *)(smem_buf.bytes + i + 12) == 0x64616f6c) // 6c 6f 61 64 == load
							break;
					}
				}
				if (i >= 512)
					return NULL;

				/* Get to the start of the export table, and find the address of the
				   routine that will get us the export library list info.  */
				core_exps = (slib_exp_lib_t *)(smem_buf.bytes + i);
				pGetLoadcoreInternalData = core_exps->exports[3];

				if(SifRpcGetOtherData(&RData, pGetLoadcoreInternalData, &smem_buf, 8, 0)>=0){
					exp_func = smem_buf.words;

					/* Parse the two instructions that hold the address of the table.  */
					if ((exp_func[0] & 0xffff0000) != 0x3c020000)	/* lui v0, XXXX */
						return NULL;
					if ((exp_func[1] & 0xffff0000) != 0x24420000)	/* addiu v0, v0, XXXX */
						return NULL;

					addr = ((exp_func[0] & 0xffff) << 16) | (exp_func[1] & 0xffff);

					if(SifRpcGetOtherData(&RData, (void*)addr, &smem_buf, 8, 0)>=0){
						_slib_cur_exp_lib_list.tail = (slib_exp_lib_t *)(smem_buf.words[0]);
						_slib_cur_exp_lib_list.head = (slib_exp_lib_t *)(smem_buf.words[1]);
						exp_lib_list = &_slib_cur_exp_lib_list;
					}
				}
			}
		}
	}

	return exp_lib_list;
}

#define EXP_LIB_MAX	SMEM_BUF_SIZE	/* We can even handle CDVDMAN's bloat!  */

int slib_get_exp_lib(const char *name, slib_exp_lib_t *library)
{
	SifRpcReceiveData_t RData;
	slib_exp_lib_list_t *exp_lib_list = &_slib_cur_exp_lib_list;
	slib_exp_lib_t *exp_lib = &smem_buf.exp_lib;
	void *cur_lib;
	int len = strlen(name), count = 0;

	if (!exp_lib_list->head && !(exp_lib_list = slib_exp_lib_list()))
		return 0;

	/* Read the tail export library to initiate the search.  */
	cur_lib = exp_lib_list->tail;

	while (cur_lib) {
		if(SifRpcGetOtherData(&RData, cur_lib, exp_lib, EXP_LIB_MAX, 0)>=0){
			if (!memcmp(exp_lib->name, name, len)) {
				while (exp_lib->exports[count] != 0)
					count++;

				if (library)
					memcpy(library, exp_lib, sizeof(slib_exp_lib_t) + count * 4);

				return count;
			}

			cur_lib = exp_lib->prev;
		}
	}

	return 0;
}

void *resolve(char *name, int export) {
	slib_exp_lib_t *modload_lib = (void *)0x100;

	memset(&_slib_cur_exp_lib_list, 0, sizeof(slib_exp_lib_list_t));

	if (!slib_get_exp_lib(name, modload_lib)) {
		return NULL;
	}

	return modload_lib->exports[export];
}
