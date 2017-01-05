/*****************
   Server Connection handling file.

   @author Fredrik Elestedt
   @date
   @history see the git log of the origin repository.
   @note For this to compile, you need https://github.com/per1234/EthernetMod
*/
#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H

#include <Ethernet.h>

class ServerConnection {
  public:
    static ServerConnection& getInstance()
    {
      static ServerConnection instance;
      return instance;
    }

    void loop();
    void setServer(byte server[4]);

    void println(String data);
  private:
    ServerConnection();
    ServerConnection(ServerConnection const&);              // Don't Implement
    void operator=(ServerConnection const&); // Don't implement

    EthernetClient server;
    long lastReconnect;
    byte serverIP[4];
    String inputString;
};

#endif
