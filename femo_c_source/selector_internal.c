/*========================================================================
  PISA  (www.tik.ee.ethz.ch/pisa/)
 
  ========================================================================
  Computer Engineering (TIK)
  ETH Zurich
 
  ========================================================================
  PISALIB 
  
  Internal functions, used in selector.c
  
  C file.
  
  file: selector_internal.c
  author: Fabian Landis, flandis@ee.ethz.ch

  revision by: Stefan Bleuler, bleuler@tik.ee.ethz.ch
  last change: $date$
  
  ========================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "selector.h"
#include "selector_user.h"
#include "selector_internal.h"

/* this is needed for the wait function */
#ifdef PISA_UNIX
#include <unistd.h>
#endif

#ifdef PISA_WIN
#include <windows.h>
#endif


/*--------------------| global variable definitions |-------------------*/

/* declared in variator_internal.h used in other files as well */

char cfg_file[FILE_NAME_LENGTH_INTERNAL];  
/* 'cfg' file (common parameters) */

char ini_file[FILE_NAME_LENGTH_INTERNAL];  
/* 'ini' file (initial population) */

char sel_file[FILE_NAME_LENGTH_INTERNAL]; 
/* 'sel' file (parents) */

char arc_file[FILE_NAME_LENGTH_INTERNAL]; 
/* 'arc' file (archive) */

char var_file[FILE_NAME_LENGTH_INTERNAL]; 
/* 'var' file (offspring) */

char sta_file[FILE_NAME_LENGTH_INTERNAL]; 
/* 'sta' file (current state) */

population global_population; /* pool of all existing individuals */

/* only used in this file */

int current_max_size; 
/* starting array size of the individuals array in global_population,
   defined in selector_internal.c */

/*-------------------------| helper functions |-------------------------*/

int write_state(int state)
/* Write the state flag */
{
     FILE *fp;

     assert(0 <= state <= 11);
     
     fp = fopen(sta_file, "w");
     assert(fp != NULL);
     fprintf(fp, "%d", state);
     fclose(fp);
     return (0);
}


int read_state()
/* Read state flag */
{
     int result;
     int state = -1;
     FILE *fp;

     fp = fopen(sta_file, "r");
     if (fp != NULL)
     {
          result = fscanf(fp, "%d", &state);
          fclose(fp);
          if (result == 1) /* exactly one element read */
          {
               if (state < 0 || state > 11)
               {
                    log_to_file(log_file, __FILE__, 
                                __LINE__, "invalid state read");   
                    printf("Selector: Invalid state read from file.\n");
               }
	  }
     }
     
     return (state);
}


int state_error(int error, int linenumber)
{
     char error_message[50];
     printf("error in state %d \n", error);
     sprintf(error_message,"error in state %d \n", error);
     log_to_file(log_file, NULL, linenumber, error_message);
     exit(EXIT_FAILURE);
}


int wait(double sec)
/* Makes the calling process sleep for 'sec' seconds. */
{
#ifdef PISA_UNIX
     unsigned int int_sec;
     unsigned int usec;

     assert(sec > 0);
     
     int_sec = (unsigned int) floor(sec);
     usec = (unsigned int) floor((sec - floor(sec)) * 1e6);
     /* split it up, usleep can fail if argument greater than 1e6 */

     
     /* two asserts to make sure your file server doesn't break down */
     assert(!((int_sec == 0) && (usec == 0))); /* should never be 0 */
     assert((int_sec * 1e6) + usec >= 10000);  /* you might change this one
                                                  if you know what you are
                                                  doing */
    
     sleep(int_sec);
     usleep(usec);
#endif

#ifdef PISA_WIN
     unsigned int msec;
     assert(sec > 0);
     msec = (unsigned int) floor(sec * 1e3);
     assert(msec >= 10); /* making sure we are really sleeping for some time*/
     Sleep(msec);
#endif

     return (0);
}

/*--------------------| global population functions |------------------*/



int add_individual(int identity, double *objective_value)  
/* function to add an individual to the global population*/
{
     int i, result;
     individual **tmp; /* in case we need to double array size */
     individual *to_add;

     /* if size=0 we need to allocate memory for our population */
     if(global_population.size == 0)
     {
          current_max_size = STANDARD_SIZE;
          global_population.individual_array =
               (individual **) malloc(current_max_size * sizeof(individual*));
          
          if (global_population.individual_array == NULL)
          {
               log_to_file(log_file, __FILE__, __LINE__,
                           "selector out of memory");
               return (1);
          }       
          global_population.last_identity = -1;
     }

     if(identity >= current_max_size)
     {    
          /* enlargement of individual array (size doubling) */
          tmp = (individual **) malloc(sizeof(individual*) *
                                        current_max_size * 2);
          if (tmp == NULL)
          {
               log_to_file(log_file, __FILE__, __LINE__,
                           "selector out of memory");
               return (1);
          } 
          /* copy old array */
          for (i = 0; i < current_max_size; i++)
               tmp[i] = global_population.individual_array[i];
          current_max_size = current_max_size * 2;
          /* free memory */ 
          free(global_population.individual_array);
          global_population.individual_array = tmp;
     }
     /* create individual */
     to_add = create_individual();
     if(to_add == NULL)
     {
          log_to_file(log_file, __FILE__, __LINE__,
                      "create_individual failed");
          return (1);
     }
     global_population.individual_array[identity] = to_add;
     /* copy objective values */
     for (i=0; i < dimension; i++)
     { 
          result = set_objective_value(to_add, i, objective_value[i]);
          if(result != 0)
          {
               log_to_file(log_file, __FILE__, __LINE__, "copy obj failed");
          } 
     } 

     if(global_population.last_identity < identity)
          global_population.last_identity = identity;

     global_population.size++;
     return (0);
}



int clean_population()
/* Frees memory for all individuals in population and for the global
   population itself. */
{
     int current_id;

     if (NULL != global_population.individual_array)
     {
        current_id = get_first();
        while(current_id != -1)
        {
           remove_individual(current_id);
           current_id = get_next(current_id);
        }

        free(global_population.individual_array);
        
        global_population.individual_array = NULL;
        global_population.size = 0;
        global_population.last_identity = -1;
     }
     
     return (0);
}


/*-------------------------| other functions |-------------------------*/

int read_common_parameters()
/* reads global parameters from cfg file */
{
     FILE *fp;
     int result; /* storing return values of called functions */
     char str[CFG_ENTRY_LENGTH_INTERNAL]; 
          
     /* reading cfg file with common configurations for both parts */
     fp = fopen(cfg_file, "r");
     assert(fp != NULL);     
 
     fscanf(fp, "%s", str);
     assert(strcmp(str, "alpha") == 0);
     fscanf(fp, "%d", &alpha);
     assert(alpha > 0);
     
     fscanf(fp, "%s", str);
     assert(strcmp(str, "mu") == 0);
     fscanf(fp, "%d", &mu);
     assert(mu > 0);
     
     fscanf(fp, "%s", str);
     assert(strcmp(str, "lambda") == 0);
     fscanf(fp, "%d", &lambda);
     assert(lambda > 0);

     result = fscanf(fp, "%s", str);
     assert(strcmp(str, "dim") == 0);
     result = fscanf(fp, "%d", &dimension);
     assert(result != EOF); /* no EOF, dim correctly read */
     assert(dimension > 0);
     
     fclose(fp);     
     return (0);
}


int check_sel()
/* Returns 0 if 'sel_file' contains only '0'and returns 1 otherwise. */
{
     int control_element = 1;

     FILE *fp;

     fp = fopen(sel_file, "r");
     assert(fp != NULL);
     fscanf(fp, "%d", &control_element);
     fclose(fp);
     
     if (0 == control_element)
          return (0); /* file is ready for writing */
     else
          return (1); /* file is not ready for writing */
}

int check_arc()
/* Returns 0 if 'arc_file' contains only '0'and returns 1 otherwise. */
{
     int control_element = 1;

     FILE *fp;

     fp = fopen(arc_file, "r");
     assert(fp != NULL);
     fscanf(fp, "%d", &control_element);
     fclose(fp);
     
     if (0 == control_element)
          return (0); /* file is ready for writing */
     else
          return (1); /* file is not ready for writing */
}
