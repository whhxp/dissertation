/*========================================================================
  PISA  (www.tik.ee.ethz.ch/pisa/)
 
  ========================================================================
  Computer Engineering (TIK)
  ETH Zurich
 
  ========================================================================
  PISALIB 
  
  Pisa basic functions for use in selector_user.c
  
  C file.
  
  file: selector.c
  author: Fabian Landis, flandis@ee.ethz.ch

  revision by: Stefan Bleuler, bleuler@tik.ee.ethz.ch
  last change: $date$
  
  ========================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "selector.h"
#include "selector_user.h"
#include "selector_internal.h"


/*--------------------| global variable definitions |-------------------*/

/* declared in variator.h used in other files as well */

int alpha; /* number of individuals in initial population */

int mu; /* number of individuals selected as parents */

int lambda; /* number of offspring individuals */

int dimension; /* number of objectives */

/*-------------------------| main() |-----------------------------------*/

int main(int argc, char *argv[])
{
     int returncode; /* storing the values that the state functions return */
     int current_state = 0;


     char filenamebase[FILE_NAME_LENGTH_INTERNAL]; /* filename base,
                                                      e.g. "dir/test." */

     double poll; /* polling interval in seconds */
     
     if (argc == 4)
     {
          sscanf(argv[1], "%s", paramfile); /* paramfile defined in
                                             * selector_user.h */
          sscanf(argv[2], "%s", filenamebase);
          sscanf(argv[3], "%lf", &poll);
          assert(poll >= 0);
     }
     else
     {
          printf("Selector - wrong number of arguments\n");
          return (1);
     }  
     
     /* generate file names based on 'filenamebase'*/
     sprintf(var_file, "%svar", filenamebase);
     sprintf(sel_file, "%ssel", filenamebase);
     sprintf(cfg_file, "%scfg", filenamebase);
     sprintf(ini_file, "%sini", filenamebase);
     sprintf(arc_file, "%sarc", filenamebase);
     sprintf(sta_file, "%ssta", filenamebase);

     /* initialize global_population (just in case we terminate
        before the population is set up.) */

     global_population.individual_array = NULL;
     
     
     /* state machine: uses the stateX() functions to do the steps required
        in state X. Those state functions have to be implemented by 
        the user in selector_user.c */

     while (current_state != 6) /* stop state for selector */
          /* Caution: if reading of the statefile fails
             (e.g. no permission) this is an infinite loop */
     {
          current_state = read_state();
          if (current_state == 1) /* inital selection */
          { 
               read_common_parameters();
               
               returncode = state1();
               if (returncode == 0)
               {
                    current_state = 2;
                    write_state(current_state);
               }
               else if (returncode != 2)
               {
                    state_error(1, __LINE__);
                    
               }
          }
          
          else if (current_state == 3) /* selection */
          {
               if (check_sel() == 0 && check_arc() == 0)
               {
                    returncode = state3();
                    if (returncode == 0)
                    {
                         current_state = 2;
                         write_state(current_state);
                    }
                    else if (returncode != 2)
                    {
                         state_error(3, __LINE__);
                    } /* else don't do anything and wait again */ 
               }
          }
          
          else if (current_state == 5) /* variator just terminated,
                                          here you can do what you want */
          {
               returncode = state5();/* e.g., terminate too */
               if (returncode == 0)
               {
                    current_state = 6;
                    write_state(current_state);
               }
               else if (returncode != 2)
               {
                    state_error(5, __LINE__);
               } /* else don't do anything and wait again */ 

          }
      
          else if (current_state == 9) /* variator ready for reset,
                                          here you can do what you want */
          {
               returncode = state9();/* e.g., get ready for reset too */
               if (returncode == 0)
               {
                    current_state = 10;
                    write_state(current_state);
               }
               else if (returncode != 2)
               {
                    state_error(9, __LINE__);
               } /* else don't do anything and wait again */ 
          }
      
          else if (current_state == 10) /* reset */
          {
               returncode = state10();
               if (returncode == 0)
               {
                    clean_population();
                    current_state = 11;
                    write_state(current_state);
               }
               else if (returncode != 2)
               {
                    state_error(10, __LINE__);
               } /* else don't do anything and wait again */ 
          }
      
          else /* state == -1 (reading failed) or state concerns variator */
          {
               wait(poll);
          }
     } /* state == 6 (stop) */
  
     returncode = state6();
     if (returncode == 0)
     {
          clean_population();
          current_state=7;
          write_state(current_state);
     }
     else
          state_error(6, __LINE__);
  
     return (0);
}

/*-------------------------| populations functions |--------------------*/


/* function takes an ID and returns a pointer to the corresponding
   individual */
individual *get_individual(int identity) 
{
     if((identity > global_population.last_identity) || (identity < 0))
          return (NULL);
     return (global_population.individual_array[identity]);
}


/* returns the identity of the first individual in the global population */
int get_first() 
{
     return (get_next(-1));
}


/* takes an identity and returns identity of the next individual in the
   global population*/
int get_next(int identity)  
{
     int next_id;
     /* check for valid id */
     if((identity < -1) || (identity > global_population.last_identity))
          return (-1); 
     next_id = identity + 1;
     while(next_id <= global_population.last_identity)
     {
          /* if pointer is not NULL it is a valid individual */
          if(global_population.individual_array[next_id] != NULL)
               return (next_id);
          next_id++;
     }
     /* no individual found in whole population, so return -1 */
     return (-1);
}


/* get size of population */
int get_size()  
{
     return (global_population.size);
}


/* Removes individual with ID 'identity' from the global population.
   If successful returns 0, and 1 otherwise. */
int remove_individual(int identity) 
{
     int last_id; 
     individual *temp;
     
     /* check for valid id */
     if(identity < 0 || identity > global_population.last_identity)
          return (1);

     temp = global_population.individual_array[identity];
     /* if individual with given id doesn't exist */
     if(temp == NULL)
          return (1);

     global_population.individual_array[identity] = NULL;
     /* free individual */
     free_individual(temp);
     /* if the id was the highest one we decrease the last id */
     if(identity >= global_population.last_identity)
     {
          global_population.last_identity--;
          last_id = global_population.last_identity;     
          while(last_id > -1 &&
                global_population.individual_array[last_id] == NULL)
          {
               global_population.last_identity--;
               last_id = global_population.last_identity;
          }	      
     }
     global_population.size--;
     return (0);
}


/*-------------------------| io |---------------------------------------*/


int read_ini(int *id_array)
/* Reads individuals from var file and updates the global population.
   The IDs of the individuals are stored in 'id_array'.  The
   number of IDs in this array is alpha. 'id_array' must be big
   enough to store alpha 'int' variables.

   If reading is successful function returns 0, otherwise it returns
   1. */
{
     int i, j, size;
     char tag[4];
     FILE *fp;
     int result; /* stores return value of called functions */
     int identity;
     double *objective_value;

     objective_value = (double *) malloc(dimension * sizeof(double));
     if (objective_value == NULL)
     {
          log_to_file(log_file, __FILE__, __LINE__,
                      "Selector out of memory.");
          exit(EXIT_FAILURE);
     }
     
     
     fp = fopen(ini_file, "r");
     assert(fp != NULL);

     assert(id_array != NULL);
     
     fscanf(fp, "%d", &size);
     /* test if size has a valid value */
     if (size != ((dimension + 1) * alpha))
     {
          log_to_file(log_file, __FILE__, __LINE__, 
                      "size in ini file is wrong");
          free(objective_value);
          fclose(fp);
          return (1);
     }
     
     for(j = 0; j < alpha; j++)
     {
          /* reading index of individual */
          result = fscanf(fp, "%d", &identity); /* fscanf() returns EOF
                                                   if reading fails.*/
          if (result == EOF) /* file not completely written */
          {
               free(objective_value);
               fclose(fp);
               return (1); /* signalling that reading failed */
          }
          id_array[j] = identity;
          for (i = 0; i < dimension; i++)
          {
               /* reading fitness values of ind */
               result = fscanf(fp, "%le", &objective_value[i]);
               if (result == EOF) /* file not completely written */
               {
                    free(objective_value);
                    fclose(fp);
                    return (1); /* signalling that reading failed */
               }
          }
          
          /* adding individual */
          result = add_individual(identity, objective_value);
          if (result != 0)
          {
               free(objective_value);
               fclose(fp);
               return (1);
          }
          
     }
     
     fscanf(fp, "%s", tag);
     if (strcmp(tag, "END") != 0)
     {
          free(objective_value);
          fclose(fp);
          return (1);  /* signalling that reading failed */
     }
     else /* "END" ok */
     {
          fclose(fp);

          /* deleting content */
          fp = fopen(ini_file, "w");
          assert(fp != NULL);
          fprintf(fp, "%d", 0);
          fclose(fp);

          free(objective_value);
          return (0);  
     }    
}


int read_var(int *id_array)
/* Reads individuals from var file and updates the global population.
   The IDs of the individuals are stored in 'id_array'.  The
   number of IDs in this array is lambda. 'id_array' must be big
   enough to store lambda 'int' variables.

   If reading is successful function returns 0, otherwise it returns
   1. */
{
     int i, j, size;
     char tag[4];
     FILE *fp;
     int result;
     int identity;
     double *objective_value;
     
     objective_value = (double *) malloc(dimension * sizeof(double));
     if (objective_value == NULL)
     {
          log_to_file(log_file, __FILE__, __LINE__,
                      "Selector out of memory.");
          exit(EXIT_FAILURE);
     }
     fp = fopen(var_file, "r");
     assert(fp != NULL);

     
     assert(id_array != NULL);
     
     fscanf(fp, "%d", &size);
     /* test if size has a valid value */
     if (size != ((dimension + 1) * lambda))
     {
          log_to_file(log_file, __FILE__, __LINE__,
                      "size in var file is wrong");
          free(objective_value);
          fclose(fp);
          return (1);
     }

     for(j = 0; j < lambda; j++)
     {
          /* reading index of individual */
          result = fscanf(fp, "%d", &identity); /* fscanf() returns EOF
                                                   if reading fails.*/     
          id_array[j] = identity;
          for (i = 0; i < dimension; i++)
          {
               /* reading fitness values of ind */
               result = fscanf(fp, "%le", &objective_value[i]);
               if (result == EOF) /* file not completely written */
               {
                    free(objective_value);
                    fclose(fp);
                    return (1); /* signalling that reading failed */
               }
          }
          /* adding individual */
          result = add_individual(identity, objective_value);
          if (result != 0)
          {
               free(objective_value);
               fclose(fp);
               return (1);
          }
     }
     

     fscanf(fp, "%s", tag);
     if (strcmp(tag, "END") != 0)
     {
          free(objective_value);
          fclose(fp);
          return (1);  /* signalling that reading failed */
     }
     else /* "END" ok */
     {
          fclose(fp);

          /* deleting content */
          fp = fopen(var_file, "w");
          assert(fp != NULL);
          fprintf(fp, "%d", 0);
          fclose(fp);

          free(objective_value);
          return (0);
     }   
}



int write_sel(int *identity) 
/* Takes an array of mu IDs and writes these IDs into the sel file.
   Returns 0 if successful and 1 otherwise */
{
     FILE *fp;
     int i;
     int min_valid, max_valid;

     if(identity == NULL)
          return (1);
     /* test if identities are valid */
     min_valid = 0;
     max_valid = global_population.last_identity;
     for(i = 0; i < mu; i++)
     {
          if (identity[i] < min_valid || identity[i] > max_valid 
              || global_population.individual_array[identity[i]] == NULL) 
          {
               log_to_file(log_file, __FILE__,
                           __LINE__, "bad id, checked in write_sel");
               return (1);
          } 
     }

     fp = fopen(sel_file, "w");
     assert(fp != NULL);
     fprintf(fp, "%d\n", mu);  
     for (i = 0; i < mu; i++)
     {
          fprintf(fp, "%d", identity[i]);
          fprintf(fp,"\n");
     }
     fprintf(fp, "END");
     fclose(fp);
     return (0);
}


int write_arc()  
/* Writes all inidviduals in global population to arc file.
   Returns 0 if successful and 1 otherwise */
{
     FILE *fp;
     int identity;
     fp = fopen(arc_file, "w");
     assert(fp != NULL);
     fprintf(fp, "%d\n", global_population.size);  
     identity = get_first();
     while (identity != -1)
     {
          fprintf(fp, "%d", identity);
          fprintf(fp, "\n");   
          identity = get_next(identity);
     }
     fprintf(fp, "END");
     fclose(fp);
     return (0);
}



int log_to_file(char *file, char *infile, int linenumber, char *string)
/* Can be used for debugging purpose.  Writes the following to the
   logfile 'file':
   - current date and time
   - 'infile': the name of the source file which produced the error
   - 'linenumber'
   - 'string': the error message.
   If 'file == NULL' the same information is written to stderr.
   'infile' can be specified as 'NULL' and 'linenumber' can be set to
   -1 in order to leave them out.
*/
{
     FILE *fp;
     struct tm   *curr_date;
     char date_string[64];
     time_t now = time(NULL);
     curr_date = (struct tm*)localtime(&now);
     if (curr_date != NULL)
     {
          strftime(date_string, sizeof (date_string), "%d.%m.%Y %H:%M:%S",
                   curr_date);
     }
     else /* no data available */
     {
          strcpy(date_string, "no date");
     }
     
     if(file != NULL)
     {
          fp= fopen(file, "a");
          assert(fp != NULL);

          if(infile != NULL && linenumber != -1)
          { 
               fprintf(fp, "%s in file %s at line %d: %s\n", date_string,
                       infile, linenumber, string);
          }
          else if(infile != NULL && linenumber == -1)
          {
               fprintf(fp, "%s in file %s: %s\n", date_string, infile,
                       string);
          }
          else if(infile == NULL && linenumber != -1)
          {
               fprintf(fp, "%s at line %d: %s\n", date_string, linenumber,
                       string); 
          } 
          else
          {
               fprintf(fp, "%s: %s\n", date_string, string); 
          }

          fclose(fp);
     }
     else
     {
          fprintf(stderr, "%s at line %d: %s\n", 
                  date_string, linenumber, string);
     }
     return (0);
}
