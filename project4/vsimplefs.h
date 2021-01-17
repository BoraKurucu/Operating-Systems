#ifndef __VSFS__
#define __VSFS__

// Do not change this file //

#define MODE_READ 0
#define MODE_APPEND 1
#define BLOCKSIZE 4096 // bytes

int read_block (void *block, int k);

int write_block ( void *block, int k );

int create_format_vdisk (char *vdiskname, unsigned int m);

int vsfs_mount (char *vdiskname);

int vsfs_umount (); 

int vsfs_create(char *filename);

int vsfs_open(char *filename, int mode);

int vsfs_close(int fd);

int vsfs_getsize (int fd);

int vsfs_read(int fd, void *buf, int n);

int vsfs_append(int fd, void *buf, int n);

int vsfs_delete(char *filename);

#endif