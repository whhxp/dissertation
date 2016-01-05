/*========================================================================
  PISA  (www.tik.ee.ethz.ch/pisa/)
 
  ========================================================================
  Computer Engineering (TIK)
  ETH Zurich
 
  ========================================================================
  PISALIB 
  
  Pisa basic functions for use in selector_user.c
  
  Header file.
  
  file: selector.h
  author: Fabian Landis, flandis@ee.ethz.ch
 
  revision by: Stefan Bleuler, bleuler@tik.ee.ethz.ch
  last change: $date$

  ========================================================================
*/

#ifndef SELECTOR_H
#define SELECTOR_H

/*----------------------| common parameters |---------------------------*/

/* defined in selector.c */

extern int alpha; /* number of individuals in initial population */

extern int mu; /* number of individuals selected as parents */

extern int lambda; /* number of offspring individuals */

extern int dimension; /* number of objectives */

/*-------------------------| individual |-------------------------------*/

typedef struct individual_t individual; 
/* individual has to be defined in selector_user.h */

/*-------------------------| global population |------------------------*/

int remove_individual(int identity);
/* Removes individual with ID == identity from global population.
   Returns 0 if successful and 1 otherwise. */


individual *get_individual(int identity);
/* Returns pointer to individual corresponding to 'identity'. */


int get_first(); 
/* Returns identity of first individual in the global population. */


int get_next(int identity); 
/* Takes an identity an returns the identity of the next following
    individual in the global population. */


int get_size();
/* Returns the size of the global population. */

/*-------------------------| io |---------------------------------------*/

int read_ini(int *id_array);
/* Reads individuals from var file and updates the global population.
   The IDs of the individuals are stored in 'id_array'.  The
   number of IDs in this array is alpha. 'id_array' must be big
   enough to store alpha 'int' variables.

   If reading is successful function returns 0, otherwise it returns
   1. */


int read_var(int *id_array); 
/* Reads individuals from var file and updates the global population.
   The IDs of the individuals are stored in 'id_array'.  The
   number of IDs in this array is lambda. 'id_array' must be big
   enough to store lambda 'int' variables.

   If reading is successful function returns 0, otherwise it returns
   1. */


int write_sel(int *identity);
/* Takes an array of mu IDs and writes these IDs into the sel file.
   Returns 0 if successful and 1 otherwise. */


int write_arc();  
/* Writes all inidviduals in global population to arc file.
   Returns 0 if successful and 1 otherwise. */



int log_to_file(char *file, char *infile, int linenumber, char *string);
/* can be used for debugging purpose.  Writes the following to the
   logfile 'file':
   - current date and time
   - 'infile': the name of the source file which produced the error
   - 'linenumber'
   - 'string': the error message.
   If 'file == NULL' the same information is written to stderr.
   'infile' can be specified as 'NULL' and 'linenumber' can be set to
   -1 in order to leave them out. */


#endif /* SELECTOR.H */
