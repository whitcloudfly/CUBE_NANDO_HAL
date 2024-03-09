// Copyright (c) 2010-2019 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
//这段代码片段展示了一个$C++$中的配置文件解析器和定时器初始化函数。
//配置文件通过打开命令行参数指定的文件或使用默认文件名“LinuxSample.cfg”来读取。
//然后逐行读取文件，提取主机IP地址和端口、执行速率（以赫兹为单位）以及其他配置参数的数值。
//timerInit函数使用POSIX定时器函数初始化一个定时器。它根据指定的执行速率设置定时器事件和间隔。timerWait函数通过等待wait_mask指定的信号来等待下一个周期。
//总的来说，这段代码用于为Linux示例应用程序设置配置参数和定时器。
// LinuxSample.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../CommonFiles/clientServer.h"
#include "../CommonFiles/clientServerData.h"
#define  SOCKET_ERROR   (-1)

using namespace std;

///~~~ GLOBAL DATA

string         ipaddr_otw;
int            ipport_otw;
double         rate;             // rate in Hz

//~~~ timer data

static timer_t        my_timer;
static sigset_t       wait_mask;

//~~~ FORWARD REFERENCES

void   readConfigFile(int argc, char *argv[]);
void   timerInit();
void   timerWait();

//-----------------------------------------------------------------------------
//      main: Main function to transfer data
//-----------------------------------------------------------------------------

int main(int argc, char *argv[]) 
{
   //~~~ general setup

   readConfigFile(argc, argv);
   timerInit();

   PACKET_SYSTEM p3dhost;
   p3dhost.OpenPort(strdup(ipaddr_otw.c_str()), false);

   //~~~ MAIN LOOP

   while (true) {

      //~~~ wait for our next interval

      timerWait();

      static double s_movePosition = 0.0;
      static double s_moveArticulations = 50.0;
      static bool s_incArticulations = true;

      s_movePosition += .0000216195652;

      if(s_moveArticulations > 100.0)
      {
          s_incArticulations = false;
          s_moveArticulations = 100.0;
      }
      else if(s_moveArticulations < 0.0)
      {
          s_incArticulations = true;
          s_moveArticulations = 0.0;
      }
      else if(s_incArticulations)
      {
          s_moveArticulations += (2.0/rate);
      }
      else
      {
          s_moveArticulations -= (2.0/rate);
      }

      //Set Data to send
      p3dhost.ownshipPosition.Latitude = 0.63500899058954607 + s_movePosition;
      p3dhost.ownshipPosition.Longitude = -2.0048015696394814;
      p3dhost.ownshipPosition.Altitude = 15000;

      p3dhost.ownshipPosition.Pitch = 0.0;
      p3dhost.ownshipPosition.Bank = 0.0;
      p3dhost.ownshipPosition.Heading = 0.0;

      p3dhost.articulations.WingPosition = s_moveArticulations;
      p3dhost.articulations.RudderPosition = s_moveArticulations;

      //Update Packet Data
      p3dhost.WritePort();
      p3dhost.ReadPort();

      //Update Local Data
      cout << p3dhost.radarAlt.RadarAlt << "\n";

   }

   //~~~ close socket
   p3dhost.ClosePort();

   //~~~ delete timer
   timer_delete(my_timer);

   cout << "LinuxSample: Shutting down" << endl;
   return 0;
}

//-----------------------------------------------------------------------------
//      readConfigFile: Reads the user customized parameters.
//-----------------------------------------------------------------------------

void readConfigFile(int argc, char *argv[])
{
   int dummy;

   //~~~ read in command line arg that holds the config file

   string cfg_filename = (argc == 2) ? argv[1] : "LinuxSample.cfg";

   //~~~ read in the xpgh config file

   ifstream cfg_file;
   cfg_file.open(cfg_filename.c_str(), ios::in);
   if (!cfg_file) {
      cerr << "LinuxSample: ERROR! Cannot open config file: " << cfg_filename 
           << endl;
      exit(1);
   }

   string label;

   //~~~ Host IP address and port

   cfg_file >> label;
   cfg_file >> ipaddr_otw;
   if (ipaddr_otw.compare("0")) {
      cfg_file >> ipport_otw;
   } else {
      cfg_file >> dummy;
      ipaddr_otw.clear();
   }

   //~~~ execution rate in Hz

   cfg_file >> label;
   cfg_file >> rate;

   cfg_file.close();
}

//-----------------------------------------------------------------------------
//      timerInit: Initialize timer data.
//-----------------------------------------------------------------------------

void timerInit()
{
   sigset_t mask;                               // block the signal
   sigemptyset(&mask);
   sigaddset(&mask, SIGRTMAX);
   sigprocmask(SIG_BLOCK, &mask, 0);

   struct sigevent timer_event;                 // setup signal info
   timer_event.sigev_notify = SIGEV_SIGNAL;
   timer_event.sigev_signo  = SIGRTMAX;

   timer_create(CLOCK_REALTIME,                 // create the timer
      &timer_event, &my_timer);

   double interval = 1.0 / rate;                // set up the interval
   struct itimerspec setting;
   setting.it_value.tv_sec  = 0;
   setting.it_value.tv_nsec = 10000000;
   int secs  = interval;
   int nsecs = (interval - secs) * 1000000000;
   setting.it_interval.tv_sec  = secs;
   setting.it_interval.tv_nsec = nsecs;
   timer_settime(my_timer, 0, &setting, 0);

   sigemptyset(&wait_mask);                     // specify signal to wait on
   sigaddset(&wait_mask, SIGRTMAX);
}

//-----------------------------------------------------------------------------
//      timerWait: Wait for next cycle.
//-----------------------------------------------------------------------------

void timerWait()
{
   sigwaitinfo(&wait_mask, 0);
}

