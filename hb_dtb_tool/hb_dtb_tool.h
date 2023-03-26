#ifndef HB_DTB_TOOL_H_
#define HB_DTB_TOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#define fdt_get_header(fdt, field) big2lit((((const struct fdt_header *)(fdt))->field))
#define fdt_magic(fdt) (fdt_get_header(fdt, magic))
#define fdt_totalsize(fdt) (fdt_get_header(fdt, totalsize))
#define fdt_off_dt_struct(fdt) (fdt_get_header(fdt, off_dt_struct))
#define fdt_off_dt_strings(fdt) (fdt_get_header(fdt, off_dt_strings))
#define fdt_off_mem_rsvmap(fdt) (fdt_get_header(fdt, off_mem_rsvmap))
#define fdt_version(fdt) (fdt_get_header(fdt, version))
#define fdt_last_comp_version(fdt) (fdt_get_header(fdt, last_comp_version))
#define fdt_size_dt_struct(fdt) (fdt_get_header(fdt, size_dt_struct))

#define FDT_FIRST_SUPPORTED_VERSION 0x10
#define FDT_LAST_SUPPORTED_VERSION 0x11
#define FDT_MAGIC 0xd00dfeed /* 4: version, 4: total size */
#define FDT_SW_MAGIC (~FDT_MAGIC)

#define DTB_NAME_MAX_LEN 32
#define DTB_MAX_NUM 20
#define DTB_MAPPING_SIZE 0x400
#define DTB_NAME_MAX_LEN 32
#define DTB_RESERVE_SIZE ((DTB_MAPPING_SIZE - (DTB_MAX_NUM * 48) - 20) / 4)

#define ANDR_BOOT_MAGIC "ANDROID!"
#define ANDR_BOOT_MAGIC_SIZE 8
#define ANDR_BOOT_NAME_SIZE 16
#define ANDR_BOOT_ARGS_SIZE 512
#define ANDR_BOOT_EXTRA_ARGS_SIZE 1024
#define PAGE_SIZE 0x800
typedef unsigned int fdt32_t;
struct fdt_header
{
    fdt32_t magic;             /* magic word FDT_MAGIC */
    fdt32_t totalsize;         /* total size of DT block */
    fdt32_t off_dt_struct;     /* offset to structure */
    fdt32_t off_dt_strings;    /* offset to strings */
    fdt32_t off_mem_rsvmap;    /* offset to memory reserve map */
    fdt32_t version;           /* format version */
    fdt32_t last_comp_version; /* last compatible version */

    /* version 2 fields below */
    fdt32_t boot_cpuid_phys; /* Which physical CPU id we're
					    booting on */
    /* version 3 fields below */
    fdt32_t size_dt_strings; /* size of the strings block */

    /* version 17 fields below */
    fdt32_t size_dt_struct; /* size of the structure block */
};

struct andr_img_hdr
{
    char magic[ANDR_BOOT_MAGIC_SIZE];

    unsigned int kernel_size; /* size in bytes */
    unsigned int kernel_addr; /* physical load addr */

    unsigned int ramdisk_size; /* size in bytes */
    unsigned int ramdisk_addr; /* physical load addr */

    unsigned int second_size; /* size in bytes */
    unsigned int second_addr; /* physical load addr */

    unsigned int tags_addr; /* physical addr for kernel tags */
    unsigned int page_size; /* flash page size we assume */
    unsigned int unused;    /* reserved for future expansion: MUST be 0 */

    /* operating system version and security patch level; for
	 * version "A.B.C" and patch level "Y-M-D":
	 * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
	 * os_version = ver << 11 | lvl */
    unsigned int os_version;

    char name[ANDR_BOOT_NAME_SIZE]; /* asciiz product name */

    char cmdline[ANDR_BOOT_ARGS_SIZE];

    unsigned int id[8]; /* timestamp / checksum / sha1 / etc */

    /* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
    char extra_cmdline[ANDR_BOOT_EXTRA_ARGS_SIZE];
};

struct hb_dtb_hdr
{
    unsigned int board_id;
    unsigned int gpio_id;
    unsigned int dtb_addr; /* Address in storage */
    unsigned int dtb_size;
    unsigned char dtb_name[DTB_NAME_MAX_LEN];
};

struct hb_kernel_hdr
{
    unsigned int Image_addr; /* Address in storage */
    unsigned int Image_size;
    unsigned int Recovery_addr; /* Address in storage */
    unsigned int Recovery_size;
    unsigned int dtb_number;
    struct hb_dtb_hdr dtb[DTB_MAX_NUM];
    unsigned int reserved[DTB_RESERVE_SIZE];
};

unsigned int big2lit(unsigned int data);
int fdt_check_header(const void *fdt);
int android_image_check_header(const struct andr_img_hdr *hdr);
void show_dtb_info(struct hb_dtb_hdr *hbdtb_hdr);
struct hb_dtb_hdr *get_hb_dtb(unsigned int board_type, struct hb_kernel_hdr *config);
int readall(int fd, char *buf, int size);
int writeall(int fd, char *buf, int size);
int readwrite(int wfd, int rfd, int size);
int htoi(char s[]);
void usage(char *argv0);
int parse_dtb_from_img(char *imgfile, unsigned int board_id);
int flash_dtb_to_img(char *imgfile, char *dtb_file, unsigned int board_id);

#endif
