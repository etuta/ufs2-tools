#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>
#include <ufs/ufs/dir.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SECTOR_SIZE DEV_BSIZE


void
listDirectory(u_int8_t* img, struct fs* fs, u_int32_t dir_inode, u_int32_t level) {
    
    u_int32_t i;
    struct ufs2_dinode* dir_inode_data = (struct ufs2_dinode*)(img + fsbtodb(fs, ino_to_fsba(fs, dir_inode)) * SECTOR_SIZE) + ino_to_fsbo(fs, dir_inode);

    /*printf("%lu %lu\n", dir_inode_data->di_blocks, UFS_NDADDR); */

    for (i=0; i<dir_inode_data->di_blocks; i++) {
        /*TODO indirect blocks*/
        /* ufs2_daddr_t    di_ib[UFS_NIADDR]; */
        ufs2_daddr_t curBlock;
        if (i < UFS_NDADDR) {
          curBlock = dir_inode_data->di_db[i];
        } 
        else {
          u_int32_t nonDireectBlockAddrs = (i-UFS_NDADDR) * sizeof(ufs2_daddr_t);
          u_int32_t blockOfblocks = nonDireectBlockAddrs / fs->fs_bsize;
          u_int32_t inBlockOffset = nonDireectBlockAddrs % fs->fs_bsize;
          /*printf("Indirect blocks\n"); */
          if (blockOfblocks >= UFS_NIADDR) {
              perror("Bad indirect block number");
              exit(-1); }

          curBlock = *((ufs2_daddr_t*)(img + fsbtodb(fs, dir_inode_data->di_ib[blockOfblocks]) * SECTOR_SIZE + inBlockOffset));          
        }
        
        struct  direct* curDir = (struct  direct*)(img + fsbtodb(fs, curBlock) * SECTOR_SIZE);
        u_int8_t* startPos=(u_int8_t*)curDir;
        /*printf("%lu\n", (char*)curDir - (char*)img);*/

        /*TODO steps in DIRBLKSIZ */
        while (curDir->d_reclen > 0 && curDir->d_ino!=0 && (u_int8_t*)curDir - startPos < fs->fs_bsize) {
             u_int32_t j=level;
             if (curDir->d_type != DT_DIR || (strcmp(curDir->d_name, "..") != 0 && strcmp(curDir->d_name, ".") != 0)) {
               u_int32_t j=level;
               while (j-- > 0) {
                   putc(' ', stdout);
                   putc(' ', stdout); }
                 
               printf("%s\n", curDir->d_name);
                 if (curDir->d_type == DT_DIR){
                     listDirectory(img, fs, curDir->d_ino, level + 1);}
             }

             curDir = (struct  direct*)((u_int8_t*)curDir + curDir->d_reclen);
        }
   }
}


int
main(int argc, char* argv[]){
    
    if (argc!=2) {
      perror("Please specify partition file!");
      return -1;

    } else {
      int fd = open(argv[1], O_RDONLY);

      if (fd!=-1) {
	  struct stat sb;

	  if (fstat(fd, &sb) == 0 && sb.st_size >= (SBLOCK_UFS2 + SBLOCKSIZE)) { 
	     void* img_data=NULL;
             printf("File size: %lu\n", sb.st_size); 
             img_data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	     if (img_data!=MAP_FAILED) {
		struct fs* sblock = (struct fs*)((char*)img_data + SBLOCK_UFS2);
		printf("Mapped successfully\n");

		if (sblock->fs_magic == FS_UFS2_MAGIC) {
                  int i;
                  printf("Superblock found!\n");


/*		  printf("Root dir block: %lu, offset: %lu, %lu\n",fsbtodb(sblock, ino_to_fsba(sblock, UFS_ROOTINO)), ino_to_fsbo(sblock, UFS_ROOTINO), lblktosize(sblock, ino_to_fsba(sblock, UFS_ROOTINO)));

                  printf("inode size: %lu fs block: %lu\n", sizeof(struct ufs2_dinode), sblock->fs_bsize); */
/*                  printf("root inode offset: %lu\n", (char*)root - (char*)img_data); */
                   listDirectory(img_data, sblock, UFS_ROOTINO, 0);


		} else {
	          perror("Unable to find UFS2 superblock");
		}
                munmap(img_data, sb.st_size);
	     } else {
	        perror("Unable to map partition to the memory");
	     }
      
      
	  } else {
            perror("Unable to stat partition file or the image is too small");
	  }
	  close(fd);
      } else {
	 perror("Unable to open partition file");
      } 
    } 
    return 0;
}

 
