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
listFile(u_int8_t* img, struct fs* fs, u_int32_t file_inode) {
    
    u_int32_t i;
    struct ufs2_dinode* file_inode_data = (struct ufs2_dinode*)(img + fsbtodb(fs, ino_to_fsba(fs, file_inode)) * SECTOR_SIZE) + ino_to_fsbo(fs, file_inode);
    if ((file_inode_data->di_mode & IFREG) == 0) return;

    u_int32_t to_print = file_inode_data->di_size;

    for (i=0; i<file_inode_data->di_blocks; i++) {
        if (to_print == 0) break;

        ufs2_daddr_t curBlock;
        if (i < UFS_NDADDR) {
          curBlock = file_inode_data->di_db[i];
        }
        
        else {
          u_int32_t nonDireectBlockAddrs = (i-UFS_NDADDR) * sizeof(ufs2_daddr_t);
          u_int32_t blockOfblocks = nonDireectBlockAddrs / fs->fs_bsize;
          u_int32_t inBlockOffset = nonDireectBlockAddrs % fs->fs_bsize;
            
          if (blockOfblocks >= UFS_NIADDR){
              perror("Bad indirect block number");
              exit(-1); }
          
          curBlock = *((ufs2_daddr_t*)(img + fsbtodb(fs, file_inode_data->di_ib[blockOfblocks]) * SECTOR_SIZE + inBlockOffset));
        }
        
        u_int32_t printSize = to_print < fs->fs_bsize? to_print : fs->fs_bsize;
        u_int8_t* curContent = (img + fsbtodb(fs, curBlock) * SECTOR_SIZE);
        fwrite(curContent, 1, printSize, stdout);
        to_print-=printSize;

   }
}


u_int32_t
locateInodeByName(u_int8_t* img, struct fs* fs, u_int32_t curINode, char* name) {
    
   u_int32_t result = 0, i;
   struct ufs2_dinode* dir_inode_data = (struct ufs2_dinode*)(img + fsbtodb(fs, ino_to_fsba(fs, curINode)) * SECTOR_SIZE) + ino_to_fsbo(fs, curINode);
   if (dir_inode_data->di_mode & IFDIR != 0) {
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
            
          if (blockOfblocks >= UFS_NIADDR){
              perror("Bad indirect block number");
              exit(-1); }
    
          curBlock = *(img + fsbtodb(fs, dir_inode_data->di_ib[blockOfblocks]) * SECTOR_SIZE + inBlockOffset);
        }
        struct  direct* curDir = (struct  direct*)(img + fsbtodb(fs, curBlock) * SECTOR_SIZE);
        u_int8_t* startPos=(u_int8_t*)curDir;
        /*printf("%lu\n", (char*)curDir - (char*)img);*/

        /*TODO steps in DIRBLKSIZ */
        while (curDir->d_reclen > 0 && curDir->d_ino!=0 && (u_int8_t*)curDir - startPos < fs->fs_bsize) {
             if (strcmp(curDir->d_name, name) == 0) {
               return curDir->d_ino;
             }

             curDir = (struct  direct*)((u_int8_t*)curDir + curDir->d_reclen);
        }
    }
   }
   return result;
}

u_int32_t
inodeByPath (u_int8_t* img, struct fs* fs, char* path) {
    
   u_int32_t result = UFS_ROOTINO;
   char* token = strtok(path, "/");
   while (token != NULL) {
      result = locateInodeByName(img, fs, result, token);
      if (result < UFS_ROOTINO) break;
      token = strtok(NULL, "/");
   }
   return result;
}

int
main(int argc, char* argv[]){
    
    if (argc!=3) {
      perror("Please specify partition file, and the file in the partition!");
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
                   u_int32_t fileInode = inodeByPath(img_data, sblock, argv[2]);
                   if (fileInode!=0)
                       listFile(img_data, sblock, fileInode);


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

 
