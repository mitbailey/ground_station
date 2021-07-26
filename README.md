# ground_station
Graphical user interface for SPACE-HAUC Ground Station operations.


### Connections Testing
The receive thread automatically finds and binds to the local IPv4 address, with a port of 51934. If the auto-detect fails, a default address of 127.0.0.1 is used.  
  
As of now, by default, the Connections Manager fills in its own receiver thread in the IP Address and Port fields. This enables the operator to press 'Connect,' and the Client should connect to itself. In the Connections Manager window, a JPEG Quality field should appear. The + and - buttons can be used to send sample data over this connection. The results of this are viewable in the Linux Terminal.

### Known Issues
None.