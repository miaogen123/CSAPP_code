this is the tiny webserver coming from CSAPP, almost the same.
as it is, no other more functions, just parse the static html, such as:
	localhost:3380/hello.html
and specific dynamic program, such as:
	localhost:3380/cgi-bin/echo					//the program is the cgi-bin folder
if the program with arguments :
	localhost:3380/cgi-bin/echo?100&200
	

	BUGS:there is buffer overflow in the process of proceeding the headers, if
	the header is too long, i mean "accept ", "accept-encode" and the like. 
	the program goes wrong.

