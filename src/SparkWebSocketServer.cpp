/*******************************************************************************
* Websocket-Arduino, a websocket implementation for Arduino
* Copyright 2014 NaAl (h20@alocreative.com)
* Based on previous implementations by
* Copyright 2011 Per Ejeklint
* and
* Copyright 2010 Ben Swanson
* and
* Copyright 2010 Randall Brewer
* and
* Copyright 2010 Oliver Smith

* Some code and concept based off of Webduino library
* Copyright 2009 Ben Combee, Ran Talbott

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* -------------
* Now based off
* http://www.whatwg.org/specs/web-socket-protocol/
* 
* - OLD -
* Currently based off of "The Web Socket protocol" draft (v 75):
* http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-75
*******************************************************************************/

#include "SparkWebSocketServer.h"

#ifdef SUPPORT_HIXIE_76
#include "MD5.c"
#endif

#include "Base64.h"

#include "tropicssl/sha1.h"

struct WS_User_Func_Lookup_Table_t
{
    int (*pUserFunc)(String userArg);
    char userFuncKey[USER_FUNC_KEY_LENGTH];
    char userFuncArg[USER_FUNC_ARG_LENGTH];
    int userFuncRet;
    bool userFuncSchedule;
} WS_User_Func_Lookup_Table[USER_FUNC_MAX_COUNT];

SparkWebSocketServer::SparkWebSocketServer(TCPServer &tcpServer) {
  for(uint8_t i=0;i<MAX_CLIETNS;i++) {
    clients[i]=NULL;
  }
  server=&tcpServer;
  previousMillis = 0;
}

bool SparkWebSocketServer::handshake(TCPClient &client) {
  uint8_t pos=0;
  bool found=false;
  for(pos=0;pos<MAX_CLIETNS;pos++) {
    if(clients[pos] != NULL && client.equals(*clients[pos])) {
#ifdef DEBUG_WS
      String ip;
      client.getIP(ip);
      Serial.print(" SparkWebSocketServer::handshake, client[");
      Serial.print(pos);
      Serial.print("]: ");
      Serial.print(ip);
      Serial.println(", re-using!!");
      delay(10);
#endif
      found=true;
      break;
    }
  }
  if(!found) {
    for(pos=0;pos<MAX_CLIETNS;pos++) {
      if(clients[pos]==NULL) {
        break;
      }
    }
  }
  if(pos>=MAX_CLIETNS) {
    return false;
  }
  // If there is an empty spot
  if (pos < MAX_CLIETNS) {
    // Check request and look for websocket handshake
#ifdef DEBUG_WS
    String ip;
    client.getIP(ip);
    Serial.print(" SparkWebSocketServer::handshake, client[");
    Serial.print(pos);
    Serial.print("]: ");
    Serial.print(ip);
    Serial.println(", analyzing");
#endif
    if (analyzeRequest(BUFFER_LENGTH, client)) {
      clients[pos]=&client;
#ifdef DEBUG_WS
      String ip;
      client.getIP(ip);
      Serial.print("SparkWebSocketServer established, ");
      Serial.println(ip);
#endif
      return true;
    } else {
      // Might just need to break until out of socket_client loop.


      return false;
    }
  } else {
#ifdef DEBUG_WS
    Serial.println("Disconnecting/no free space for client");
#endif
    disconnectClient(client);
    return false;
  }
}

void SparkWebSocketServer::disconnectClient(TCPClient &client) {
#ifdef DEBUG_WS
  Serial.print("Terminating TCPClient: ");
  String ip;
  client.getIP(ip);
  Serial.println(ip);
#endif

  if (hixie76style) {
#ifdef SUPPORT_HIXIE_76
    // Should send 0xFF00 to server to tell it I'm quitting here.
    client.write((uint8_t) 0xFF);
    client.write((uint8_t) 0x00);
#endif
  } else {

    // Should send 0x8700 to server to tell it I'm quitting here.
    client.write((uint8_t) 0x87);
    client.write((uint8_t) 0x00);
  }

  client.flush();
  delay(10);
  client.stop();
  for(uint8_t i=0;i<MAX_CLIETNS;i++) {
    if(clients[i]!=NULL && client.equals(*clients[i])) {
#ifdef DEBUG_WS
      String ip;
      client.getIP(ip);
      Serial.print("found: ");
      Serial.print(ip);
      Serial.println(" and removing it from the list of the clients");
#endif
      TCPClient *tmp=clients[i];
      delete tmp;
      clients[i]=NULL;
      break;
    }
  }
}

void SparkWebSocketServer::getData(String &data, TCPClient &client) {

  if (hixie76style) {
#ifdef SUPPORT_HIXIE_76
    handleHixie76Stream(data, client);
#endif
  } else {
    handleStream(data, client);
  }
}


void SparkWebSocketServer::handleStream(String &data, TCPClient &client) {

  int length;
  uint8_t mask[4];



  if (client.connected()) {
    length=timedRead(client);
    if (!client.connected() || length==-1) {
      return ;
    }

    length = timedRead(client) & 127;
    if (!client.connected()) {
      return ;
    }

    if (length == 126) {
      length = timedRead(client) << 8;
      if (!client.connected()) {
        return ;
      }

      length |= timedRead(client);
      if (!client.connected()) {
        return ;
      }

    } else if (length == 127) {
#ifdef DEBUG_WS
      Serial.println("No support for over 16 bit sized messages");
#endif
      return;
    }

    // get the mask
    mask[0] = timedRead(client);
    if (!client.connected()) {
      return ;
    }

    mask[1] = timedRead(client);
    if (!client.connected()) {

      return ;
    }

    mask[2] = timedRead(client);
    if (!client.connected()) {
      return ;
    }

    mask[3] = timedRead(client);
    if (!client.connected()) {
      return ;
    }

    for (int i=0; i<length; ++i) {
      data += (char) (timedRead(client) ^ mask[i % 4]);
      if (!client.connected()) {
        return ;
      }
    }
  }
}

int SparkWebSocketServer::timedRead(TCPClient &client) {
  uint8_t test=0;
  while (test<20 && !client.available() && client.connected()) {
    delay(1);
    test++;
  }
  if(client.connected()) {
    return client.read();
  }
  return -1;
}

void SparkWebSocketServer::sendEncodedData(char *str, TCPClient &client) {
  int size = strlen(str);
  if(!client) {
    return;
  }
  // string type
  client.write(0x81);

  // NOTE: no support for > 16-bit sized messages
  if (size > 125) {
    client.write(126);
    client.write((uint8_t) (size >> 8));
    client.write((uint8_t) (size && 0xFF));
  } else {
    client.write((uint8_t) size);
  }

  for (int i=0; i<size; ++i) {
    client.write(str[i]);
  }

}

void SparkWebSocketServer::sendEncodedData(String str, TCPClient &client) {
  int size = str.length() + 1;
  char cstr[size];

  str.toCharArray(cstr, size);

  sendEncodedData(cstr, client);
}

void SparkWebSocketServer::sendData(const char *str, TCPClient &client) {
  if (client && client.connected()) {
    if (hixie76style) {
      client.print(0x00); // Frame start
      client.print(str);
      client.write(0xFF); // Frame end
    } else {
      sendEncodedData(str, client);
    }
  }

}

void SparkWebSocketServer::sendData(String str, TCPClient &client){
  if (client && client.connected()) {
    if (hixie76style) {
      client.print((char)0x00); // Frame start
      client.print(str);
      client.write(0xFF); // Frame end
    } else {
      sendEncodedData(str, client);
    }
  }

}
void SparkWebSocketServer::doIt() {
  //handle new clients

  unsigned long currentMillis = millis();
  bool beat=currentMillis - previousMillis > HB_INTERVAL;
  if(beat) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
  }
  TCPClient *client=server->available();

  if(client && client->connected() ) {
#ifdef DEBUG_WS
    String ip;
    client->getIP(ip);
    Serial.print("new client connecting, testing: ");
    Serial.println(ip);
#endif
    handshake(*client);
  }
  for(uint8_t i=0;i<MAX_CLIETNS;i++) {
    TCPClient *myClient=clients[i];
    if(myClient!=NULL && !myClient->connected()) {
#ifdef DEBUG_WS
      String ip;
      myClient->getIP(ip);
      Serial.print("Client[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(ip);
      Serial.println(" disconnected!");
#endif
      disconnectClient(*myClient);
      //      clients[i]=NULL;
    } else if(myClient!=NULL && myClient->connected()) {
      String test;
      getData(test,*myClient);
      if(test.length()>0) {
#ifdef DEBUG_WS
        String ip;
        myClient->getIP(ip);
        Serial.print("got : ");
        Serial.print(test+" from: ");
        Serial.println(ip);
        delay(1000);
#endif
        if(test.startsWith("/dfu")) {
          sendData("going to dfu mode", *myClient);
          for(uint8_t j=0;j<MAX_CLIETNS;j++) {
            TCPClient *tmp=clients[j];
            if(tmp!=NULL && !tmp->connected()) {
              disconnectClient(*tmp);
            }
          }
          delay(1000);
        }
        String resutl;
        (*cBack)(test, resutl);
#ifdef DEBUG_WS
        Serial.print("result: ");
        Serial.println(resutl);
#endif
        sendData(resutl, *myClient);
      } else {
        if(beat) {
          if(myClient!=NULL && myClient->connected()) {
#ifdef DEBUG_WS
            String ip;
            myClient->getIP(ip);
            Serial.print("sending HB to: ");
            Serial.println(ip);
#endif
            sendData("HB", *myClient);
          } else {
            disconnectClient(*myClient);
          }
        }
      }
    }
  }
}

bool SparkWebSocketServer::analyzeRequest(int bufferLength, TCPClient &client) {
  // Use String library to do some sort of read() magic here.
  String temp;

  int bite;
  bool foundupgrade = false;
  String oldkey[2];
  String newkey;

  hixie76style = false;

#ifdef DEBUG_WS
  Serial.println("Analyzing request headers");
#endif
  // TODO: More robust string extraction
  while ((bite = client.read()) != -1) {

    temp += (char)bite;

    if ((char)bite == '\n') {

#ifdef DEBUG_WS
      Serial.print("Got Line: " + temp);
#endif
      // TODO: Should ignore case when comparing and allow 0-n whitespace after ':'. See the spec:
      // http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
      if (!foundupgrade && temp.startsWith("Upgrade: WebSocket")) {
        // OK, it's a websockets handshake for sure
        foundupgrade = true;
        hixie76style = true;
      } else if (!foundupgrade && temp.startsWith("Upgrade: websocket")) {
        foundupgrade = true;
        hixie76style = false;
      } else if (temp.startsWith("Origin: ")) {
        origin = temp.substring(8,temp.length() - 2); // Don't save last CR+LF
      } else if (temp.startsWith("Host: ")) {
        host = temp.substring(6,temp.length() - 2); // Don't save last CR+LF
      } else if (temp.startsWith("Sec-WebSocket-Key1: ")) {
        oldkey[0]=temp.substring(20,temp.length() - 2); // Don't save last CR+LF
      } else if (temp.startsWith("Sec-WebSocket-Key2: ")) {
        oldkey[1]=temp.substring(20,temp.length() - 2); // Don't save last CR+LF
      } else if (temp.startsWith("Sec-WebSocket-Key: ")) {
        newkey=temp.substring(19,temp.length() - 2); // Don't save last CR+LF
      }
      temp = "";
    }

    if (!client.available()) {
      delay(20);
    }
  }

  if (!client.connected()) {
    return false;
  }

  temp += 0; // Terminate string

  // Assert that we have all headers that are needed. If so, go ahead and
  // send response headers.
  if (foundupgrade == true) {

#ifdef SUPPORT_HIXIE_76
    if (hixie76style && host.length() > 0 && oldkey[0].length() > 0 && oldkey[1].length() > 0) {
      // All ok, proceed with challenge and MD5 digest
      char key3[9] = {0};
      // What now is in temp should be the third key
      temp.toCharArray(key3, 9);

      // Process keys
      for (int i = 0; i <= 1; i++) {
        unsigned int spaces =0;
        String numbers;

        for (int c = 0; c < oldkey[i].length(); c++) {
          char ac = oldkey[i].charAt(c);
          if (ac >= '0' && ac <= '9') {
            numbers += ac;
          }
          if (ac == ' ') {
            spaces++;
          }
        }
        char numberschar[numbers.length() + 1];
        numbers.toCharArray(numberschar, numbers.length()+1);
        intkey[i] = strtoul(numberschar, NULL, 10) / spaces;
      }

      unsigned char challenge[16] = {0};
      challenge[0] = (unsigned char) ((intkey[0] >> 24) & 0xFF);
      challenge[1] = (unsigned char) ((intkey[0] >> 16) & 0xFF);
      challenge[2] = (unsigned char) ((intkey[0] >>  8) & 0xFF);
      challenge[3] = (unsigned char) ((intkey[0]      ) & 0xFF);
      challenge[4] = (unsigned char) ((intkey[1] >> 24) & 0xFF);
      challenge[5] = (unsigned char) ((intkey[1] >> 16) & 0xFF);
      challenge[6] = (unsigned char) ((intkey[1] >>  8) & 0xFF);
      challenge[7] = (unsigned char) ((intkey[1]      ) & 0xFF);

      memcpy(challenge + 8, key3, 8);

      unsigned char md5Digest[16];
      MD5(challenge, md5Digest, 16);

      client.print("HTTP/1.1 101 Web Socket Protocol Handshake\r\n"));
      client.print("Upgrade: WebSocket\r\n"));
      client.print("Connection: Upgrade\r\n"));
      client.print("Sec-WebSocket-Origin: "));
      client.print(origin);
      client.print(CRLF);

      // The "Host:" value should be used as location
      client.print("Sec-WebSocket-Location: ws://"));
      client.print(host);
      client.print(socket_urlPrefix);
      client.print(CRLF);
      client.print(CRLF);

      client.write(md5Digest, 16);

      return true;
    }
#endif

    if (!hixie76style && newkey.length() > 0) {
#ifdef DEBUG_WS
      Serial.print("!hixie76style: " + newkey);
#endif
      // add the magic string
      newkey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

      uint8_t hash[100];
      char result[21];
      char b64Result[30];

      //char hexstring[41];
      unsigned char a[newkey.length()+1];
      a[newkey.length()]=0;
      memcpy(a,newkey.c_str(),newkey.length());
      sha1(a,newkey.length(),hash); // 10 is the length of the string
      //toHexString(hash, hexstring);


      for (uint8_t i=0; i<20; ++i) {
        result[i] = (char)hash[i];
      }
      result[20] = '\0';

      base64_encode(b64Result, result, 20);

      client.print("HTTP/1.1 101 Web Socket Protocol Handshake\r\n");
      client.print("Upgrade: websocket\r\n");
      client.print("Connection: Upgrade\r\n");
      client.print("Sec-WebSocket-Accept: ");
      client.print(b64Result);
      client.print(CRLF);
      client.print(CRLF);

      return true;
    } else {
      // something went horribly wrong
      return false;
    }
  } else {
    // Nope, failed handshake. Disconnect
#ifdef DEBUG_WS
    Serial.println("Header mismatch");
#endif
    return false;
  }
}

#ifdef SUPPORT_HIXIE_76
String SparkWebSocketServer::handleHixie76Stream(String &socketString, TCPClient &client) {
  int bite;
  int frameLength = 0;
  // String to hold bytes sent by client to server.

  if (client.connected() && client.available()) {
    bite = timedRead(client);

    if (bite != -1) {
      if (bite == 0)
        continue; // Frame start, don't save

      if ((uint8_t) bite == 0xFF) {
        // Frame end. Process what we got.

      } else {
        socketString += (char)bite;
        frameLength++;

        if (frameLength > MAX_FRAME_LENGTH) {
          // Too big to handle!
#ifdef DEBUG_WS
          Serial.print("Client send frame exceeding ");
          Serial.print(MAX_FRAME_LENGTH);
          Serial.println(" bytes");
#endif
          return;
        }
      }
    }
  }

}

#endif

//
//void SparkWebSocketServer::function(const char *funcKey, int (*pFunc)(String paramString))
//{
//  int i = 0;
//  if(NULL != pFunc && NULL != funcKey)
//  {
//    if(WS_User_Func_Count == USER_FUNC_MAX_COUNT)
//      return;
//
//    for(i = 0; i < WS_User_Func_Count; i++)
//    {
//      if(WS_User_Func_Lookup_Table[i].pUserFunc == pFunc && (0 == strncmp(WS_User_Func_Lookup_Table[i].userFuncKey, funcKey, USER_FUNC_KEY_LENGTH)))
//      {
//        return;
//      }
//    }
//
//    WS_User_Func_Lookup_Table[WS_User_Func_Count].pUserFunc = pFunc;
//    memset(WS_User_Func_Lookup_Table[WS_User_Func_Count].userFuncArg, 0, USER_FUNC_ARG_LENGTH);
//    memset(WS_User_Func_Lookup_Table[WS_User_Func_Count].userFuncKey, 0, USER_FUNC_KEY_LENGTH);
//    memcpy(WS_User_Func_Lookup_Table[WS_User_Func_Count].userFuncKey, funcKey, USER_FUNC_KEY_LENGTH);
//    WS_User_Func_Lookup_Table[WS_User_Func_Count].userFuncSchedule = false;
//    WS_User_Func_Count++;
//  }
//}
