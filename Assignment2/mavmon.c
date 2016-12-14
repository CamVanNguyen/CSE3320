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

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "train.h"

struct train_struct
{
  int id;
  int direction;
};

void trainArrives( uint32_t train_id, enum TRAIN_DIRECTION train_direction );
void trainCross  ( uint32_t train_id, enum TRAIN_DIRECTION train_direction );
void trainLeaves ( uint32_t train_id, enum TRAIN_DIRECTION train_direction );

// Current time of day in seconds since midnight
int32_t  current_time;
uint32_t clock_tick;

// Current train in the intersection
uint32_t in_intersection;

#define MAX_DIRECTIONS 4

#define MAX_TRAINS 255

//Array that holds the total trains for the direction
//the direction's count is stored in the index based 
//off of the train.h enumaration 
int trainCount[MAX_DIRECTIONS];

//Keeps the count of the consective trains passing an 
//intersection
int trainSequence[MAX_DIRECTIONS];

pthread_mutex_t intersection_mutex;
pthread_mutex_t north_mutex;
pthread_mutex_t south_mutex;
pthread_mutex_t east_mutex;
pthread_mutex_t west_mutex;

pthread_cond_t westCond; //= PTHREAD_COND_INITIALIER;
pthread_cond_t eastCond; //= PTHREAD_COND_INITIALIER;
pthread_cond_t northCond; //= PTHREAD_COND_INITIALIER;
pthread_cond_t southCond;// = PTHREAD_COND_INITIALIER;
pthread_t trains[255];

int train_num;
//Pointer to train struct
struct train_struct *ts;


void * trainLogic( void * val )
{


}


void trainLeaves( uint32_t train_id, enum TRAIN_DIRECTION train_direction )
{
  fprintf( stdout, "Current time: %d MAV %d heading %s leaving the intersection\n", 
           current_time, train_id, directionAsString[ train_direction ] );

  in_intersection = INTERSECTION_EMPTY;

  pthread_exit(NULL);
  // TODO: Handle any cleanup 
}

void trainCross( uint32_t train_id, enum TRAIN_DIRECTION train_direction )
{
  // TODO: Handle any crossing logic


  fprintf( stdout, "Current time: %d MAV %d heading %s entering the intersection\n", 
           current_time, train_id, directionAsString[ train_direction ] );

  if( in_intersection == INTERSECTION_EMPTY )
  {
    in_intersection = train_id;
  }
  else
  {
    fprintf( stderr, "CRASH: Train %d collided with train %d\n",
                      train_id, in_intersection );
    exit( EXIT_FAILURE );
  }

  // Sleep for 10 microseconds to simulate crossing
  // the intersection
  usleep( 10 * 1000000 / clock_tick );

  // Leave the intersection
  trainLeaves( train_id, train_direction );

  return;
}

void trainArrives( uint32_t train_id, enum TRAIN_DIRECTION train_direction )
{
  fprintf( stdout, "Current time: %d MAV %d heading %s arrived at the intersection\n", 
           current_time, train_id, directionAsString[ train_direction ] );

  // TODO: Handle the intersection logic
  
  ts = (struct train_struct*) malloc( sizeof(struct train_struct));
  
  ts->id = train_id;
  ts->direction = train_direction;

  pthread_create(&trains[train_num],NULL, trainLogic,(void*) ts);
  train_num++;
  return;
}

void mediate( )
{
  // TODO: MAV monitor code
}

void init( )
{
  // TODO: Any code you need called in the initialization of the application
  
  int i = 0;
 
  //setting the intial count of trains per directions 0 
  //also setting the consquentive trains to 0 
  for ( i = 0; i < MAX_DIRECTIONS; i++)
  {
    trainCount[i] = 0;
    trainSequence[i] = 0;
  }
  
  //sets the total number of trains to 0
  train_num = 0;
  
  pthread_mutex_init( &intersection_mutex, NULL);
  pthread_mutex_init( &north_mutex, NULL);
  pthread_mutex_init( &south_mutex, NULL);
  pthread_mutex_init( &east_mutex, NULL);
  pthread_mutex_init( &west_mutex, NULL);

  pthread_cond_init(&northCond,NULL);
  pthread_cond_init(&southCond,NULL);
  pthread_cond_init(&westCond,NULL);
  pthread_cond_init(&eastCond,NULL);

}



/*
 *
 *
 *  DO NOT MODIFY CODE BELOW THIS LINE
 *
 *
 *
 */

int process( )
{
  // If there are no more scheduled train arrivals
  // then return and exit
  if( scheduleEmpty() ) return 0;

  // If we're done with a day's worth of schedule then
  // we're done.
  if( current_time > SECONDS_IN_A_DAY ) return 0;

  // Check for deadlocks
  mediate( );

  // While we still have scheduled train arrivals and it's time
  // to handle an event
  while( !scheduleEmpty() && current_time >= scheduleFront().arrival_time ) 
  {

#ifdef DEBUG
    fprintf( stdout, "Dispatching schedule event: time: %d train: %d direction: %s\n",
                      scheduleFront().arrival_time, scheduleFront().train_id, 
                      directionAsString[ scheduleFront().train_direction ] );
#endif  

    trainArrives( scheduleFront().train_id, scheduleFront().train_direction );

    // Remove the event from the schedule since it's done
    schedulePop( );

  }

  // Sleep for a simulated second. Depending on clock_tick this
  // may equate to 1 real world second down to 1 microsecond
  usleep( 1 * 1000000 / clock_tick );

  current_time ++;

  return 1;
}

int main ( int argc, char * argv[] )
{

  // Initialize time of day to be midnight
  current_time = 0;
  clock_tick   = 1;

  // Verify the user provided a data file name.  If not then
  // print an error and exit the program
  if( argc < 2 )
  {
    fprintf( stderr, "ERROR: You must provide a train schedule data file.\n");
    exit(EXIT_FAILURE);
  }

  // See if there's a second parameter which specifies the clock
  // tick rate.  
  if( argc == 3 )
  {
    int32_t tick = atoi( argv[2] );
    
    if( tick <= 0 )
    {
      fprintf( stderr, "ERROR: tick rate must be positive.\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      clock_tick = tick;
    }
  }

  buildTrainSchedule( argv[1] );

  // Initialize the intersection to be empty
  in_intersection = INTERSECTION_EMPTY;

  // Call user specific initialization
  init( );

  // Start running the MAV manager
  while( process() );
 
  return 0;
}
