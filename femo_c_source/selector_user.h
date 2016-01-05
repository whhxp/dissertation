/*========================================================================
  PISA  (www.tik.ee.ethz.ch/pisa/)
 
  ========================================================================
  Computer Engineering (TIK)
  ETH Zurich
 
  ========================================================================
  PISALib 

  Header file.
  
  All the things declared in this file have to be implemented by the
  user in order to get the module running.
  
  Add your own functions and variables but do not change the existing
  function declarations because these functions will be called by
  other parts of the PISALib. When implementing these functions you
  can assume that the pre-conditions hold when the function is called
  and you have to garuantee that the post-conditions hold when the
  function returns.
  
  file: selector_user.h
  author: Fabian Landis, flandis@ee.ethz.ch
  last change: 20.01.03
  
  ========================================================================
*/

#ifndef SELECTOR_USER_H
#define SELECTOR_USER_H

#define PISA_UNIX /**** replace with PISA_WIN if compiling for Windows */

/* maximal length of filenames */
#define FILE_NAME_LENGTH 128  /* change the value if you like */

/* maximal length of entries in local cfg file */
#define CFG_NAME_LENGTH 128   /* change the value if you like */


/*---| declaration of global variables (defined in selector_user.c) |-----*/

extern char *log_file; /* file to log to */

extern char paramfile[]; /* file with local parameters */

/*-----------------------------------------------------------------------*/

struct individual_t
{
     /**********| added for FEMO |**************/
     double *objective_value;
     int counter;
     /**********| addition for FEMO end |*******/
};

/*-------------------| functions for individual struct |----------------*/

int set_objective_value(individual *ind, int i, double obj_value);
/* Sets the objective_value of an individual.
   
   pre: 0 <= i <= dim - 1 (dim is the number of objectives)
        obj_value >= 0
        
   post: The objective value for the objective number 'i' in individual
         '*ind' is set to 'obj_value'.
         return value is 0 if sucessfull and 1 otherwise
*/        


individual *create_individual();
/* Allocates memory for a new individual and initializes values.

   post: returns a pointer to the allocated memory
         returns NULL if allocation failed
*/


void free_individual(individual *ind);
/* Frees the memory for one indiviual.

   post: memory for ind is freed
*/



/*-------------------------| statemachine |-----------------------------*/

int state1();
/* Do what needs to be done in state 1.

   pre: The global variable 'paramfile' contains the name of the
        parameter file specified on the commandline.
        The global variable 'alpha' contains the number of indiviuals
        you need to read using 'read_ini()'.
        The global variable 'mu' contains the number of individuals
        you need to select.
        
   post: optionally read parameter specific for the module
         optionally do some initialization
         read_ini() called
         'mu' parents selected
         undesired individuals deleted from the global population
         write_sel() called
         write_arc() called
         return value == 0 if successful,
                      == 1 if unspecified errors happened,
                      == 2 if file reading failed.

*/


int state3();
/* Do what needs to be done in state 3.

   pre: The global variable 'lambda' contains the number of indiviuals
        you need to read using 'read_var()'.
        The global variable 'mu' contains the number of individuals
        you need to select and then write to the 'sel' file.
        
   post: read_var() called
         mu parents selected
         undesired individuals deleted from the global population
         write_sel() called
         write_arc() called
         return value == 0 if successful,
                      == 1 if unspecified errors happened,
                      == 2 if file reading failed.
*/


int state5(); 
/* Do what needs to be done in state 5.

   pre: state 5 means the variator has just terminated

   post: you probably don't need to do anything
         return value == 0 if successful,
                      == 1 if unspecified errors happened,
                      == 2 if file reading failed.

*/


int state6(); 
/* Do what needs to be done in state 6.

   pre: state 6 means that the selector has to terminate.

   post: free all memory (if you want to be nice ;-)
         return value == 0 if successful,
                      == 1 if unspecified errors happened,
                      == 2 if file reading failed.
*/


int state9();
/* Do what needs to be done in state 9.

   pre: State 9 means that the variator is reset and ready to start
        again in state 0.

   post: You probably don't have to do anything, just return 0.
         return value == 0 if successful,
                      == 1 if unspecified errors happened,
                      == 2 if file reading failed.
*/


int state10(); 
/* Do what needs to be done in state 9.

   pre: State 10 means that the selector need to reset and get ready
        to start again in state 1.

   post: Get ready to start again in state 1, this includes:
         Free all memory.
         return value == 0 if successful,
                      == 1 if unspecified errors happened,
                      == 2 if file reading failed.
*/


int is_finished(); 
/* Tests if ending criterion of your algorithm applies.

   post: return value == 1 if optimization should stop
         return value == 0 if optimization should continue

   remark: Normally the variator decides when to terminate. So
           just return 0.
*/

/**********| added for FEMO |**************/

/* read local parameters from file */
int read_local_parameters();

/* select mu individuals out of new_identity (of size size)
   and return their ids */ 
int select_ind(int size, int *new_identity, int *sel_identities,
                int dimension);

/* Determines if one individual dominates another.
   Minimizing fitness values. */
int dominates(int ind_a, int ind_b, int dim);

/* Determines if two individuals are equal in all objective values.*/
int is_equal(int ind_a, int ind_b, int dim);

int irand(int range);

/* choose individual with lowest counter uniformly */
int femo_choose();

int get_counter(int id);

int increase_counter(int id);

int decrease_counter(int id);

double get_objective_value(int id, int index);

/**********| addition for FEMO end |*******/


#endif /* USER.H */
