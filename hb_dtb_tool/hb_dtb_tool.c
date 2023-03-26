#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include "hb_dtb_tool.h"

int main(int argc, char *argv[])
{
    char *imgfile = NULL;
    char *dtb_file = NULL;
    unsigned int board_id = 0xffff;
    struct stat fstat;
    struct andr_img_hdr *boot_hdr;
    struct hb_kernel_hdr *hbk_hdr;
    struct hb_dtb_hdr *hbdtb_hdr;
    int set_dtb_flg = 0;
    int get_dtb_flg = 0;
    int ch, ret;
    while ((ch = getopt(argc, argv, "i:b:gs:h")) != -1)
    {
        // printf("optind: %d\n", optind);
        switch (ch)
        {
        case 'i':
            imgfile = optarg;
            break;
        case 'b':
            if (optarg[1] == 'x' || optarg[1] == 'X')
            {
                board_id = htoi(optarg);
            }
            else
            {
                board_id = atoi(optarg);
            }
            break;
        case 'g':
            get_dtb_flg = 1;
            break;
        case 's':
            dtb_file = optarg;
            set_dtb_flg = 1;
            break;
        case 'h':
            usage(argv[0]);
            return -1;
        case '?':
            usage(argv[0]);
            return -1;
        }
    }
    if (imgfile == NULL || board_id == 0xffff)
    {
        usage(argv[0]);
        return -1;
    }
    if (get_dtb_flg)
    {
        ret = parse_dtb_from_img(imgfile, board_id);
        if (ret < 0)
        {
            printf("parse_dtb_from_img failed!\n");
            return -1;
        }
    }
    else if (set_dtb_flg && dtb_file)
    {
        ret = flash_dtb_to_img(imgfile, dtb_file, board_id);
        if (ret < 0)
        {
            printf("flash_dtb_to_img failed!\n");
            return -1;
        }
    }
    else
    {
        usage(argv[0]);
        return -1;
    }
}


unsigned int big2lit(unsigned int data)
{
    unsigned char buf[4];
    unsigned int int_size = sizeof(int);
    for (size_t i = 0; i < int_size; i++)
    {
        buf[i] = 0xff & (data >> (8 * i));
    }
    data = 0;
    for (size_t i = 0; i < int_size; i++)
    {
        data = data | (buf[int_size - 1 - i] << (8 * i));
    }
    return data;
}

int fdt_check_header(const void *fdt)
{
    if (fdt_magic(fdt) == FDT_MAGIC)
    {
        /* Complete tree */
        if (fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION)
            return -1;
        if (fdt_last_comp_version(fdt) > FDT_LAST_SUPPORTED_VERSION)
            return -2;
    }
    else if (fdt_magic(fdt) == FDT_SW_MAGIC)
    {
        /* Unfinished sequential-write blob */
        if (fdt_size_dt_struct(fdt) == 0)
            return -3;
    }
    else
    {
        return -4;
    }
    return fdt_totalsize(fdt);
}

int android_image_check_header(const struct andr_img_hdr *hdr)
{
	return memcmp(ANDR_BOOT_MAGIC, hdr->magic, ANDR_BOOT_MAGIC_SIZE);
}

void show_dtb_info(struct hb_dtb_hdr *hbdtb_hdr)
{
    if (hbdtb_hdr)
    {
        printf("board_id\t%d\n", hbdtb_hdr->board_id);
        printf("dtb_name\t%s\n", hbdtb_hdr->dtb_name);
        printf("dtb_size\t%d\n", hbdtb_hdr->dtb_size);
        printf("dtb_addr\t%d\n", hbdtb_hdr->dtb_addr);
    }
    else
    {
        printf("show_dtb_info has err parm\n");
    }
}

struct hb_dtb_hdr *get_hb_dtb(unsigned int board_type, struct hb_kernel_hdr *config)
{
    struct hb_dtb_hdr *pdtb = NULL;
    int i, dtb_number;
    if (!config)
    {
        printf("error: hb_kernel_hdr is null !\n");
        return NULL;
    }
    pdtb = config->dtb;
    dtb_number = config->dtb_number;
    // printf("config->dtb_number: %d\n", dtb_number);
    if (dtb_number > DTB_MAX_NUM)
    {
        printf("error: count %02x not support\n", dtb_number);
        return NULL;
    }
    // for (i = 0; i < dtb_number; i++)
    // {
    //     pdtb = config->dtb + i;
    //     printf("dtb_name: %s board_id = 0x%x,dtb_addr = 0x%x,dtb_size = 0x%x,gpio_id = 0x%x\n", pdtb->dtb_name, pdtb->board_id, pdtb->dtb_addr, pdtb->dtb_size, pdtb->gpio_id);
    // }

    for (i = 0; i < dtb_number; i++)
    {
        if (board_type == config->dtb[i].board_id)
        {
            pdtb = config->dtb + i;
            break;
        }
    }
    if (i == dtb_number)
    {
        printf("error: board_type %02x not support\n", board_type);
        return NULL;
    }
    show_dtb_info(pdtb);
    return pdtb;
}

int readall(int fd, char *buf, int size)
{
    char *bptr = buf;
    int ret = 0;
    while (ret < size)
    {
        ret += read(fd, bptr, size);
        if (ret < 0)
        {
            perror("read err");
            return ret;
        }
    }
    return ret;
}

int writeall(int fd, char *buf, int size)
{
    char *bptr = buf;
    int ret = 0;
    while (ret < size)
    {
        ret += write(fd, bptr, size);
        if (ret < 0)
        {
            perror("write err");
            return ret;
        }
    }
    return ret;
}

int readwrite(int wfd, int rfd, int size)
{
    char buf[1024];
    char *bptr = buf;
    int read_ret = 0, write_ret = 0;
    while (read_ret < size)
    {
        read_ret += readall(rfd, bptr, 1024);
        write_ret += writeall(wfd, bptr, 1024);
        if (read_ret < 0)
        {
            perror("readall err");
            return read_ret;
        }
    }
    return read_ret;
}

int htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        i = 2;
    }
    else
    {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i)
    {
        if (tolower(s[i]) > '9')
        {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        }
        else
        {
            n = 16 * n + (tolower(s[i]) - '0');
        }
    }
    return n;
}

void usage(char *argv0)
{
    printf("Get a dtb from bootimg.\nUsage: %s -i [imgfile] -b [board_id] [option gs:] [dtb_file]\n", argv0);
}

int parse_dtb_from_img(char *imgfile, unsigned int board_id)
{
    int ret;
    int img_fd;
    char *buffer, *hbdtb_buf;
    unsigned char dtb_name[DTB_NAME_MAX_LEN];
    struct andr_img_hdr *boot_hdr;
    struct hb_kernel_hdr *hbk_hdr;
    struct hb_dtb_hdr *hbdtb_hdr;
    buffer = malloc(sizeof(struct andr_img_hdr));
    if (!buffer)
    {
        perror("malloc buffer");
        return -1;
    }
    img_fd = open(imgfile, O_RDONLY);
    if (img_fd < 0)
    {
        perror("imgfile");
        return -1;
    }
    ret = lseek(img_fd, 0x0, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek buffer");
        return -1;
    }
    ret = readall(img_fd, buffer, sizeof(struct andr_img_hdr));
    if (ret < 0)
    {
        perror("read buffer");
        return -1;
    }
    // printf("read buffer 0x%x,%s\n", ret, buffer);
    boot_hdr = (struct andr_img_hdr *)buffer;
    ret = android_image_check_header(boot_hdr);
    if (ret)
    {
        printf("android_image_check_header err");
        return -1;
    }
    // printf("kernel_size 0x%x,kernel_addr 0x%x,second_size 0x%x,,tags_addr 0x%x,cmdline %s,page_size 0x%x,\n",
    //        boot_hdr->kernel_size, boot_hdr->kernel_addr, boot_hdr->second_size, boot_hdr->tags_addr, boot_hdr->cmdline, boot_hdr->page_size);
    int k_off;
    //再加上ramdisk_size的
    if (boot_hdr->kernel_size % PAGE_SIZE)
    {
        k_off = (boot_hdr->kernel_size / PAGE_SIZE + 2) * PAGE_SIZE;
    }
    else
    {
        k_off = boot_hdr->kernel_size + 1;
    }
    ret = lseek(img_fd, k_off, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek k_off");
        return -1;
    }
    // printf("lseek off is 0x%x ks rm 0x%x\n", ret,boot_hdr->ramdisk_size);

    ret = readall(img_fd, buffer, sizeof(struct hb_kernel_hdr));
    if (ret < 0 && ret != sizeof(struct hb_kernel_hdr))
    {
        perror("read buffer");
        return -1;
    }
    hbk_hdr = (struct hb_kernel_hdr *)buffer;
    // printf("readall ret = 0x%x ,hbk_hdr is %p,Image_size 0x%x,Image_addr = 0x%x\n", ret, hbk_hdr, hbk_hdr->Image_size, hbk_hdr->Image_addr);
    // printf("kernel_hdr_off,dtb_addr 0x%x,%d,0x%x\n", kernel_hdr_off, dtb_addr, dtb_len);
    hbdtb_hdr = get_hb_dtb(board_id, hbk_hdr);
    if (!hbdtb_hdr)
    {
        printf("get dtb err! \n");
        return -1;
    }
    unsigned int dtb_addr = sizeof(struct hb_kernel_hdr) + k_off + hbdtb_hdr->dtb_addr;
    // printf("dtb_addr  0x%x,dtb_len 0x%x,dtbname %s\n", dtb_addr, hbdtb_hdr->dtb_size, hbdtb_hdr->dtb_name);

    strcpy(dtb_name, hbdtb_hdr->dtb_name);
    ret = lseek(img_fd, dtb_addr, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek buffer");
        return -1;
    }
    int dtbfd = open(hbdtb_hdr->dtb_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXG | S_IRWXU | S_IRWXO);
    if (dtbfd < 0)
    {
        perror(hbdtb_hdr->dtb_name);
        return -1;
    }
    // printf("img %d,dtb %d\n",img_fd,dtbfd);
    hbdtb_buf = malloc(hbdtb_hdr->dtb_size);
    ret = readall(img_fd, hbdtb_buf, hbdtb_hdr->dtb_size);
    if (ret < 0 && ret != hbdtb_hdr->dtb_size)
    {
        perror("readall buffer");
        return -1;
    }
    // printf("read len %x\n",ret);
    ret = writeall(dtbfd, hbdtb_buf, hbdtb_hdr->dtb_size);
    if (ret < 0 && ret != hbdtb_hdr->dtb_size)
    {
        perror("writeall buffer");
        return -1;
    }
    // printf("write len %x\n",ret);
    free(hbdtb_buf);
    // ret =  copy_file_range(img_fd, dtb_addr,
    //                            dtbfd, 0,
    //                            len, 0);

    close(dtbfd);
    ret = lseek(img_fd, dtb_addr, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek buffer");
        return -1;
    }
    ret = readall(img_fd, buffer, sizeof(struct fdt_header));
    if (ret < 0 && ret != sizeof(struct fdt_header))
    {
        perror("read buffer");
        return -1;
    }
    ret = fdt_check_header(buffer);
    if (ret < 0)
    {
        printf("fdt_check_header is failed ret %d\n", ret);
        return -1;
    }
    // printf("parse_dtb_from_img fdt_totalsize %d \n",fdt_totalsize(buffer));
    // printf("name\t%s\nsize\t%d\n", dtb_name, ret);
    free(buffer);
    close(img_fd);
    return 0;
}

int flash_dtb_to_img(char *imgfile, char *dtb_file, unsigned int board_id)
{
    int ret, dtb_fd, img_fd;
    unsigned char *buffer, *hbdtb_buf;
    struct stat fstat;
    unsigned int dtb_addr;
    unsigned char dtb_name[DTB_NAME_MAX_LEN];
    struct andr_img_hdr *boot_hdr;
    struct hb_kernel_hdr *hbk_hdr;
    struct hb_dtb_hdr *hbdtb_hdr;

    ret = stat(dtb_file, &fstat);
    if (ret < 0)
    {
        perror("stat file err");
        return -1;
    }
    hbdtb_buf = malloc(fstat.st_size);
    if (!hbdtb_buf)
    {
        perror("malloc hbdtb_buf");
        return -1;
    }
    dtb_fd = open(dtb_file, O_RDONLY);
    if (dtb_fd < 0)
    {
        perror(dtb_file);
        return -1;
    }
    ret = readall(dtb_fd, hbdtb_buf, fstat.st_size);
    if (ret < 0 || ret != fstat.st_size)
    {
        perror("readall buffer");
        return -1;
    }
    ret = fdt_check_header(hbdtb_buf);
    if (ret < 0)
    {
        printf("dtb_file %s fdt_check_header is failed ret %d\n", dtb_file, ret);
        return -1;
    }
    printf("flash_dtb_to_img fdt_totalsize %d \n",fdt_totalsize(hbdtb_buf));
    // if (fdt_totalsize(hbdtb_buf) != fstat.st_size)
    // {
    //     printf("fdt_totalsize %d is not eq %s %ld\n", fdt_totalsize(hbdtb_buf), dtb_file, fstat.st_size);
    //     return -1;
    // }
    buffer = malloc(sizeof(struct andr_img_hdr));
    if (!buffer)
    {
        perror("malloc buffer");
        return -1;
    }
    img_fd = open(imgfile, O_RDWR);
    if (img_fd < 0)
    {
        perror("imgfile");
        return -1;
    }
    ret = lseek(img_fd, 0x0, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek buffer");
        return -1;
    }
    ret = readall(img_fd, buffer, sizeof(struct andr_img_hdr));
    if (ret < 0)
    {
        perror("read buffer");
        return -1;
    }
    // printf("read buffer 0x%x,%s\n", ret, buffer);
    boot_hdr = (struct andr_img_hdr *)buffer;
    ret = android_image_check_header(boot_hdr);
    if (ret)
    {
        printf("android_image_check_header err");
        return -1;
    }
    
    // printf("kernel_size 0x%x,kernel_addr 0x%x,second_size 0x%x,,tags_addr 0x%x,cmdline %s,page_size 0x%x,\n",
    //        boot_hdr->kernel_size, boot_hdr->kernel_addr, boot_hdr->second_size, boot_hdr->tags_addr, boot_hdr->cmdline, boot_hdr->page_size);
    int k_off;
    //再加上ramdisk_size的
    if (boot_hdr->kernel_size % PAGE_SIZE)
    {
        k_off = (boot_hdr->kernel_size / PAGE_SIZE + 2) * PAGE_SIZE;
    }
    else
    {
        k_off = boot_hdr->kernel_size + 1;
    }

    ret = lseek(img_fd, k_off, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek k_off");
        return -1;
    }
    // printf("lseek off is 0x%x\n", ret);
    ret = readall(img_fd, buffer, sizeof(struct hb_kernel_hdr));
    if (ret < 0 && ret != sizeof(struct hb_kernel_hdr))
    {
        perror("read buffer");
        return -1;
    }
    hbk_hdr = (struct hb_kernel_hdr *)buffer;
    // printf("readall ret = 0x%x ,hbk_hdr is %p,Image_size 0x%x,Image_addr = 0x%x\n", ret, hbk_hdr, hbk_hdr->Image_size, hbk_hdr->Image_addr);
    // printf("kernel_hdr_off,dtb_addr 0x%x,%d,0x%x\n", kernel_hdr_off, dtb_addr, dtb_len);
    hbdtb_hdr = get_hb_dtb(board_id, hbk_hdr);
    if (!hbdtb_hdr)
    {
        printf("get dtb err! \n");
        return -1;
    }
    //update hb_kernel_hdr
    hbdtb_hdr->dtb_size = fdt_totalsize(hbdtb_buf);
    ret = lseek(img_fd, k_off, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek k_off");
        return -1;
    }

    ret = writeall(img_fd, buffer, sizeof(struct hb_kernel_hdr));
    if (ret < 0 && ret != sizeof(struct hb_kernel_hdr))
    {
        perror("writeall buffer");
        return -1;
    }

    dtb_addr = sizeof(struct hb_kernel_hdr) + k_off + hbdtb_hdr->dtb_addr;
    strcpy(dtb_name, hbdtb_hdr->dtb_name);

    ret = lseek(img_fd, dtb_addr, SEEK_SET);
    if (ret < 0)
    {
        perror("lseek buffer");
        return -1;
    }
    ret = writeall(img_fd, hbdtb_buf, fstat.st_size);
    if (ret < 0 && ret != hbdtb_hdr->dtb_size)
    {
        perror("writeall buffer");
        return -1;
    }
    
    printf("FLASH DONE\n");
    free(buffer);
    free(hbdtb_buf);
    close(dtb_fd);
    close(img_fd);
    return 0;
}