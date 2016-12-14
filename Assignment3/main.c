/*
 * Name:Cam Nguyen 
 * ID #: 1000952534
 * Programming Assignment 3
 * Description: A program that take in an input file
 * and simulates the 4 page replacement strategies
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define MAX_PAGES 255

int read_file(char *file_name, int **reference_string, int *page_table_size);
void init_page_table(int **page_table, int size);
void print_page_table( int *page_table, int size);
int linear_search( int *array, int value, int size);
void fifo(int size, int *reference_string, int reference_size);
void optimal(int size, int *reference_string, int reference_size);
void least_frequently_used(int size, int *reference_string, int reference_size);
void least_recently_used(int size, int *reference_string, int reference_size);
int find_greatest_element(int size, int *array);
int find_least_element(int size, int *array);

int find_instances(int *array,int size, int value, int current_index);

int main( int argc,char *argv[]) 
{
  int *input;
  int reference_size;
  int page_size;
  int i =0; 
  int *duplicate;

  // allocates memeory for the file input reference 
  // string that will be processed through the paging  simulations 
  input =  (int*) malloc( sizeof(int) * MAX_PAGES); 
  //the filename was not given to the command line
  if(argc < 2)
  {
    printf("Please provide an input file.\n");
  }
  //The file has been provide from the command line
  else if( argc == 2)
  {
    //checks if the file exists and we have read permissions for the file
    if(access(argv[1],R_OK) == -1)
    {
      //prints a error if access was not sucessful
      perror("");
    }
    else
    {
      // stores the size of the reference string so we can reallocate 
      // the array after we read all the reference string and page table
      // information from the file
      reference_size = read_file(argv[1],&input, &page_size);
      input = realloc(input, (reference_size - 1) * sizeof(int));
      
      duplicate = malloc(reference_size * sizeof(int));
      for( i = 0; i < reference_size; i++)
      {
        duplicate[i] = input[i];
      }

      fifo(page_size, duplicate,reference_size);
      least_recently_used(page_size,duplicate,reference_size);
      least_frequently_used(page_size,duplicate,reference_size);
      optimal(page_size,duplicate,reference_size);  
  }
  }
//  free(duplicate);
  free(input);
}

int read_file(char *file_name, int **reference_string, int *page_table_size)
{
  FILE *fp = fopen( file_name, "r");  
  int count = 0;

  //reads in the table size since it will be the first item of the file
  fscanf( fp, "%d", page_table_size);
  //reads the pages while the file is not at the EOF
  while(!feof(fp))
  {
    fscanf( fp,"%d",*(reference_string)+count);
    count++;    
  }
  //closes the file
  fclose(fp);
  //returns the count to realloc the array
  return count;
}
void init_page_table(int **page_table, int size)
{
  int i = 0;
  int *temp;
  
  temp = malloc(size * sizeof(int)); 
  //checks if the size was greater than 0 for the page table to guard for bad input
  if( size > 0 ) 
  {
    //dynamically allocates the page table to the size from the file
    *page_table = malloc(size * sizeof(int));
 
    //inits all the values of the page table to -1 so we can show that it is empty
    for( i = 0; i <= size; i++)
    {
      temp[i] = -1;
    }      
    *page_table = temp;
  }
  else
  {
    printf("Page table must be a greater than zero value");
  }
}
//prints the page table values denoting it by spaces
void print_page_table(int *page_table, int size)
{
  int i = 0;
  for ( i = 0; i < size; i++)
  {
    printf("%2d ", page_table[i] );
  }
  printf("\n");
}
//searches for a value and returns its index within
//the array if it is not in the array returns a -1 
int linear_search( int *array, int value, int size)
{
  int i = 0;
  for(i = 0; i < size; i++)
  {
    if(value == array[i])
    {
        return i;
    }
  } 
  return -1;
}

void fifo( int size, int *reference_string, int reference_size)
{
  int page_fault = 0; //counter of page faults
  int i =0; //counter through the reference string
  int j = 0; // counter through the page table
  int *page_table;//holds the page table values
  int in_table;// keeps the search results
 
  //sets up the page table and assigns the initial values of -1 to mark emptiness
  init_page_table(&page_table, size);
 
  for( i = 0; i < reference_size - 1 ; i++)
  {
    //searches if the value exist in the table and stores the index if 
    //found or -1 if was not in it
    in_table = linear_search(page_table,reference_string[i],size);
    //checks if the table is full and if the element is in the page table
    if( j < size && in_table == -1  ) 
    {
      page_table[j] = reference_string[i];
      print_page_table(page_table,size);
      page_fault++;
      j++;
    }
    // resets the j to the first index and also decrements i so it doesn't skip   
    else if ( j == size)
    {
      i--;
      j = 0;
    }
    //reprints the page table because the page was already in the page table
    else if (in_table != -1 )
    {
      print_page_table(page_table,size);              
    }
  }
  printf("Page fault of FIFO: %d\n\n", page_fault);
}
//checks the next time the value will be used in the 
//reference strign and returns a -1 if it does not occur again
int find_instances(int *array,int size, int value, int current_index)
{
  int instances[size]; 
  int i = 0;
 
  for( i = 0; i < size; i++)
  {
    if(array[i] == value)
    {
      instances[i] = i;
    }
    else
    {
      instances[i] = -1;
    }
  } 

  for( i = current_index; i < size; i++)
  {

    if(instances[i] > current_index) 
    {
      return i ;
    }
 }

 return -1;
}
void optimal(int size, int *reference_string, int reference_size)
{
  int page_fault = 0;//counts the page faults
  int *page_table; //holds the pages
  int *next_reference;//holds the next reference for the page table values
  int i = 0; 
  int j = 0;
  int temp  = 0;
  int in_table = 0;   
  int index = 0;

  init_page_table(&page_table,size);  
  init_page_table(&next_reference,size);

  for( i = 0 ; i < reference_size - 1; i++) 
  {
    //checks if the page table is not full and the page is not already in the table
    in_table = linear_search(page_table,reference_string[i],size);
    if( j < size && in_table == -1)
    {
      page_table[j] = reference_string[i];
      next_reference[j] = find_instances(reference_string,reference_size,reference_string[i],i);
      print_page_table(page_table,size);
      j++;
      page_fault++;
    }
    //if the table has already been init 
    else if( j >= size &&  in_table == -1)
    {
      index = find_greatest_element(size,next_reference);
      page_table[index] = reference_string[i];
      next_reference[index] = find_instances(reference_string,reference_size,reference_string[i],i);
      print_page_table(page_table,size);
      page_fault++; 
    }
    //value already exists in the table
    else if (in_table != -1)
    {
      temp = find_instances(reference_string,reference_size,reference_string[i],i);
      index = linear_search(page_table, reference_string[i],size); 
      next_reference[index] = temp;
      print_page_table(page_table,size);
    }
  }
  //free(page_table);
  printf("Page fault of Optimal: %d\n\n", page_fault);
} 
int find_greatest_element(int size, int *array)
{  int index_highest = 0;
  int i = 0;
  int highest = array[0];
 // returns the index of the largest value or 
 // the index of a -1 because a -1 is that there 
 // will be no future references of that value  
 for( i = 0 ; i < size; i++)
 {
   if(array[i] == -1)
   {
     return i;
   }
  else if ( array[i] > highest)
   {
     index_highest =i;
     highest = array[i];
   }
 }
 return index_highest;
}
//checks for the lowest element and returns the lowest value
int find_least_element(int size, int *array)
{
  int i = 1;
  int lowest = array[0];

  for( i = 1; i < size; i++)
  {
    if( array[i] < lowest) 
    {
      lowest = array[i];
    }
  }
  return lowest;
}

void least_recently_used(int size, int *reference_string, int reference_size)
{
  int i = 0;
  int *page_table;
  int *recently_used;//holds the index of when the page value was used
  int page_fault = 0;//counts the amount of page fautls;
  int  j = 0;// counter for initializing the table with first values
  int temp = 0;
  int least_recent_index = 0;
  int  in_table = 0;//holds the index of a page if it foudn or -1 if not
 
  //sets up the page table and assigns the initial values of -1 to mark emptiness
  init_page_table(&page_table, size);
  init_page_table(&recently_used,size);
    
  for(i = 0; i < reference_size -1; i++)
  {
    in_table =  linear_search(page_table,reference_string[i],size) ;
  
    //initially fills the table with the values
    if( j < size && in_table == -1)
    {
      //updates the pagetable value with the 
      page_table[j] = reference_string[i];
      // updates the recently used table to the current index in the 
      // reference table
      recently_used[j] = i;
      print_page_table(page_table,size);
      j++;
      page_fault++;
    }
    else if ( j >= size && in_table == -1)
    {
      //scans through the array of recent indexes to find the lower index
      //which should be the oldest index and the one that will be replaced
      least_recent_index = find_least_element(size,recently_used);    
      
      //finds where that index is stored in the table
      temp = linear_search(page_table,reference_string[least_recent_index],size);
      //sets the value to the page table
      page_table[temp] = reference_string[i];
      //updates the recently used with newly used index
      recently_used[temp] = i;
      print_page_table(page_table,size);
      page_fault++;
    } 
   // the value already exists in the recently used table updates the index for that value and prints
   // the page table 
    else if ( in_table != -1)
    {
      temp = in_table;
      recently_used[temp] = i;
      print_page_table(page_table,size);
    }
  }  
  free(recently_used);
  printf("Page fault of LRU: %d\n\n", page_fault);
}

void least_frequently_used(int size, int *reference_string, int reference_size)
{
  int page_fault = 0;// count of the page faults 
  int *page_table;// holds the pages 
  int *frequency_table;//holds the count of how often a page value is used
  int i = 0;//incrementer for the page string
  int j = 0;
  int index  = 0;
  int in_table = 0;
  int temp = 0;

  init_page_table(&page_table,size);
  init_page_table(&frequency_table,size);

  for( i = 0 ; i < reference_size - 1; i++)
  {
    //looks for the value in the page table
    in_table =  linear_search(page_table,reference_string[i],size) ;
    
    //initially fills the table with the values
    if( j < size && in_table == -1)
    {
      page_table[j] = reference_string[i]; // assigns the page value
      frequency_table[j] = 1; //inits the frequency to 1
      print_page_table(page_table,size);
      j++;
      page_fault++;
    }
    //not in the table and after we initialized the table 
    else if( j >= size && in_table == -1)
    {
      //find the element with the lowest frequency
      temp = find_least_element(size,frequency_table);
      //find the index of the element
      index = linear_search(frequency_table,temp,size);
      //page at the index position to replace the lowest frequency
      page_table[index] = reference_string[i];
      // resets the count or frequency to 1
      frequency_table[index] = 1;
      print_page_table(page_table,size);
      page_fault++;
    }
    //value already exists in the table
    else if(in_table != -1)
    {
     //in_table is the index of where the page value was if not -1
     //increase the frequency count for that value 
     frequency_table[in_table] += 1;
     print_page_table(page_table,size); 
    }
  }
  free(frequency_table);
  printf("Page fault of LFU: %d\n\n",page_fault);
}
