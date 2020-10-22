#include "lfs.h"
#include "flash.h"
#include "log.h"

#include "lfs_port.h"

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
lfs_file_t file;

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
	//sfud_read(w25q128, (block * c->block_size + off), size, (uint8_t*)buffer);
    for(int i = 0; i < size; i++)
    {
        *(index + i) = *((volatile uint8_t*)(block * c->block_size + off + LFS_BLOCK_BASE_ADDR + i));
    }
 
	return 0;
}
 
static int block_device_prog(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, const void *buffer, lfs_size_t size)
{
	//sfud_write(w25q128, (block * c->block_size + off), size, (uint8_t*)buffer);
    uint8_t *index = (uint8_t *)buffer;
    for(int i = 0; i < size; i++)
    {
        Flash_WriteByte((block * c->block_size + off + LFS_BLOCK_BASE_ADDR + i), *(index + i));
    }
	return 0;
}
 
static int block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	//sfud_erase(w25q128,(block * c->block_size), c->block_size);
    Flash_SectorErase((block * c->block_size + LFS_BLOCK_BASE_ADDR));
	return 0;
}
 
static int block_device_sync(const struct lfs_config *c)
{
	return 0;
}

void lfs_init(void)
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

#if 0   //测试移植是否成功
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
    logInfo("boot_count: %d\n", boot_count);
#endif
}

void lfs_uninit(void)
{
    lfs_unmount(&lfs);
}

