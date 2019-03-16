#ifndef AntMinerClass_h
#define AntMinerClass_h

#include  <WiFi.h>
#include  <WiFiClient.h>

#define   MINER_URL_MAX_LEN    100
#define   ERRMSG_BUF_LEN       40

class AntMiner {

  private:
  
    WiFiClient    miner;
    int           update_interval;
    unsigned long last_update;
    char          miner_url[MINER_URL_MAX_LEN] = {0};
    int           miner_id;

    char  *removeWhiteSpaces( char  *cp );
    int   parseCurleyBrackets( char *g_buf, char  *buf );

  public:

    bool          online;
    char          status_msg[ERRMSG_BUF_LEN] = {0};
    bool          console_logging_enabled;

    float         ghs_av;
    float         ghs_5s;
    unsigned long fan3;
    unsigned long fan6;
    unsigned long temp6;
    unsigned long temp7;
    unsigned long temp8;
    unsigned long uptime;
    unsigned long frequency;

    bool    oled_update_required;
    bool    network_problem_detected;
   
    AntMiner( int interval, char  *url, int id );
    void  update();
    void  dataTimeStamp();

};


#endif
