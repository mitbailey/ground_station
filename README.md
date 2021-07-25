# ground_station
Graphical user interface for SPACE-HAUC Ground Station operations.


### Connections Testing
As of now, by default, the Connections Manager fills in its own receiver thread in the IP Address and Port fields. This enables the operator to press 'Connect,' and the Client should connect to itself. In the Connections Manager window, a JPEG Quality field should appear. The + and - buttons can be used to send sample data over this connection. The results of this are viewable in the Linux Terminal.

### Known Issues
Freeing and Destroying of memory is not yet implemented properly.