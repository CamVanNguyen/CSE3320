// Code copyright Michelle Strout

#include <stdio.h> 
#include <stdlib.h>


// Function: isBitSet
// Description: Determines if a particular bit position is set
// Return: 0 if the specified bit position is not set (bit equal to 0)
// and a number other than zero if the bit position is set (bit equal to 1).
// Errors: The bit position should have a value between 0 and 31 inclusive.

unsigned int isBitSet(unsigned int bit_pattern, unsigned int bit_position)
{
  // Exit the program if the bit_position is not between 0 and 31 inclusive.
  // Don't have to check lower bound because the number is unsigned.
  if (bit_position >= 32) 
  {
    fprintf(stderr, "ERROR: bit_position is not in 0 to 31 range.");
    exit(0);
  }

  // Create a mask at the specified bit position.
  unsigned int bit_mask = 1<<bit_position;

  // Check if the specified bit is set and return a non-zero value if it is.
  // Return a zero value if the bit is NOT set.
  return bit_pattern & bit_mask;
}

int main()
{
  unsigned int bit_pattern, bit_position, bit_set;

  // Query user for the bit pattern and which bit position to check.
  printf ("Bit Set Program\n");
  printf("Enter a hexadecimal number (e.g., 0xDEADBEEF): ");
  scanf("%x", &bit_pattern); 
  printf("Enter a bit position (MSB is 31, LSB is 0): ");
  scanf("%d", &bit_position); 

  // Call function to determine if the bit is set.
  bit_set = isBitSet(bit_pattern, bit_position);

  if ( bit_set ) 
  {
   printf("In 0x%X, bit position %d is set\n",
       bit_pattern, bit_position);
  } 
  else 
  {
   printf("In 0x%X, bit position %d is NOT set\n",
           bit_pattern, bit_position);
  }
}
