
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
void print_one_line(FILE *);
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
	FILE	*fp_tty;			/* stream to keyboard	*/

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
		print_one_line( fp );
		space_left--;				/* count it	*/
	}
	fclose( fp_tty );			/* disconnect keyboard	*/
	return SUCCESS;				/* EOF => done		*/
}

/*  print_one_line(fp) -- copy data from input to stdout until \n or EOF */
void print_one_line( FILE *fp )
{
	int	c;

	while( ( c = getc(fp) ) != EOF && c != '\n' )
		putchar( c ) ;
	putchar('\n');
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

int get_terminal_height(){
	int rows_cols[2];
	if(get_term_size(rows_cols)==0){
		return rows_cols[0];
	}
	return -1;
}
int get_terminal_width(){
	int rows_cols[2];
	if(get_term_size(rows_cols)==0){
		return rows_cols[1];
	}
	return -1;
}
