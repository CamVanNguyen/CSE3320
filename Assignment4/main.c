// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/*
* Name: Cam Nguyen 
* Assignment 4
*Description:file system based on indexed allocation
*
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#define NEWLINE "\n"
#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192 
#define NUM_FILES  128 //max number of files that can be supported by the system
#define NUM_INODES 128
#define MAX_BLOCKS_PER_FILE 32 
#define MAX_FILE_NAME 32
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define PUT_COMMAND "put"
#define GET_COMMAND "get"
#define DEL_COMMAND "del"
#define LIST_COMMAND "list"
#define DF_COMMAND "df"
#define UNDELETE_COMMAND "undelete"

unsigned char data_blocks[NUM_BLOCKS][BLOCK_SIZE];

int  used_blocks[NUM_BLOCKS];

struct inode {
  int size;
  time_t date;
  int valid;
  int first_block;
  int blocks[MAX_BLOCKS_PER_FILE];
};

struct directory_entry {
  char *name;
  int valid;
  int inode_idx;
};

int findFreeBlock();
struct directory_entry *directory_ptr;
struct inode * inode_array_ptr[NUM_INODES];
int findFreeInodeBlockEntry(int inode_index);
int df();
unsigned char file_data[NUM_BLOCKS][BLOCK_SIZE];
void commandPut(char **file_name);
int commandSelection(char **choice, int args);
int commandGet(char *input, char *output);
void commandList();
void init();
int findFreeDirectoryEntry();
int findFreeInode();
int findFreeBlockEntry();
int isDirectoryEmpty();
int inDirectory(char *filename);
int getArraySize(char **array);


int main( int argc, char *argv[] )
{
  init();
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    
    //only parses if it is not a new line 
    if(strcmp(cmd_str,NEWLINE))
    {
       // Parse input 
      char *token[MAX_NUM_ARGUMENTS];

      int   token_count = 0;                                 
                                                           
      // Pointer to point to the token
      // parsed by strsep
      char *arg_ptr;                                         
                                                           
      char *working_str  = strdup( cmd_str );                

      // we are going to move the working_str pointer so
      // keep track of its original value so we can deallocate
      // the correct amount at the end
      char *working_root = working_str;
   

      // Tokenize the input stringswith whitespace used as the delimiter
      while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && (token_count<MAX_NUM_ARGUMENTS))
      {
        token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );

        if( strlen( token[token_count] ) == 0 )
        {
          token[token_count] = NULL;
        }

        token_count++;
      }
    
      commandSelection(token, token_count);

      free(working_root);
     }
   } 
    free(cmd_str);     
    return 0;
 }

//removes the file from the file system
void del(char *filename)
{
  int i = 0;
  int location = -1;
  int block_index = 0;
  int size;

  struct stat buf;

  int status = stat( filename, &buf );
  
  //checks if the file exists in the directory
  if(status == -1)
  {
    perror("");
    return;
  }
  //checks if the file is within the directory
  location = inDirectory( filename);
  
  //file exists within the directory
  if(location != -1)
  {
     //makes it unvalid to free it for use
     directory_ptr[location].valid = 0;
     inode_array_ptr[directory_ptr[location].inode_idx]->valid =0;  
     // stores the size so we can loop through the blocks
     size = inode_array_ptr[directory_ptr[location].inode_idx]->size;
     //gets the first position of where it is stored in the blocks
     block_index = inode_array_ptr[directory_ptr[location].inode_idx]->first_block;
   
     while( size > 0)
     {
       // sets the valid to -1 to clean up the block, resets used block to 0 to reclaim the 
       // memory 
       inode_array_ptr[directory_ptr[location].inode_idx]->blocks[block_index] = -1;
       used_blocks[block_index] = 0;
       //decrements block by a block size and increases the block index to get the next section
       size -= BLOCK_SIZE;
       block_index++;
     }    
  }
  else
  {
    printf("del error: File not found.\n");
    return;
  }
}
int inDirectory(char *filename)
{
  int i = 0;
  int location = -1;
  
    // check if the name has not been overwritten yet
  for ( i = 0; i < NUM_FILES; i++)
  {
    //check if the name is not null
    if(directory_ptr[i].name)
    {
      //tests if the name that is in the directory is the same as the file that will be recovered
      if(!strncmp(directory_ptr[i].name,filename, strlen(filename)))
      {
        location = i;
        break;
      }
    }
  }
  return location;
}
void undelete(char *filename)
{
 
  int i = 0;
  int location = -1;
  int block_index = 0;
  int size;

  struct stat buf;

  int status = stat( filename, &buf );

  //checks if the file exists in the directory
  if(status == -1)
  {
    perror("");
    return;
  }
  
  // check if the name has not been overwritten yet
  location = inDirectory(filename);
 
  //file exists within the directory
  if(location != -1)
  {
     //makes it unvalid to free it for use
     directory_ptr[location].valid = 1;
     inode_array_ptr[directory_ptr[location].inode_idx]->valid = 1;
     // stores the size so we can loop through the blocks
     size = inode_array_ptr[directory_ptr[location].inode_idx]->size;
     //gets the first position of where it is stored in the blocks
     block_index = inode_array_ptr[directory_ptr[location].inode_idx]->first_block;
     int i = 0;
    
     while(size > 0)
     {
       // sets the valid to -1 to clean up the block, resets used block to 0 to reclaim the 
       // memory 
       inode_array_ptr[directory_ptr[location].inode_idx]->blocks[block_index] = block_index;
       used_blocks[block_index] = 1;
       //decrements block by a block size and increases the block index to get the next section
       size -= BLOCK_SIZE;
       block_index++;
     }    
     
  }
  else
  {
    printf("undelete eror: File cannot be recovered from the directory.\n");
    return;
  }
}      
void commandPut ( char **filename )
{
    struct stat buf;
    
    int status = stat( filename[1], &buf );
    //checks if the file exist in the current directory
    if( status == -1 ) 
    {
      printf("Error: File not found \n");
      return ;
    }
    // checks if the file is greater than the max possible file size
    if( buf.st_size > 32 * BLOCK_SIZE)
    {
      printf("Error: Not enough room in the file sysstem\n");
      return;
    }
    //checks if there is still disk space in the file system 
    if( buf.st_size > df() )
    {
      printf("Error: Not enough room in the file system \n");
      return ;
    }
    // finds  a free directory 
    int dir_idx = findFreeDirectoryEntry( );
    if( dir_idx == -1 )
    {
      printf("Error: Not enough room in the file system \n");
      return ;
    }
    
    if(strlen(filename[1]) <= 32)
    {
      // copies the name into the file directory 
      directory_ptr[dir_idx].name = (char*)malloc( strlen( filename[1] ) + 1 );
      memset(directory_ptr[dir_idx].name,0, strlen(filename[1]) +1);
      strncpy( directory_ptr[dir_idx].name, filename[1], strlen( filename[1] ) + 1);
    }
    else
    {
      printf("Error: File name is too long.\n");

    }

    // invalidates the bit since it is used
    directory_ptr[dir_idx].valid = 1;
    

    int inode_idx = findFreeInode( );
   
    if( inode_idx == -1 )
    {
      printf("Error: No free inodes\n");
      return;
    } 
    // stores the inode index, size, time the file was inserted 
    directory_ptr[dir_idx].inode_idx = inode_idx;

    inode_array_ptr[inode_idx]->size = buf.st_size;
    inode_array_ptr[inode_idx]->date = time( NULL );  
    inode_array_ptr[inode_idx]->valid = 1;

    // Open the input file read-only 
    FILE *ifp = fopen ( filename[1], "r" ); 

    int copy_size = buf.st_size;
    int offset    = 0;
   
    inode_array_ptr[inode_idx]->first_block = findFreeBlock();

    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 
    // memory pool. Why? We are simulating the way the file system stores file data in
    // blocks of space on the disk. block_index will keep us pointing to the area of
    // the area that we will read from or write to.
 
    // copy_size is initialized to the size of the input file so each loop iteration we
    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
    // we have copied all the data from the input file.
    while( copy_size >= BLOCK_SIZE )
    {
      int block_index = findFreeBlock();

      if( block_index == -1 )
      {
        printf("Error: Can't find free block\n");
        // Cleanup a bunch of directory and inode stuff
        return;
      }

      used_blocks[ block_index ] = 1;

      int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
      if( inode_block_entry == -1 )
      {
        printf("Error: Can't find free node block\n");
        // Cleanup a bunch of directory and inode stuff
        return;
      }
      inode_array_ptr[inode_idx]->blocks[inode_block_entry] = block_index;

      // Index into the input file by offset number of bytes.  Initially offset is set to
      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 
      // then increase the offset by BLOCK_SIZE and continue the process.  This will
      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
      fseek( ifp, offset, SEEK_SET );
 
      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array. 
      int bytes  = fread( data_blocks[block_index], BLOCK_SIZE, 1, ifp );

      // If bytes == 0 and we haven't reached the end of the file then something is 
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return;
      };

      clearerr( ifp );

      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
    }
    if( copy_size > 0 )
    {
      int block_index = findFreeBlock();

      if( block_index == -1 )
      {
        printf("Error: Can't find free block\n");
        // Cleanup a bunch of directory and inode stuff
        return;
      }

      int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
      if( inode_block_entry == -1 )
      {
        printf("Error: Can't find free node block\n");
        // Cleanup a bunch of directory and inode stuff
        return;
      }
      inode_array_ptr[inode_idx]->blocks[inode_block_entry] = block_index;

      used_blocks[ block_index ] = 1;

      // handle remainder
      fseek( ifp, offset, SEEK_SET );
      int bytes  = fread( data_blocks[block_index], copy_size, 1, ifp );
    }

  fclose( ifp );
  return;
}
int findFreeBlock( )
{
  int retval = -1;
  int      i = 0;
  // loops starting from the last index of an inode to 
  // the max number of blocks to find an empty block
  for( i = 130; i < 4226; i++ )
  { //checks if the used block is free and breaks with the return value having the 
    // first free index that was unused
    if( used_blocks[i] == 0 )
    {
      retval = i;
      break;
    }
  }
  return retval;
} 

int commandGet(char *input, char *output)// add another input and check if null or not
{
    //*********************************************************************************
    //
    // The following chunk of code demonstrates similar functionality to your get command
    //

  int    status;                   // Hold the status of all return values.
  struct stat buf;                 // stat struct to hold the returns from the stat call

  // Call stat with out input filename to verify that the file exists.  It will also 
  // allow us to get the file size. We also get interesting file system info about the
  // file such as inode number, block size, and number of blocks.  For now, we don't 
  // care about anything but the filesize.
  status =  stat(input, &buf );

  // If stat did not return -1 then we know the input file exists and we can use it.
  if( status != -1 )
  {
    //checks if the file exists within the directory
    int check = inDirectory(input);
    if(check ==  - 1)
    {
      printf("get error: File not found.\n");
      return -1;
    }
    //prints an error if the file is not actually valid
    if(directory_ptr[check].valid == 0)
    {
      printf("get error: File not found.\n");
      return -1;
    }
    
    FILE *ofp;
    if( output == NULL)
    {
      // opens input for writing b/c a second desitnation string was not provided
      ofp = fopen(input, "w");
    }
    else if(output)
    {
      // opens output for writing since it is teh destination the user provided
      ofp = fopen(output,"w");
    }
    
    if( ofp == NULL )
    {
      perror("Opening output file returned");
      return -1;
    }

    // Initialize our offsets and pointers just we did above when reading from the file.
    int inode_block_index = 0;
    // size from the inode and check is the index that returned the name from the directory pointer
    int copy_size   = inode_array_ptr[directory_ptr[check].inode_idx]->size;
    int offset      = 0;
    int block_index;

    // Using copy_size as a count to determine when we've copied enough bytes to the output file.
    // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from
    // our stored data to the file fp, then we will increment the offset into the file we are writing to.
    // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy
    // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the
    // last iteration we'd end up with gibberish at the end of our file. 
    while( copy_size >= BLOCK_SIZE )
    { 
      block_index = inode_array_ptr[directory_ptr[check].inode_idx]->
                    blocks[inode_block_index++];

      // Write num_bytes number of bytes from our data array into our output file.
      fwrite( data_blocks[block_index],BLOCK_SIZE, 1, ofp ); 

      // Reduce the amount of bytes remaining to copy, increase the offset into the file
      // and increment the block_index to move us to the next data block.
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      
      // Since we've copied from the point pointed to by our current file pointer, increment
      // offset number of bytes so we will be ready to copy to the next area of our output file.
      fseek( ofp, offset, SEEK_SET );
    }
    if( copy_size >  0)
    {
      block_index = inode_array_ptr[directory_ptr[check].inode_idx]->
                    blocks[inode_block_index++];

      // Write num_bytes number of bytes from our data array into our output file.
      fwrite( data_blocks[block_index],BLOCK_SIZE, 1, ofp );

      // Reduce the amount of bytes remaining to copy, increase the offset into the file
      // and increment the block_index to move us to the next data block.
      copy_size -= copy_size;
      offset    += copy_size;
      fseek (ofp,offset, SEEK_SET);
    }
    
    // Close the output file, we're done. 
    fclose( ofp );
  }
  else
  {
    perror("Opening the input file returned: ");
    return -1;
  }
  return 0;
}

void init()
{
  
  int i;
  directory_ptr = (struct directory_entry*) &data_blocks[0];
   
  //inits the validity to 0 so the file system will have 
  // them all valid initially
  for( i = 0; i < NUM_FILES; i++ )
  {
     directory_ptr[i].valid = 0;
  }
   
  int inode_idx = 0;
  //inits so that the data blocks contains all the inodes
  //sets validiity to 0 to be 
  for( i = 1; i < 130; i++ )
  {
    inode_array_ptr[inode_idx] = (struct inode*) &data_blocks[i];
    inode_array_ptr[inode_idx]->valid = 0;
    inode_array_ptr[inode_idx]->first_block = -1;
    int j;
    for( j = 0; j < 32; j++ )
    {
      inode_array_ptr[inode_idx]->blocks[j] = -1;
    }
    inode_idx++;
  }
  
  for(i = 0; i < NUM_BLOCKS;i++)
  {
    used_blocks[i] = 0;
  }

  used_blocks[0] = 1;

}

int findFreeDirectoryEntry()
{
  int i;
  int retval = -1;
  for( i = 0; i < 128; i++ )
  {
    if( directory_ptr[i].valid == 0 )
    {
      retval = i;
      break;
    }
  }
  return retval;
}

int findFreeInode()
{
  int i;
  int retval = -1;
  
  for( i = 0; i < 128; i++ )
  {
    
    if( inode_array_ptr[i]->valid == 0 )
    {
    
      retval = i;
      break;
    }
  }
  return retval;
}
int findFreeInodeBlockEntry( int inode_index )
{
  int i;
  int retval = -1;
  for( i = 0; i < 32; i++ )
  {
    if( inode_array_ptr[inode_index]->blocks[i] == -1 ) 
    {
      retval = i;
      break;
    }
  }
  return retval;
}

int commandSelection(char **choice, int args)
{
   if(choice == NULL)
   {
     return 0;
   } 
   if( !strcmp( PUT_COMMAND, choice[0]))
   { 
     commandPut(choice);      
     return 0;
   }
   else if (!strcmp( GET_COMMAND,choice[0]))
   {
    
     if(args == 3)
     {
       commandGet(choice[1], NULL);
     }
     else if( args == 4)
     {
       commandGet(choice[1],choice[2]);
     }
   }
   else if (!strcmp( DEL_COMMAND,choice[0]))
   {
     del(choice[1]);
   }
   else if (!strcmp(LIST_COMMAND,choice[0]))
   {
     commandList();
     return 0;
   }
   else if(!strcmp(DF_COMMAND,choice[0]))
   {
     printf("%d bytes free\n",df());
     return 0;
   }   
   else if(!strcmp(UNDELETE_COMMAND,choice[0]))
   {
     undelete(choice[1]);
   }
   else
   {
     printf("Command does not exist.\n");
     return 0;
   }
}

//calculates the free disk space
int df()
{
  int i = 130, count = 0;
  for( i = 130; i < 4226; i++)
  {
    if(used_blocks[i] == 0)
    {
      count++;
    }
  }
  return count * BLOCK_SIZE;
}
int isDirectoryEmpty()
{
 int i;
  int retval = 0;
  for( i = 0; i < 128; i++ )
  {
    if( directory_ptr[i].valid == 1 )
    {
      retval++;
    }
  }
  return retval;
}
void commandList()
{
  char *date_string;

  // Now iterate over the directory structure to make sure it's good.
  // This is similar to the list() function you need to write
  int idx = 0;
  if(!isDirectoryEmpty())
  {
    printf("list: No files found.\n");
  }
  else
  {
    for( idx = 0; idx < 128; idx++ )
    {
      if( directory_ptr[idx].valid == 1 )
      {
        //prints the inode information with the size first, then date, then time, then the name
        printf("%d ",inode_array_ptr[directory_ptr[idx].inode_idx]->size);
     
        date_string = (char*)malloc(sizeof(char) * 25);
        //puts 0s in the string so it will be clean
        memset(date_string,0,25);
        //copies the date into the string for printing
        strncpy(date_string, ctime(&inode_array_ptr[directory_ptr[idx].inode_idx]->date),25);
        //removes the newline
        strtok(date_string, "\n");
        printf("%s %s \n",date_string, directory_ptr[idx].name );
      }
    }
  }
}
