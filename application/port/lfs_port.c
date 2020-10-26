#include "lfs.h"
#include "flash.h"
#include "log.h"
#include "auto_init.h"
#include "shell.h"

#ifndef LFS_READ_SIZE
    #define LFS_READ_SIZE 16
#endif

#ifndef LFS_PROG_SIZE
    #define LFS_PROG_SIZE 16
#endif

#ifndef LFS_BLOCK_SIZE
    #define LFS_BLOCK_SIZE 512
#endif

#ifndef LFS_BLOCK_COUNT
    #define LFS_BLOCK_COUNT 64
#endif

#ifndef LFS_BLOCK_BASE_ADDR
    #define LFS_BLOCK_BASE_ADDR 0x38000 //0x38000-0x3FFFF 32K
#endif

#ifndef LFS_CACHE_SIZE
    #define LFS_CACHE_SIZE LFS_PROG_SIZE
#endif

#ifndef LFS_BLOCK_CYCLES
    #define LFS_BLOCK_CYCLES 100
#endif

#ifndef LFS_LOOKAHEAD_SIZE
    #define LFS_LOOKAHEAD_SIZE 128
#endif

// variables used by the filesystem
lfs_t lfs;

uint8_t lfs_read_buffer[LFS_READ_SIZE] = {0};
uint8_t lfs_prog_buffer[LFS_PROG_SIZE] = {0};
uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_SIZE] = {0};

static int block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
static int block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
static int block_device_erase(const struct lfs_config *c, lfs_block_t block);
static int block_device_sync(const struct lfs_config *c);

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = block_device_read,
    .prog  = block_device_prog,
    .erase = block_device_erase,
    .sync  = block_device_sync,
    
    // block device configuration
    .read_size      = LFS_READ_SIZE,
    .prog_size      = LFS_PROG_SIZE,
    .block_size     = LFS_BLOCK_SIZE,
    .block_count    = LFS_BLOCK_COUNT,
    .cache_size     = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .block_cycles   = LFS_BLOCK_CYCLES,
    
    .read_buffer    = lfs_read_buffer,
    .prog_buffer    = lfs_prog_buffer,
    .lookahead_buffer = lfs_lookahead_buffer,
};


static int block_device_read(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint8_t *index = (uint8_t *)buffer;
    for(int i = 0; i < size; i++)
    {
        *(index + i) = *((volatile uint8_t*)(block * c->block_size + off + LFS_BLOCK_BASE_ADDR + i));
    }
 
	return 0;
}
 
static int block_device_prog(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint8_t *index = (uint8_t *)buffer;
    for(int i = 0; i < size; i++)
    {
        Flash_WriteByte((block * c->block_size + off + LFS_BLOCK_BASE_ADDR + i), *(index + i));
    }
	return 0;
}
 
static int block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
    Flash_SectorErase((block * c->block_size + LFS_BLOCK_BASE_ADDR));
	return 0;
}
 
static int block_device_sync(const struct lfs_config *c)
{
	return 0;
}

int lfs_port_init(void)
{
    ///< 确保初始化正确执行后方能进行FLASH编程操作，FLASH初始化（编程时间,休眠模式配置）
    while(Ok != Flash_Init(12, TRUE))
    {
        ;
    }
    
    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

#if 0   // 测试移植是否成功
    lfs_file_t file;
    // read current count
    uint32_t boot_count = 0;
    //lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    struct lfs_file_config defaults = {0};
    char buf[LFS_CACHE_SIZE] = {0};
    defaults.buffer = buf;
    lfs_file_opencfg(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT, &defaults);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // print the boot count
    //logInfo("boot_count: %d\n", boot_count);
#endif
    return 0;
}
INIT_COMPONENT_EXPORT(lfs_port_init);

int lfs_uninit(void)
{
    return lfs_unmount(&lfs);
}

#if 1 // 文件系统相关 shell 命令
void lfs_print_err(int err)
{
    switch(err)
    {
        case LFS_ERR_IO:
        {
            logPrintln("Error during device operation");
            break;
        }
        
        case LFS_ERR_CORRUPT:
        {
            logPrintln("Corrupted");
            break;
        }
        
        case LFS_ERR_NOENT:
        {
            logPrintln("No directory entry");
            break;
        }
        
        case LFS_ERR_EXIST:
        {
            logPrintln("Entry already exists");
            break;
        }
        
        case LFS_ERR_NOTDIR:
        {
            logPrintln("Entry is not a dir");
            break;
        }
        
        case LFS_ERR_ISDIR:
        {
            logPrintln("Entry is a dir");
            break;
        }
        
        case LFS_ERR_NOTEMPTY:
        {
            logPrintln("Dir is not empty");
            break;
        }
        
        case LFS_ERR_BADF:
        {
            logPrintln("Bad file number");
            break;
        }
        
        case LFS_ERR_FBIG:
        {
            logPrintln("File too large");
            break;
        }
        
        case LFS_ERR_INVAL:
        {
            logPrintln("Invalid parameter");
            break;
        }
        
        case LFS_ERR_NOSPC:
        {
            logPrintln("No space left on device");
            break;
        }
        
        case LFS_ERR_NOMEM:
        {
            logPrintln("No more memory available");
            break;
        }
        
        case LFS_ERR_NOATTR:
        {
            logPrintln("No data/attr available");
            break;
        }
        
        case LFS_ERR_NAMETOOLONG:
        {
            logPrintln("File name too long");
            break;
        }
    }
}

int lfs_ls(lfs_t *pLfs, const char *path) {
    lfs_dir_t dir;
    int err = lfs_dir_open(pLfs, &dir, path);
    if (err) {
        lfs_print_err(err);
        return err;
    }

    struct lfs_info info;
    while (true) {
        int res = lfs_dir_read(pLfs, &dir, &info);
        if (res < 0) {
            return res;
        }

        if (res == 0) {
            break;
        }

        switch (info.type) {
            case LFS_TYPE_REG: logPrint("reg "); break;
            case LFS_TYPE_DIR: logPrint("dir "); break;
            default:           logPrint("?   "); break;
        }

        static const char *prefixes[] = {"", "K", "M", "G"};
        for (int i = sizeof(prefixes)/sizeof(prefixes[0])-1; i >= 0; i--) {
            if (info.size >= (1 << 10*i)-1) {
                logPrint("%*u%sB ", 4-(i != 0), info.size >> 10*i, prefixes[i]);
                break;
            }
        }

        logPrint("%s\n", info.name);
    }

    err = lfs_dir_close(pLfs, &dir);
    if (err) {
        lfs_print_err(err);
        return err;
    }

    return 0;
}

void doDir()
{
	int done = 0, error;

	lfs_dir_t lfs_dir;
	struct lfs_info entryinfo;
	lfs_ssize_t lfs_ssize = lfs_fs_size(&lfs);

	lfs_dir_open(&lfs, &lfs_dir, "/");

	while(done == 0)
	{
		error = lfs_dir_read(&lfs, &lfs_dir, &entryinfo);

		if(error <= 0)
        {
            lfs_print_err(error);
			done = 1;
        }

		logPrint("%s %-10d %s\n"
				,(entryinfo.type==LFS_TYPE_REG)?"PRG":"DIR"
				,entryinfo.size
				,entryinfo.name
				);
	}
	logPrint("Total size: %-10d Used: %-10d\n",cfg.block_size*cfg.block_count,lfs_ssize);

	lfs_dir_close(&lfs, &lfs_dir);
}

int shell_ls(int argc, char *argv[])
{
#if 1
    if(argc > 1)
    {
        return lfs_ls(&lfs, argv[1]);
    }
    else
    {
        return lfs_ls(&lfs, "/");
    }
#else
    doDir();
#endif
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ls, shell_ls, list files);

int shell_touch(int argc, char *argv[])
{
    lfs_file_t file;
    
    struct lfs_file_config defaults = {0};
    char buf[LFS_CACHE_SIZE] = {0};
    defaults.buffer = buf;
    if(argc < 2)
    {
        logPrintln("touch <fileName>");
        return -1;
    }
    int error = lfs_file_opencfg(&lfs, &file, argv[1], LFS_O_RDWR | LFS_O_CREAT, &defaults);
    
	if(error < 0)
	{
        lfs_print_err(error);
		logPrint("Error while touching file: %d\n",error);
		return error;
	}
	lfs_file_close(&lfs, &file);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), touch, shell_touch, touch file);

int shell_rm(int argc, char *argv[])
{
    if(argc < 2)
    {
        logPrintln("rm <fileName>");
        return -1;
    }
    
    int err = lfs_remove(&lfs, argv[1]);
    if(err < 0)
    {
        lfs_print_err(err);
        return err;
    }
    
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), rm, shell_rm, remove file or directory);
#endif
