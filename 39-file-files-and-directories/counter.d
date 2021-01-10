/*
 * Count off and report the number of seconds elapsed
 */
dtrace:::BEGIN
{
	/* i = 0; */

  /* Find the max size of a char in D */
	i = (char)0; 
}

profile:::tick-100msec
{
	i = i + 1;
	trace(i);
}

dtrace:::END
{
	trace(i);
}
