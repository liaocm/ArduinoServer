# ArduinoServer
A light weight server on Arduino boards with WiFi 101 shields that handles GET requests with arbitrary path and parameters.

## Requirements
- Arduino WiFi 101 Shield
- Arduino boards (preferably Mega 2560)

## Usage
Edit the SSID and password fields
```
char ssid[] = "YOUR SSID HERE";
char pass[] = "YOUR PASSWORD HERE";
```
in the code before compiling and uploading to your board. If your network encryption is not WPA2 Personal, you can modify the code according to [Arduino WiFi 101 Reference](https://www.arduino.cc/en/Reference/WiFi101). You may also want to edit these functions

```
void sendResponseByFd(int fd, WiFiClient client);

int fileDescriptorForRequest(char *path, 
                             unsigned char num_params,
                             char **param_names,
                             char **param_vals);
                          
```
in lw-server.ino and

```
void sendResponseHeader(int code, WiFiClient client);
```
in html.h, so that you can customize the actions taken by the board (i.e. actuate, reading sensor values, etc.) and the response pages sent back to the client.

After connecting to a local WiFi network, access the board by browsing the ip address of the board on port 80 (you can also change this in the code). You will then be able to view the response sent back by the board.

## Debugging
The WiFi 101 library is quite heavy-weighted and memory usage can be an issue. I recommand tracking available memory throughout the code and against using malloc in the code.

You can turn on/off serial printing by commenting / uncommenting the line `#define SERVER_DEBUG_MODE`.
