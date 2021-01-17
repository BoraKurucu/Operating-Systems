#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "vsimplefs.h"
#include "superblock.h"
#include "fat.h"
#include "root_block.h"
#include "otp.h"

// Global Variables =======================================
int vdisk_fd; // Global virtual disk file descriptor. Global within the library.
              // Will be assigned with the vsfs_mount call.
              // Any function in this file can use this.
              // Applications will not use  this directly. 
// ========================================================

// read block k from disk (virtual disk) into buffer block.
// size of the block is BLOCKSIZE.
// space for block must be allocated outside of this function.
// block numbers start from 0 in the virtual disk.

int read_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = read ( vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	    printf ("read error\n");
	return -1;
    }
    return (0); 
}

// write block k into the virtual disk. 
int write_block ( void *block, int k )
{
    if( k == 0 ){
        //printf("writing to block : %d\n",k);
        struct Superblock* sp = (struct Superblock*) block;
        //printf("free_fcb : %d, %d , %d \n" , sp->free_fcb[0],sp->free_fcb[1],sp->free_fcb[2] );
    }
        
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = write (vdisk_fd, block, BLOCKSIZE);
    
    if (n != BLOCKSIZE) {
	printf ("write error\n");
	return (-1);
    }
    return 0; 
}


/**********************************************************************
   The following functions are to be called by applications directly. 
***********************************************************************/

// this function is partially implemented.
int create_format_vdisk (char *vdiskname, unsigned int m)
{
    char command[1000];
    int size;
    int num = 1;
    int count;
    size  = num << m;
    count = size / BLOCKSIZE;
    //    printf ("%d %d", m, size);
    sprintf (command, "dd if=/dev/zero of=%s bs=%d count=%d",
             vdiskname, BLOCKSIZE, count);
    //printf ("executing command = %s\n", command);
    system (command);

    // now write the code to format the disk below.
    // .. your code... 

    //Initilize all block types

    vsfs_mount(vdiskname);
    
    struct FatBlock* fb;
    fb = (struct FatBlock*)malloc( sizeof( struct FatBlock ) );

    init_superblock( size );
    
    init_fat_block();

    init_dir_block();

    free( fb );
    
    vsfs_umount();

    return (0); 
}


// already implemented
int vsfs_mount (char *vdiskname)
{
    // simply open the Linux file vdiskname and in this
    // way make it ready to be used for other operations.
    // vdisk_fd is global; hence other function can use it. 
    vdisk_fd = open(vdiskname, O_RDWR); 
    return(0);
}


// already implemented
int vsfs_umount ()
{
    fsync (vdisk_fd); // copy everything in memory to disk
    close (vdisk_fd);
    return (0); 
}


int vsfs_create(char *filename)
{
    int fbc_num = get_empty_dir();
    if( fbc_num == -1 )return -1;

    //check if same file exist
    struct Superblock* sb;
    sb = (struct Superblock*)malloc( sizeof( struct Superblock ) );
    read_block ( sb, 0 );
  
    int i,j;  
    struct DirBlock* cur_dir_block;
    cur_dir_block = (struct DirBlock*)malloc( sizeof( struct DirBlock ) );
    for( i = 0 ; i < ROOT_BLOCKS ; i++ ){
        read_block( cur_dir_block , i + 1 );
        for( j = 0 ; j < BLOCKSIZE / sizeof( struct DirEntry) ; ++j ){
            if( sb->free_fcb[ i * ( BLOCKSIZE / sizeof( struct DirEntry ) ) + j ] == 1 && ( strcmp( filename, cur_dir_block->entries[j].name ) == 0 ) ){
                free( sb );
                free( cur_dir_block );
                return -1;
            }     
        }
    }

    free( sb );
    free( cur_dir_block );




    struct DirEntry* fbc = (struct DirEntry*)malloc( sizeof( struct DirEntry ) );
    fbc->file_size = 0;
    fbc->first_block = -1;
    strcpy( fbc->name , filename );

    //update empty directory list in superblock
    set_sb_dir( fbc_num );

    //update file control block in the root directory entries 
    set_dir_entry( fbc_num, fbc );

    free( fbc );
    return (0);
}


int vsfs_open(char *file, int mode)
{
    struct DirEntry* fbc = (struct DirEntry*)malloc( sizeof( struct DirEntry ) );

    int ret = find_dir_entry( file, fbc );
    if( ret == -1 ){
        free( fbc );
        return -1;
    }

    return add_otp_file( fbc, mode , ret );  
}

int vsfs_close(int fd){

    remove_otp_file( fd );

    return (0); 
}

int vsfs_getsize ( int  fd )
{
    return otp[fd].file_size;
    //return (0); 
}

int vsfs_read(int fd, void *buf, int n){

    //check valid fd and mode
    if( fd >= MAX_FILES || modes[fd] != MODE_READ ) return -1;

    //compare n with file size

    if( check_possible_read( fd , n ) == 0 ) return -1;

    int first_pos = last_pos[ fd ];

    int remaining_blocks = (first_pos + n - 1)/BLOCKSIZE - first_pos/BLOCKSIZE + 1;
    
    int buf_pos = 0;
    int cur_block = get_block_from_pos( otp + fd , last_pos[ fd ] );
    
    //printf(" last pos : %d , cur block : %d, remaining block : %d\n", last_pos[ fd ] ,cur_block, remaining_blocks );
    
    int cur_offset;

    char* content = (char*)malloc( BLOCKSIZE );

    read_block( content, cur_block );
    
    int i;
    for( i = 0 ; i < remaining_blocks ; ++i ){

        if( i == remaining_blocks - 1 ){

            //1 - transfer cur block to buf
            int last_offset;
            last_offset = ( first_pos + n ) % BLOCKSIZE;
            if( last_offset == 0 )last_offset = BLOCKSIZE;

            for( cur_offset = last_pos[ fd ] % BLOCKSIZE ; cur_offset < last_offset ; cur_offset++ ){
                
                //printf("content cur offset : %c\n" , content[ cur_offset ] );
                
                ( (char*)buf )[ buf_pos ] = content[ cur_offset ];
                buf_pos++;
            }

            //2 - update last position
            advance_last_pos(  fd , last_offset - last_pos[ fd ] % BLOCKSIZE );

        }
        else{

            //1 - transfer cur block to buf
            for( cur_offset = last_pos[ fd ] % BLOCKSIZE ; cur_offset < BLOCKSIZE ; cur_offset++ ){
                ( (char*)buf )[ buf_pos ] = content[ cur_offset ];
                buf_pos++;
            }
            advance_last_pos(  fd , BLOCKSIZE - last_pos[ fd ] % BLOCKSIZE );

            //2 - get next block number
            cur_block = get_next_fat( cur_block );

            //3 - read cur block to content
            read_block( content, cur_block );
        }

    }

    return (n); 
}


int vsfs_append(int fd, void *buf, int n )
{

    //check valid fd and mode
    if( fd >= MAX_FILES || modes[fd] != MODE_APPEND ) return -1;

    //find required new blocks
    int cur_blocks = otp[fd].file_size / BLOCKSIZE;
    if( otp[fd].file_size % BLOCKSIZE > 0 ) cur_blocks += 1;

    int new_blocks = ( otp[fd].file_size + n ) / BLOCKSIZE;
    if( ( otp[fd].file_size + n ) % BLOCKSIZE > 0 ) new_blocks += 1;
    
    int req_blocks = new_blocks - cur_blocks;

    //check empty space
    int t = check_enough_blocks( req_blocks );
    if( t == 0 )return -1;

    //write buffer
    write_blocks( otp + fd , buf, n , req_blocks );

    //TODO : update fcb both in disk and open file table
    
    //update fcb in memory
    otp[ fd ].file_size += n;

    //update fcb in disk
    set_dir_entry( entry_pos[ fd ] , otp + fd );

    return (n); 
}

int vsfs_delete( char *filename )
{
    int i;
    //find if file is open
    for( i = 0 ; i < MAX_FILES ; i++ ){
        if( valid_otp[i] == 1 && strcmp( otp[i].name , filename ) == 0  ){
            return -1;
        }
    }

    //get fbc
    struct DirEntry* fbc = (struct DirEntry*)malloc( sizeof( struct DirEntry ) );
    int ret = find_dir_entry( filename, fbc );

    //update bitmap of dir entries in superblock
    struct Superblock* sb;
    sb = (struct Superblock*)malloc( sizeof( struct Superblock ) );
    
    read_block( sb, 0 );
    sb->free_fcb[ ret ] = 0;
    write_block( sb ,0 );
    free( sb );

    //update FAT entries corresponding to deleted file content blocks
    int cur_block = fbc->first_block;
    int total_block = fbc->file_size / BLOCKSIZE;

    struct FatEntry* fe = (struct DirEntry*)malloc( sizeof( struct FatEntry ) );

    for( i = 0 ; i < total_block ; i++ ){
        get_fat_entry( cur_block, fe );
        fe->is_full = 0;
        write_fat_entry( cur_block, fe );

        cur_block = get_next_fat( cur_block );
    }

    free( fe );
    free( fbc );

    return (0); 
}

