#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "logio.h"

typedef struct {
  int left;        /* Left adjust field */
  int sign;        /* Prefix number with sign */
  int space;       /* Prefix number with space if no sign is to be printed */
  int zero;        /* Zero-pad to the field width with leading zeros */
  int alt;         /* Alternatate form of output */     
  int min;         /* Minimum field width */
  int prec;        /* Precision */
  int qual;        /* Qualifier character */
  int conv;        /* Conversion specification character */
} Options;

/*
 * Specify the size of the buffer used to contain the formatted output
 * associated with a single format directive. ANSI says that a conforming
 * program should not exceed 509 chars per format item. Checks will be made
 * to ensure that the buffer size is not exceeded.
 */

enum {LPBUFF_MAX=512};

static int get_int(char **form, va_list *ap, int def);
static char *get_flags(Options *opts, char *form);
static int space_pad(Logfn output, void *out, int npad);
static void ret_total(Options *opts, va_list *ap, int ntotal);
static int lperror(const char *format, ...);

/*.......................................................................
 * Function responsible for parsing the format string and sending the
 * lprintf output to a user provided output function.
 *
 * Input:
 *  output  Logfn    Pointer to function that takes the output buffer.
 *  out      void *  Pointer to output stream, output buffer or to other
 *                   information required by the particular output function
 *                   'output()'. This is the first argument to output().
 *  format   char *  The printf style format string.
 *  ap    va_list    The Argument list associated with 'format'.
 */
int lprint(Logfn output, void *out, const char *format, va_list ap)
{
  static Options def_opts = {0,0,0,0,0,0,-1,0,0};  /* Default options */
  static char buff[LPBUFF_MAX+10];                 /* Output buffer */
  static Options opts; /* Printing options */
  char *sptr;          /* Pointer to start of format item */
  char *eptr;          /* Pointer to one-past end of format item */
  int c;        /* The latest unprocessed character in the format string */
  int ntotal=0; /* The total number of characters so far written */
  int n;        /* The latest number of characters to write */
  int toobig;   /* True if formatted output would overflow the buffer */
/*
 * Sanity check.
 */
  if(format==NULL)
    return lperror("lprint: NULL printf format string intercepted\n");
/*
 * Parse the format string.
 */
  for(sptr=eptr=(char *)format; *sptr; sptr=eptr) {
/*
 * Find the next % character.
 */
    do c = *eptr++; while(c && c!='%');
/*
 * Output everything up to the %.
 */
    n = (eptr-sptr) - 1;
    if(n>0 && output(out, sptr, n))
      return -1;
/*
 * Gone off end of format string?
 */
    if(c=='\0')
      eptr--;
/*
 * Increment the count of the total number of characters so far written.
 */
    ntotal += n;
/*
 * Handle the format directive.
 */
    if(c=='%') {
      char *bufptr = &buff[0]; /* Pointer to buffer to be output */
      int dopad = 0;           /* Flags whether space padding is required */
/*
 * Keep a record of the start of the format directive (including the %).
 */
      sptr = eptr-1;
/*
 * Assign default conversion options.
 */
      opts = def_opts;
/*
 * Get the conversion flags.
 */
      eptr = get_flags(&opts, eptr);
/*
 * Get the minimum field width.
 */
      opts.min = get_int(&eptr, &ap, 0);
/*
 * Get the precision.
 */
      if(*eptr=='.') {
	eptr++;
	opts.prec = get_int(&eptr, &ap, -1);
      };
/*
 * Get any length modifier from the format string.
 */
      switch(*eptr) {
      case 'h': case 'l': case 'L':
	opts.qual = *eptr++;
      };
/*
 * Get the conversion specifier character.
 */
      opts.conv = c = *eptr++;
/*
 * Limit the size of formatted output for all but string values.
 */
      if(opts.min > LPBUFF_MAX && opts.conv!='s') {
	toobig = 1;
      } else {
	enum {MAX_FORM=80}; /* Max width of format directive */
	static char subform[MAX_FORM+1];
	toobig = 0;
/*
 * Extract a copy of the format directive.
 */
	n = eptr-sptr;
	if(n >= MAX_FORM)
	  return lperror("Format directive %s too large\n", sptr);
	strncpy(subform, sptr, (size_t) (eptr-sptr));
	subform[n] = '\0';
/*
 * The precision implies a mimimum field width for some format types.
 * Check for potential buffer overflow and write the value into the buffer.
 */
	switch(c) {
	case 'd': case 'i':  /* Plain integer */
	case 'o':            /* Octal */
	case 'x': case 'X':  /* Hex */
	case 'u':            /* Unsigned */
	  toobig = opts.prec > LPBUFF_MAX-2;
	  if(!toobig) {
	    if(opts.qual=='l')
	      sprintf(buff, subform, va_arg(ap, long));
	    else
	      sprintf(buff, subform, va_arg(ap, int));
	  };
	  break;
	case 'c':            /* Single character: Padded below */
	  buff[0] = (unsigned char) va_arg(ap, int);
	  buff[1] = '\0';
	  dopad = 1;
	  break;
	case 's':           /* String: Handled below */
	  bufptr = va_arg(ap, char *);
	  dopad = 1;
	  break;
	case 'f':           /* Plain float */
	  toobig = opts.prec > LPBUFF_MAX-2;
	  if(!toobig) {
	    if(opts.qual=='L')
	      sprintf(buff, subform, va_arg(ap, long double));
	    else
	      sprintf(buff, subform, va_arg(ap, double));
	  };
	  break;
	case 'e': case 'E': /* Floating point with exponent */
	case 'g': case 'G': /* Floating point with or without exponent */
	  toobig = opts.prec > LPBUFF_MAX-7;
	  if(!toobig) {
	    if(opts.qual=='L')
	      sprintf(buff, subform, va_arg(ap, long double));
	    else
	      sprintf(buff, subform, va_arg(ap, double));
	  };
	  break;
	case 'p':           /* Pointer (machine specific representation) */
	  sprintf(buff, subform, va_arg(ap, void *));
	  break;
	case 'n':           /* Return count of chars output so far */
	  ret_total(&opts, &ap, ntotal);
	  buff[0] = '\0';
	  break;
	case '%':           /* Plain % sign */
	  buff[0] = '%';
	  buff[1] = '\0';
	  break;
	default:
	  return lperror("lprint: Bad conversion character (%c) in format\n",
			 c);
	};
      };
/*
 * Was the formatted output too big?
 */
      if(toobig) {
	return lperror("lprint: Format \"%.10s...\" too wide for buffer\n",
		       sptr);
      };
/*
 * Determine the number of characters to be output from the buffer.
 */
      switch(opts.conv) {
      case 'n':
	n = 0;   /* Nothing to output */
	break;
      case 's':  /* Argument string */
/*
 * Work out the amount of the string to be written.
 */
	if(opts.prec < 0)
	  n = strlen(bufptr);
	else {
	  char *cptr=bufptr;
	  for(n=0; n<opts.prec && *cptr; n++,cptr++);
	};
	break;
      default:
	n = strlen(bufptr);
	break;
      };
/*
 * If a field width greater than the length of the string has been specified
 * and right-adjustment padding is required - pad to the required width
 * with spaces.
 */
      if(dopad && n<opts.min && !opts.left) {
	int npad=opts.min - n;
	if(space_pad(output, out, npad))
	  return -1;
	ntotal += npad;
      };
/*
 * Output the buffer.
 */
      if(output(out, bufptr, n))
	return -1;
      ntotal += n;
/*
 * If a field width greater than the length of the string has been specified
 * and left-adjustment padding is required - pad to the required width with
 * spaces.
 */
      if(dopad && n<opts.min && opts.left) {
	int npad=opts.min - n;
	if(space_pad(output, out, npad))
	  return -1;
	ntotal += npad;
      };
    };
  };
/*
 * Return a count of the number of characters output.
 */
  return ntotal;
}

/*.......................................................................
 * Register the format options that immediately follow a % format
 * directive.
 *
 * Input:
 *  opts     Options *   The descriptor to register the options in.
 *  form        char *   The pointer into the format string.
 * Output:
 *  return      char *   The pointer to the next un-processed character
 *                       in the format string.
 */
static char *get_flags(Options *opts, char *form)
{
/*
 * Identify all flags.
 */
  for(;;form++) {
    switch(*form) {
    case '-':
      opts->left = 1;
      break;
    case '+':
      opts->sign = 1;
      break;
    case ' ':
      opts->space = 1;
      break;
    case '0':
      opts->zero = 1;
      break;
    case '#':
      opts->alt = 1;
      break;
    default:
      return form;  /* Return the pointer to the unprocessed char */
    };
  };
}

/*.......................................................................
 * Get a number from the format specifier - if this field contains a
 * '*' then get that number from the next argument in the argument list.
 *
 * Input:
 *  form     char **  Pointer to the callers pointer into the format
 *                    string. *form will be returned pointing at the
 *                    next unprocessed char in the format string.
 *  ap    va_list *   Pointer to the argument list.
 *  def       int     The default to return if no integer is provided.
 * Output:
 *  return    int     The integer read, or -1
 *                   -1 - Error.
 */
static int get_int(char **form, va_list *ap, int def)
{
/*
 * If the next character is an asterix then get the required int from
 * the next argument.
 */
  if(**form == '*') {
    (*form)++;
    return va_arg(*ap, int);
  }
/*
 * If it is a digit then read the int from the format string.
 */
  else if(isdigit((int) **form))
    return (int) strtol(*form, form, 10);
/*
 * Otherwise no int exists.
 */
  return def;
}

/*.......................................................................
 * Return the count of characters so far written, in the next
 * argument.
 *
 * Input:
 *  opts   Options *  The format conversion options.
 *  ap     va_list *  Pointer to argument list.
 *  ntotal     int    The total number of chars written so far.
 */
static void ret_total(Options *opts, va_list *ap, int ntotal)
{
/*
 * Assign to the appropriately typed return argument.
 */
  switch(opts->qual) {
  case 'h':
    *va_arg(*ap, short *) = ntotal;
    break;
  case 'l':
    *va_arg(*ap, long *) = ntotal;
    break;
  default:
    *va_arg(*ap, int *) = ntotal;
    break;
  };
  return;
}

/*.......................................................................
 * Output a given number of space padding characters.
 *
 * Input:
 *  output  Logfn    Pointer to function that takes the output buffer.
 *  out      void *  Pointer to output stream, output buffer or to other
 *                   information required by the particular output function
 *                   'output()'. This is the first argument to output().
 *  npad      int    The number of spaces to be written.
 * Output:
 *  return    int    0 - OK.
 *                   1 - Error.
 */
static int space_pad(Logfn output, void *out, int npad)
{
  static char spaces[]="                                            ";
  static const nmax = sizeof(spaces)/sizeof(char)-1;
  int nsent;  /* Total number of characters sent */
  int nnew;   /* Number of characters to send next */
/*
 * Loop until the required number of spaces have been output, or an error
 * occurs.
 */
  for(nsent=0; npad > nsent; nsent += nnew) { 
    nnew = npad - nsent;
    if(nnew > nmax) nnew = nmax;
    if(output(out, spaces, nnew))
      return 1;
  };
  return 0;
}

/*.......................................................................
 * lprint error function.
 *
 * Input:
 *  format   const char *  The printf format string to use.
 *  ...                    Variable argument list.
 * Output:
 *  return          int    -1.
 */
static int lperror(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  return -1;
}
