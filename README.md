# UDP Statistics Server
Simple UDP Server, for exporting received data traffic statistics to files. Suited for Big Data pipelines or data collectors tests.  
Good for embedded systems.

Exports data traffic statistics each minute to file, allowing fast checking of data stream size.

The data gathered is: Total (KB|B) received, total messages, maximum message size, mean of messages size, KB received
per minute and per second, messages received per minute and per second.

-This data is wrote in file each minute, but if the server doesn´t receive data, will write nothing until next minute with data, 
taking this into account for calculation of stream rates.-

## Prerequisites:
- Linux System (because of C socket dependencies)

## Installing - Run:
On project folder:
- "make" to compile.
- "./sserver --port--" to run.

The text files for data exporting will be under /DataFiles/ on project folder.
