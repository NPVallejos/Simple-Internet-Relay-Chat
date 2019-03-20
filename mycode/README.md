# Compiling code
- you can simply type 'make' and it will compile both the server and client code

# Executing code
- run './server'
- this will then print out the servers ip address into the console (it's in the first line)
- run './client <server_ip_address>'
	- it's important that you type in the exact ip address that is printed out when you run './server'
	- ex) './client 128.226.114.202'

# What works
- It should be able to connect between separate machines
- I tested these two programs remotely (i.e. ran server on nvallej1@remote02, ran client on nvallej1@remote03, remote05, etc. and it connected to the server)
- I also tested connection between server and client on a single machine and that worked

# Limitations
- I was limited to testing my code on the remote system (nvallej1@remote.cs.binghamton.edu)
