#musthave action 

#setifnotgiven data = "-"
#setifnotgiven inlinedata = ""

#include $chunk_read

#proc processdata
action: @action
#ifspec fields
outfile: stdout
