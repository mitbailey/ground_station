# ground_station
Graphical user interface for SPACE-HAUC Ground Station operations.  
  
### Current State
The Client packages and sends NetworkFrames over a socket connection. The outbound connection is enduced by the operator via the Connections Manager window. Currently sent and received data can be viewed either through the Linux Terminal or in the 'Plaintext RX Display' and 'ACS Update Display' windows. Received NetworkFrames are parsed and displayed. Outbound NetworkFrames undergo an integrity check before they are allowed to be sent. The Client will periodically send NULL NetworkFrames to the Ground Station Network Server for status information on the devices connected to the Network.  

### Connections Testing  
2021.08.04  
Tested connection to the server with all other clients, everything connected and displayed status correctly. Versions below.  
GUI Client     v1.3-alpha  
Server         v1.1.0-alpha  
Roof UHF       v1.0-alpha  
Roof X-Band    v0.1-alpha  
Haystack       v0.1-alpha  
  
2021.08.03
Tested connection to the server with preliminary version of Roof UHF program (v1.3-alpha). Correctly receives the status of all parties, stays connected, and the Roof UHF program also stays connected. Roof UHF successfully reconnects if the server crashes, and does not crash itself trying to get the UHF Radio up and running.  

2021.07.29b  
Port connection issue fixed, see ground_station_server repository for more information.  

2021.07.29   
Failing to connect to server via port 54210, all others work. Successfully sends, receives, and displays status info.  
  
2021.07.27  
Successfully connected to server (see: ground_station_server repo), sends ClientServerFrames, and receives ClientServerFrames.  
  
2021.07.26  
The receive thread automatically finds and binds to the local IPv4 address, with a port of 51934. If the auto-detect fails, a default address of 127.0.0.1 is used. The receive thread will attempt to bind infinitely until it does successfully.   
  
The Connections Manager fills in its own receiver thread IPv4 address and port in the IP Address and Port fields. This enables the operator to press 'Connect,' and connect back into the client's own receiver thread. In the Connections Manager window, a JPEG Quality field should appear. The '+' and '-' buttons can be used to send sample data over this connection. The results of this are viewable in the Linux Terminal. The JPEG example should cause an integrity failure, however if data is sent using the Data-down Arrow Buttons the integrity check should return successfully.  
  
### Known Issues
None.  

### IP & Ports
Server   
  IP:               ?  
  Ports:  
    (GUI Client)    54200  
    (Roof UHF)      54210  
    (Roof X-Band)   54220  
    (Haystack)      54230   
    
    
### Repositories
ground_station  
ground_station_server  
ground_station_uhf  
ground_station_xband  
  
  
    
   
  
  
Splitting into Columns~

    // static int test1 = 0, test2 = 0, test3 = 0;
    // ImGui::Columns(3, "first_column_set", true);
    // ImGui::InputInt("test1", &test1);
    // ImGui::NextColumn();
    // ImGui::InputInt("test2", &test2);
    // ImGui::NextColumn();
    // ImGui::InputInt("test3", &test3);
