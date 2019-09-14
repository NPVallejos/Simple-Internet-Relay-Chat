# What this project is about
I made a simple IRC that follows the TCP.<br/>
The purpose of this project was to learn about network programming while building a fun little chat system that works in terminal.

# Compiling code (On a linux machine)
Type 'make' and it will compile both the server and client code

# Executing code
1. Open a bash terminal.<br/>
2. Type './server'...(This will then print out the servers ip address into the console (it's in the first line))<br/>
3. Open another bash terminal.<br/>
4. Type './client <server_ip_address>'<br/>
Note: it's important that you type in the exact ip address that is printed out when you run './server'...<br/>
ex) './client 128.226.114.202'

# What works
- It should be able to connect between separate machines
- I tested these two programs remotely (i.e. ran server on nvallej1@remote02, ran client on nvallej1@remote03, remote05, etc. and it connected to the server)
- The server can handle multiple clients at once
- I also tested connection between server and client on a single machine and that worked

# Limitations
- I was limited to testing my code on the remote system (nvallej1@remote.cs.binghamton.edu)
