### Group Assignment 

## Team
<ol>
<li>Vishwanath</li>
<li>Lucas</li>
<li>Siddharth</li>
<li>Anmol</li>

<hr>
Asana.com link : <a>https://app.asana.com/share/vishw/cs631/1203202255113786/ac1cd1613952692d3cae622be0ecd424
</a>



## Our Plan
<ol>
<li>Together : Create boilerplate code for the server</li>
<li>Then devide each segment to individual members</li>
<li>If any memeber is facing difficulty then all come together to collaborate with him</li>
</ol>

Right now for "If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT" if the regex passes for the date type then it wouldn't really validate different time stamp.
  
  Test Cases For CGI
  1. Input given for -c is absolute ending with a / Ex: ./sws -c /home/z/sws/
  2. Input given for -c is absolute ending without a / Ex: ./sws -c /home/z/sws
  3. Input given for -c is relative ending with a / Ex: ./sws -c ../../../
  4. Input given for -c is relative ending without a / Ex: ./sws -c ../../..
  5. Input given in the user request is absolute GET /cgi-bin/hello.cgi HTTP/1.0
  6. Input given in the user request is absolute and ends with "/" EX: GET /cgi-bin/hello.cgi/ HTTP/1.0
  7.  6. Input given in the user request is relative GET /cgi-bin/../sws/hello.cgi/ HTTP/1.0 and ends with / -- this is a rare case and any sensible wouldn't type this so we don't serve it , try doing <code>ls happy.c/</code> it will throw an error.
  8. Input given in the user request is relative GET /cgi-bin/../sws/hello.cgi HTTP/1.0 and it stays inside the cgi-bin (fail)
  9. Input given in the user request is relative GET /cgi-bin/../../hello.cgi HTTP/1.0 and it goes outside the cgi-bin
  8.  
