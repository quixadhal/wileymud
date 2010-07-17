/*
 * yywrap is provided to quite the compiler about missing functions that
 * lex/flex generates calls to.  It should simply return 1.
 */

inline int yywrap(void);

inline int yywrap(void)
{
  return 1;
}
