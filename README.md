# SPACE-HAUC Ground Station GUI Client
Graphical user interface for SPACE-HAUC Ground Station operations. Intended to be used with the SPACE-HAUC Ground Station Network.

## The Network
[GUI Client](https://github.com/mitbailey/ground_station) (ground_station)

[Server](https://github.com/mitbailey/ground_station_server) (ground_station_server)

[Roof UHF](https://github.com/mitbailey/ground_station_uhf) (ground_station_uhf)

[Roof X-Band](https://github.com/mitbailey/ground_station_xband) (ground_station_xband)

[Haystack](https://github.com/mitbailey/ground_station_haystack) (ground_station_haystack)
  
## Current State
The Client packages and sends NetworkFrames over a socket connection. The outbound connection is opened by the operator via the Connections Manager window. Currently, sent and received data can be viewed either through the Linux Terminal or in the 'Plaintext RX Display' and 'ACS Update Display' windows. NetworkFrames undergo an integrity check prior to leaving and when they are received. The Client will periodically send null NetworkFrames to the Ground Station Network Server for status information on the devices connected to the Network. The other Network devices should connect to the server automatically.  

## Connections Testing  
__*2021.08.10*__



__*2021.08.04*__

Tested connection to the server with all other clients, everything connected and displayed status correctly. Versions below. 
```
GUI Client --- v1.3-alpha 
Server ------- v1.1.0-alpha
Roof UHF ----- v1.0-alpha 
Roof X-Band -- v0.1-alpha 
Haystack ----- v0.1-alpha
```
__*2021.08.03*__

Tested connection to the server with preliminary version of Roof UHF program (v1.3-alpha). Correctly receives the status of all parties, stays connected, and the Roof UHF program also stays connected. Roof UHF successfully reconnects if the server crashes, and does not crash itself trying to get the UHF Radio up and running.  

__*2021.07.29b*__  

Port connection issue fixed, see ground_station_server repository for more information.  

__*2021.07.29*__  

Failing to connect to server via port 54210, all others work. Successfully sends, receives, and displays status info.  
  
__*2021.07.27*__

Successfully connected to server (see: ground_station_server repo), sends ClientServerFrames, and receives ClientServerFrames.  
  
__*2021.07.26*__

The receive thread automatically finds and binds to the local IPv4 address, with a port of 51934. If the auto-detect fails, a default address of 127.0.0.1 is used. The receive thread will attempt to bind infinitely until it does successfully.   
  
The Connections Manager fills in its own receiver thread IPv4 address and port in the IP Address and Port fields. This enables the operator to press 'Connect,' and connect back into the client's own receiver thread. In the Connections Manager window, a JPEG Quality field should appear. The '+' and '-' buttons can be used to send sample data over this connection. The results of this are viewable in the Linux Terminal. The JPEG example should cause an integrity failure, however if data is sent using the Data-down Arrow Buttons the integrity check should return successfully.  
  
## Known Issues
- The GUI Client's X-Band infrastructure is lacking; xb_gs_test's UI and back-end will be integrated into the GUI Client once finished. 
-  

## IP & Ports
```
Server   
  IP:               TBD  
  Ports:  
    (GUI Client)    54200  
    (Roof UHF)      54210  
    (Roof X-Band)   54220  
    (Haystack)      54230  
``` 
 
  
  
    
   
  
  
Splitting into Columns~

    // static int test1 = 0, test2 = 0, test3 = 0;
    // ImGui::Columns(3, "first_column_set", true);
    // ImGui::InputInt("test1", &test1);
    // ImGui::NextColumn();
    // ImGui::InputInt("test2", &test2);
    // ImGui::NextColumn();
    // ImGui::InputInt("test3", &test3);
