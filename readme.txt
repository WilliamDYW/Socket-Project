a. Dongyang Wu

b. 3956874874

c. All Phases(1-4) and the functionality for extra credits are done.

d. The code files are:
client.c: send authentication & query requests to Main Server, and receive the results.
serverM.c: the Main Server, process then send and receive data to/from clients and other servers.
serverC.c: the Credential Server, authenticate the username and password and send result to Main Server.
serverCS/EE.c: the Server for CS/EE subject, find information of given courses and senf result to Main Server.
functions.c/functions.h: some functions & variables that will be used by multiple programs will be in function.c & function.h.

e. The format of all the messages exchanged:
client -> Main Server
	1. Authentication: 'X' + "username" + ',' + "password"; 'X' = 'A' + Attempts Remaining (e.g., 'C' -> 1st attempt, 'A' -> 3rd attempt)
	e.g.: "Cjames,2kAnsa7s)"
	2. Multiple-Course Query: 	'X' + "username" + ' ' + "subject indicator" + ' ' + course numbers * "course code without subject";
				'X' = 'Q' + course numbers; (e.g., 'S' -> 2 courses, 'T' -> 3 courses)
				"subject indicator": e.g., "100" means that 1st course is CS course, while 2nd & 3rd course is EE course.  
	e.g.: "Tjames 100 356 450 658" (Request for CS356, EE450 and EE658)
	3. Single-Course Query with Categories:	'X' + "username" + ' ' + "subject indicator" + ' ' + "course code without subject"
					'X' = 'Q' - category index (e.g. Credit -> 0, Days -> 2)
	e.g.: "Qjames 0 450" (Request for the Credit of EE450)
Main Server -> ServerC
	Authentication: "username(encrypted)" + ',' + "password(encrypted)"
	e.g.: "neqiw,6oErwe1w)"
ServerC -> Main Server (will be the same when Main Server -> client)
	Authentication: "0" means passed, "1" means password is incorrect, "2" means username does not exist.
Main Server -> ServerCS/EE
	Query Request for all information: "course code without subject"
	e.g.: "450" (Request for all information of EE/CS450)
	Query Request for specific category: "course code without subject" + 'X'; 'X' = 'A' + category index (e.g. Credit -> 0, Days -> 2)
	e.g.: "450A" (Request for the Credit of EE/CS450)
ServerCS/EE -> Main Server (will be the same when Main Server -> client)
	Information of a specific category: "The <category> of <course> is <...>"
	e.g. "The Credit of EE450 is 4"
	Information of the entire class: "<course>:<...>"
	e.g. "EE450:4,Ali Zahid,Tue;Thu,Introduction to Computer Networks"
	Information of non-existing course: "Didn't find the course: <course>"
	e.g. "Didn't find the course: EE100"

g. Idiosyncrasy:
The system makes the following assumptions:
	1. There are no ',' characters in username or password. Otherwise, it may lead to false results of authentication.
	2. All course codes end with a digit (e.g. No "EE450L"). Otherwise, the course will be ignored.
	3. All servers must be on when the client is sending data. Otherwise, there may be some unexpected faults.
The system makes the following responses towards other undefined behaviors:
	1. In Query Phase, if a part of input is not in correct format(e.g., "AB123" or "EE33ffff"), this part of input will be completely ignored.
	e.g., inputting "EE450 AB123" will be considered 1 course only; thus, the user will be prompted to enter the category of query.
	2. When the user enters a non-existing category of query, the system will display all information (just as Multiple-Course Query, but one course only)
	3. When the username or password input is less than 5 characters, the system will consider them wrong without authentication.
	If the username (length > 5) does not exist while the password input is less than 5 characters, the system will return "Password does not match".
	4. When the username or password input is more than 50 characters, only the first 50 characters will be considered valid.
	5. When the course code input is more than 60 characters, only the first 60 characters will be considered valid.
	6. When the query category input is more than 10 characters, only the first 10 characters will be considered valid.
The Main Server of the system will input extra messages on Multiple-Course Query.
The system is using assert() to find abnormal behaviors.

h. Reused Code:
1. In functions.c, the function open_socket() is mainly copied from Beej's Guide to Network Programming, and is slightly modified from client.c, server.c, listener.c and talker.c.
2. Line 19-24 and Line 83-111 of serverM.c are directly copied from Beej's Guide to Network Programming, from server.c
3. All send() and recv() functions are modified from Beej's Guide to Network Programming, from server.c & client.c