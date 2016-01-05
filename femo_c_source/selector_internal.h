/*========================================================================
  PISA  (www.tik.ee.ethz.ch/pisa/)
 
  ========================================================================
  Computer Engineering (TIK)
  ETH Zurich
 
  ========================================================================
  PISALIB 

  Helper functions used by main and functions in selector.c
   
  Header file.
  
  file: selector_internal.h
  author: Fabian Landis, flandis@ee.ethz.ch
  
  revision by: Stefan Bleuler, bleuler@tik.ee.ethz.ch
  last change: $date$
  
  ========================================================================
*/

#ifndef SELECTOR_INTERNAL_H
#define SELECTOR_INTERNAL_H

/*-------------------------| constants |--------------------------------*/


#define FILE_NAME_LENGTH_INTERNAL 128
/* maximal length of filenames */


#define CFG_ENTRY_LENGTH_INTERNAL 128 
/* maximal length of entries in cfg file */


#define STANDARD_SIZE 32200  
/* Start with array of this size for global population */

/*---------------| declaration of global variables |-------------------*/

/* file names - defined in selector_internal.c */

extern char cfg_file[];  
/* 'cfg' file (common parameters) */

extern char ini_file[];  
/* 'ini' file (initial population) */

extern char sel_file[]; 
/* 'sel' file (parents) */

extern char arc_file[]; 
/* 'arc' file (archive) */

extern char var_file[]; 
/* 'var' file (offspring) */

extern char sta_file[]; 
/* 'sta' file (current state) */


/*-------------------| functions for handling states |------------------*/

int write_state(int state);
/* Write state flag. */

int read_state(void);
/* Read state flag. */

int state_error(int error, int linenumber);
/* Output an error message. */

int wait(double sec);
/* Makes the calling process sleep for 'sec' seconds. */

/*-------------------------| global population |------------------------*/

/* pool of all existing individuals */
typedef struct population_t 
{
     int size;        /* size of the population */
     individual **individual_array; /* array of individuals */
     int last_identity; /* largest identity- needed for memory management */
} population;

/* the only population we need is */
extern population global_population; /* defined in selector_internal.c */


int add_individual(int identity, double *objective_value);
/* Adds an individual to the global population and sets
   the objective values. */  

int clean_population(void);
/* Frees memory for all individuals in population and for the global
   population itself. */


/*-------------------------| other functions |-------------------------*/

int read_common_parameters(void);
/* Reads global parameters from 'cfg' file. */

int check_sel(void);
/* Returns 0 if 'sel_file' contains only '0'and returns 1 otherwise. */

int check_arc(void);
/* Returns 0 if 'arc_file' contains only '0'and returns 1 otherwise. */

#endif /* SELECTOR_INTERNAL.H */
