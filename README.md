# ground_station
Graphical user interface for SPACE-HAUC Ground Station operations.  
  
### Current State
The Client packages and sends ClientServerFrames over a socket connection. The outbound connection is enduced by the operator via the Connections Manager window. Currently sent and received data must be viewed through the Linux Terminal, except for ACS Update data, which will display as graphed data in the relevant window. Received ClientServerFrames are currently not parsed fully and no functionality is implemented to this end yet. Outbound ClientServerFrames undergo and integrity check before they are allowed to be sent.  

### Connections Testing  
The receive thread automatically finds and binds to the local IPv4 address, with a port of 51934. If the auto-detect fails, a default address of 127.0.0.1 is used. The receive thread will attempt to bind infinitely until it does successfully.  
  
The Connections Manager fills in its own receiver thread IPv4 address and port in the IP Address and Port fields. This enables the operator to press 'Connect,' and connect back into the client's own receiver thread. In the Connections Manager window, a JPEG Quality field should appear. The '+' and '-' buttons can be used to send sample data over this connection. The results of this are viewable in the Linux Terminal. The JPEG example should cause an integrity failure, however if data is sent using the Data-down Arrow Buttons the integrity check should return successfully.  
  
### Known Issues
None.