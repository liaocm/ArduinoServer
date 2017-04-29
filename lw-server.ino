#include <SPI.h>
#include <WiFi101.h>
#include "html.h"

#define SERVER_DEBUG_MODE

char ssid[] = "YOUR SSID HERE";
char pass[] = "YOUR PASSWORD HERE";
int keyIndex = 0;

int status = WL_IDLE_STATUS;
const size_t BUF_SIZE = 128;
const unsigned long TTL = 250; // timeout in milliseconds

WiFiServer server(80); // server runs on port 80

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  server.begin();
  printConnectionStatus();
}


void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    #ifdef SERVER_DEBUG_MODE
      Serial.println("New connection.");
    #endif
    processRequest(client);
  }
}

void sendResponseByFd(int fd, WiFiClient client) {
  if (fd == 200) {
    sendDummyResponse(client);
  } else if (fd == 500) {
    sendErrorResponse(client);
  } else {
    // actuate according to the fd
    sendDummyResponse(client);
  }

}

void processRequest(WiFiClient client) {
  char buffer[BUF_SIZE + 1];
  int idx = 0;
  int response_fd = 0;
  unsigned long time_stamp = millis();

  while (client.connected() && millis() - time_stamp < TTL) {
    if (!client.available()) continue;

    time_stamp = millis();
    buffer[idx] = client.read();
    
    #ifdef SERVER_DEBUG_MODE
      Serial.write(buffer[idx]);
    #endif

    if (buffer[idx] == '\n') {
      if (idx <= 1) {
        #ifdef SERVER_DEBUG_MODE
          Serial.println("End of header detected.");
        #endif
        // end of request header, send response
        sendResponseByFd(response_fd, client);
        client.stop();
        #ifdef SERVER_DEBUG_MODE
          Serial.println("client disconnected.");
        #endif
        return;
      }

      // else, process the current line
      buffer[idx+1] = '\0';
      int line_result = processLine(buffer);
      if (line_result != 0) {
        response_fd = line_result;
      }
      idx = -1;
    }

    idx++;
    if (idx >= BUF_SIZE) {
      idx = 0;
    }
  }
  #ifdef SERVER_DEBUG_MODE
    if (client.connected()) client.stop();
    Serial.println("time out: connection closed.");
  #endif
}

int processLine(char *line) {
  if (!isGETField(line)) {
    return 0;
  }
  return processGETLine(line);
}

int processGETLine(char *line) {
  // GET /path/to/file?x&y=1&&z=asd HTTP/1.1\r\n
  int start = 4;
  int curr = 4;
  while (line[curr] != ' ' && line[curr] != '?' && line[curr] != '\n') curr++;
  if (line[curr] == '\n') {
    // ERROR
    return 500;
  }

  unsigned char num_params = 0;
  char *param_str;
  if (line[curr] == '?') {
    num_params = 1;
    // Process params
    int param_start = curr + 1;
    int param_end = param_start;
    while (line[param_end] != ' ' && line[param_end] != '\n') param_end++;
    
    if (line[param_end] == '\n') {
      return 500;
    }

    line[param_end] = '\0';
    param_str = line + param_start;
    for (int i = 0; i < strlen(param_str); i++) {
      if (param_str[i] == '&') num_params++;
    }
  }
  // process path name
  // /path/to/file
  line[curr] = '\0';
  char *path = line + start;

  // process param string
  // i.e. x&y=1&&z=asd
  char *param_names[num_params];
  char *param_vals[num_params];
  int param_id = 0;
  int idx = 0;
  curr = 0;
  char empty_str[1];
  empty_str[0] = '\0';
  while (true) {
    if (param_str[idx] == '&' || param_str[idx] == '\0') {
      boolean break_flag = param_str[idx] == '\0';
      param_str[idx] = '\0';
      char *current_param = param_str + curr;
      // i.e. current_param = "x=3"
      size_t param_len = strlen(current_param);
      if (param_len == 0) {
        param_names[param_id] = empty_str;
        param_vals[param_id] = empty_str;
      } else {
        size_t eq_idx = param_len - 1;
        boolean has_eq = false;
        for (int i = 0; i < param_len; i++) {
          if (current_param[i] == '=') {
            eq_idx = i;
            has_eq = true;
            break;
          }
        }
        if (has_eq) {
          // i.e. eq_idx = 1, "x=3"
          current_param[eq_idx] = '\0';
        }
        param_names[param_id] = current_param;
        param_vals[param_id] = current_param + eq_idx + 1;
      }
      param_id++;
      curr = ++idx;
      if (break_flag) break;
    } else {
      idx++;
    }
  }

  #ifdef SERVER_DEBUG_MODE
  // print out request contents
  Serial.println("GET request:");
  Serial.println("----------------");
  Serial.print("PATH: ");
  Serial.println(path);
  Serial.println();
  Serial.print("Number of params: ");
  Serial.println(num_params);
  Serial.println();
  for (int i = 0; i < num_params; i++) {
    Serial.print("Param ");
    Serial.println(i);
    Serial.println(param_names[i]);
    Serial.println(param_vals[i]);
  }
  Serial.println("---end of request---");
  #endif

  return fileDescriptorForRequest(path, num_params, param_names, param_vals);
}

int fileDescriptorForRequest(char *path, 
                             unsigned char num_params,
                             char **param_names,
                             char **param_vals) {
  // Define your files, actions and responses here.
  return 200;
}

boolean isGETField(char *line) {
  size_t line_size = strlen(line);

  if (line_size <= 4) {
    return false;
  }
  if (line[0] != 'G' || line[1] != 'E' || line[2] != 'T' || line[3] != ' ') {
    return false;
  }
  #ifdef SERVER_DEBUG_MODE
    Serial.println("GET line detected!");
  #endif
  return true;
}

void printConnectionStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
