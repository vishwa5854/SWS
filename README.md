### Group Assignment 

## Team
<ol>
<li>Vishwanath</li>
<li>Lucas</li>
<li>Siddharth</li>
<li>Anmol</li>

<hr>
## Asana.com link : <a>https://app.asana.com/share/vishw/cs631/1203202255113786/ac1cd1613952692d3cae622be0ecd424
</a>

<h1>Manual Page</h1>
https://stevens.netmeister.org/631/sws.1.pdf


Right now for "If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT" if the regex passes for the date type then it wouldn't really validate different time stamp.
  
  Test Cases For CGI
  1. Input given for -c is absolute ending with a / Ex: ./sws -c /home/z/sws/
  2. Input given for -c is absolute ending without a / Ex: ./sws -c /home/z/sws
  3. Input given for -c is relative ending with a / Ex: ./sws -c ../../../
  4. Input given for -c is relative ending without a / Ex: ./sws -c ../../..
  5. Input given in the user request is absolute GET /cgi-bin/hello.cgi HTTP/1.0
  6. Input given in the user request is absolute and ends with "/" EX: GET /cgi-bin/hello.cgi/ HTTP/1.0
  7.  6. Input given in the user request is relative GET /cgi-bin/../sws/hello.cgi/ HTTP/1.0 and ends with / -- this is a rare case and any sensible wouldn't type this so we don't serve it , try doint ls happy.c/ it will throw an error.
  8. Input given in the user request is relative GET /cgi-bin/../sws/hello.cgi HTTP/1.0 and it stays inside the cgi-bin
  9. Input given in the user request is relative GET /cgi-bin/../../hello.cgi HTTP/1.0 and it goes outside the cgi-bin
  8.  


Crazy Things
1. If I put a relative path in CURL it will remove all dots before making the request which
    wasted almost 2 hours of our time.
2. Telnet fails when a large CGI is being executed.
3. If we run the server as daemon and give the working directory as '.' then the daemon is resolving 
    it as / since it's working directory is at root.
4. For some apparent reason if we user flags.argument_path as server's working directory
    it is reset to null as soon as it is being passed to any function, there is no other place
    where we were editing that attribute, we have created a new variable in the struct with
    different name and assigned the same value which works flawlessly. To figure out why this is 
    not working it almost took 2 hours of hour time :(.

Credits
1. We have used command system library source code for our CGI from System V.
2. We have used online websites like geeks for geeks in very few places where we couldn't figure
    out things from manual pages.
