d# CS118 Project 1

This is the repo for spring 2022 cs118 project 1.

## Makefile

This provides a couple make targets for things.
By default (all target), it makes the `server` executables.

It provides a `clean` target, and `tarball` target to create the submission file as well.

You will need to modify the `Makefile` USERID to add your userid for the `.tar.gz` turn-in at the top of the file.

## Academic Integrity Note

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.

## Provided Files

`server.c` is the entry points for the server part of the project.

## TODO

Project 1 Group: 
    Diana Dai dianadai0426@ucla.edu 105383173
    Mihir Hasan hasanmihir123@gmail.com 305402509

High Level Design: 
The program creates a socket, binds it to the local IP address and port number. The socket starts listening and accepting new connection inside an infinite loop. When the socket accepts a new connection, it return a new file descriptor to the http request.
The program then extracts and parse the filename, extracts the file type, opens the file to create a new fd. To create the http response, the program assembles the header according to the filetype and the existence of the file. Then
the program 

Problems: 

One signficant issue we ran into was the case in which the HTTP file request had whitespaces in the filename. We were initialliy confused by this because these whitespaces were converted into their ascii codes. We addressed this issue by first tokenizing the HTTP request with strtok, and extracting the filename with the space/%20 ascii code included. We then created a helper function to parse out the ascii code and replace it with a white space to create the correct filename before opening. While using this function we ran into issues because we were returning a local pointer that wouldn't give us the parsed string we needed. We addressed this by declaring our parsed string return buffer as a global variable. 

Another issue we ran into was occasional seg faults when running on ubuntu. We addressed this by proceeding with our debugging on ubuntu. We realized we were not consistent about allocating memory for the buffers we used, and were able to remove the seg faults by allocating enough memory for every single buffer we used. 

Finally, we struggled with error handling and returning the correct 404 not found page when the HTTP request was for a file that did not exist. Although we were able to return a buffer containing the error, we wanted the user to see an html page. Since we struggled with creating this html page in line in server.c, we created a separate html file that we opened everytime a the client requested a file that did not exist. We also had to make sure to modify the other fields of the HTTP response to reflect the 404 nto found error. 
