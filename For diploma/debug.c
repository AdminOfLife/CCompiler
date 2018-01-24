#include <stdio.h>
#include <stdarg.h>
#include "globals.h"
#include "debug.h"

/* Global variables visible within this module only. */
static int gbl_debug_level = DEBUG_TRACE;

MODULE_IDENT( "@(#)debug.c	10.7");

void
gbl_debug_print( int debuglevel, char *fmt_str, ... )
{
	va_list ap;

	va_start( ap, fmt_str );

	if ( gbl_debug_level <= debuglevel )
	{
		(void)vfprintf( stdout, fmt_str, ap );
		(void)fflush( stdout );
	}
	va_end( ap );
}  /* gbl_debug_print */

int
gbl_set_debug_level( int debuglevel )
{
	FCT_IDENT();
	
	if ( debuglevel < DEBUG_ALL || debuglevel > DEBUG_NONE )
	{
		fprintf(stderr, "%s: Specified level (%d) is out-of-range", FCT_NAME, debuglevel);
		return 0;
	}
	gbl_debug_level = debuglevel;
	return 1;
}  /* gbl_set_debug_level */
