
/*  more03.c  - version 0.2 of more
 *	read and print 24 lines then pause for a few special commands
 *	v02: reads user control cmds from /dev/tty
 */
#include <stdio.h>
#include <stdlib.h>
#include "termfuncs.h"
#define  PAGELEN	24
#define  ERROR		1
#define  SUCCESS	0
#define  EXIT_PROGRAM	2
#define  has_more_data(x)   (!feof(x))
#define	CTL_DEV	"/dev/tty"		/* source of control commands	*/

int  do_more(FILE *);
int  how_much_more(FILE *);
int print_one_line(FILE *);
int get_terminal_height();
int get_terminal_width();

int main( int ac , char *av[] )
{
	FILE	*fp;			/* stream to view with more	*/
	int	result = SUCCESS;	/* return status from main	*/

	if ( ac == 1 )
		result = do_more( stdin );
	else
		while ( result == SUCCESS && --ac )
			if ( (fp = fopen( *++av , "r" )) != NULL ) {
				result = do_more( fp ) ; 
				fclose( fp );
			}
			else
				result = ERROR;
	return result;
}
/*  do_more -- show a page of text, then call how_much_more() for instructions
 *      args: FILE * opened to text to display
 *      rets: SUCCESS if ok, ERROR if not
 */
int do_more( FILE *fp )
{
	int	space_left = get_terminal_height();		/* space left on screen */
	int	reply;				/* user request		*/
	int rows_used;
	FILE	*fp_tty;			/* stream to keyboard	*/

	//error handling for get_terminal_height
	if(space_left==-1){
		return ERROR;
	}

	fp_tty = fopen( CTL_DEV, "r" );		/* connect to keyboard	*/
	while ( has_more_data( fp ) ) {		/* more input	*/
		if ( space_left <= 0 ) {		/* screen full?	*/
			reply = how_much_more(fp_tty);	/* ask user	*/
			if ( reply == 0 ){
				fclose( fp_tty );		/* disconnect keyboard	*/
				return EXIT_PROGRAM;	/* exit method and return value to stop processing all inputs	*/
			}		
			space_left = reply;		/* reset count	*/
		}
		rows_used = print_one_line( fp );
		space_left = space_left-rows_used;				/* update the number of rows used by one line of the input file. 	*/
	}
	fclose( fp_tty );			/* disconnect keyboard	*/
	return SUCCESS;				/* EOF => done		*/
}

/*  print_one_line(fp) -- copy data from input to stdout until \n or EOF or the number of rows in the terminal has been reached*/
// returns the number of rows used so to not print more than the number of rows available in the terminal
int print_one_line( FILE *fp )
{
	int	c;
	int term_width = get_terminal_width();
	int col_pos = 0;
	
	int max_rows = get_terminal_height()-1; //need to account for more at the end of the terminal.
	int rows_used = 0;

	while( ( c = getc(fp) ) != EOF && rows_used<max_rows){
		if(c== '\n'){
			if(col_pos==0){		/* check for an empty line	*/
				putchar('\n');
				rows_used++;
			}
			break;
		}
		putchar(c);
		col_pos++;

		if(col_pos>=term_width){	/* If column position exceeds terminal width, print a new line	*/
			putchar('\n');
			col_pos=0;
			rows_used++;
		}
	}

	if(col_pos>0){				/* if not a newline do a carriage return	*/
		putchar('\n');
		rows_used++;
	}

	return rows_used;

}

/*  how_much_more -- ask user how much more to show
 *      args: none
 *      rets: number of additional lines to show: 0 => all done
 *	note: space => screenful, 'q' => quit, '\n' => one line
 */
int how_much_more(FILE *fp)
{
	int	c;
	int term_height = get_terminal_height(); /* get terminal height if re-sized	*/
	int user_selection =0;

	printf("\033[7m more? \033[m");		/* reverse on a vt100	*/
	while( (c = rawgetc(fp)) != EOF )		/* get user input	*/
	{
		if ( c == 'q' )	{		/* q -> N		*/
			user_selection= 0;
			break;
		}
		if ( c == ' ' ){			/* ' ' => next page	*/
			user_selection= term_height;		/* how many to show	*/
			break;
		}
		if ( c == '\n' ){		/* Enter key => 1 line	*/
			user_selection= 1;
			break;
		}
	}
	printf("\033[2K\r"); // Clear the more prompt
	return user_selection;
}

/*
 * get_terminal_height -- returns #rows in the terminal by calling the get_term_size function
 *     args: none
 *     rets: number of rows of the terminal, -1 if error
 *     note: 
 */
int get_terminal_height(){
	int rows_cols[2];
	if(get_term_size(rows_cols)==0){
		return rows_cols[0];
	}
	return -1;
}

/*
 * get_terminal_width -- returns #columns in the terminal by calling the get_term_size function
 *     args: none
 *     rets: number of columns of the terminal, -1 if error
 *     note: 
 */
int get_terminal_width(){
	int rows_cols[2];
	if(get_term_size(rows_cols)==0){
		return rows_cols[1];
	}
	return -1;
}
