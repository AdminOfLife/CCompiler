#ifndef IFS_DEBUG_INC           /* Protect against multiple inclusion */
#define IFS_DEBUG_INC

//#define DEBUG_BUILD #if defined(_DEBUG) || defined(__DEBUG__)

#if defined(_DEBUG) || defined(__DEBUG__)
#define debug_print printf
#define HEADER header
#define DEBUG_INIT() char header[256]; sprintf(header, "%-25.25s%-25s", "TOKEN", "TOKEN_TYPE"); printf("%s\n", HEADER)
#define DEBUG_TOKEN() printf("%-25s%-25s\n", token, get_type_desc(token_type))
#else
#define debug_print
#endif

#define FCT_NAME        fct_name
#define MOD_NAME        mod_name
#define MODULE_IDENT(m) static const char *mod_name = m
#define FCT_IDENT()		const char *FCT_NAME=__FUNCTION__; \
                        gbl_debug_print(DEBUG_TRACE, \
                                        "=>%s (%s)\n", FCT_NAME, MOD_NAME)
#define END_FCT() gbl_debug_print(DEBUG_TRACE, \
                                        "<=%s (%s)\n", FCT_NAME, MOD_NAME)

/*
** The following four macros are for use with both gbl_set_debug_level()
** and gbl_debug_print().
*/
#define DEBUG_TRACE     1       /* low-level control-flow                 */
#define DEBUG_DEVELOP   2       /* contents of variables, etc.            */
#define DEBUG_TEST      3       /* information to be used for testing     */
#define DEBUG_PROD      4       /* production mode messages               */

/* The following two macros are ONLY for use with gbl_set_debug_level(). */
#define DEBUG_ALL       0       /* all debugging messages are displayed   */
#define DEBUG_NONE      99      /* no debugging output requested          */
                                        
/* Function prototypes */
int   gbl_set_debug_level(int debuglevel);
int   gbl_get_debug_level(void);
int   gbl_set_debug_via_env(void);
void  gbl_debug_print(int debuglevel, const char *fmt_str, ...);
void  gbl_debug_trace( const char *str );
int   xprintf_generic(int debuglevel, unsigned char *buf, int len, int type);
int   xprintf(unsigned char *buf, int len);
int   xprintf_safe( unsigned char *buf, int len );
int   xprintf_ascii(int debuglevel, unsigned char *buf, int len);
int   dump_buffer(int debug_level, unsigned char *buf, int len, int mode);

#endif /* IFS_DEBUG_INC */

