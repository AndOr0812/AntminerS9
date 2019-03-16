#include  "AntMinerClass.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
AntMiner::AntMiner( int interval, char  *url, int id ) {
    
  update_interval = interval;
  last_update = millis();
  strcpy( miner_url, url );
  miner_id = id;
  network_problem_detected = false;
  oled_update_required = false;

  online = false;
  console_logging_enabled = true;
  strcpy( status_msg, "Offline" );

  ghs_av = 0;
  fan3 = 0;
  fan6 = 0;
  temp6 = 0;
  temp7 = 0;
  temp8 = 0;
  uptime = 0;
  frequency = 0;
}
  
////////////////////////////////////////////////////////////////////////////////////////////////////
void  AntMiner::update() {
  int   pc;
    
  if((millis() - last_update) > update_interval) {
    last_update = millis();
      
    if( miner.connect(miner_url, 4028)) {
      miner.printf( "{\"command\": \"stats\"}");
      String reply = miner.readStringUntil('\r');
      reply.replace("}{", "},{");
      online = true;
      strcpy( status_msg, "Online" );

      pc = parseCurleyBrackets( "", (char *)reply.c_str() );
    } else {
      Serial.printf( "Failed to connect to miner : %s\n", miner_url );
      online = false;
      strcpy( status_msg, "Offline" );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
char  *AntMiner::removeWhiteSpaces( char  *cp ) {
  while( *cp && (isprint(*cp) == 0 || *cp == ' ' )) {
    //Serial.printf( "[%02x] [%c] \n", *cp, *cp );
    cp ++;
  }
  return cp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int  AntMiner::parseCurleyBrackets( char *g_buf, char  *buf ) {

  char  *cp;
  char  p_buf[1024];
  char  v_buf[1024];
  int   cc, pc;
  bool  selected; 
       
  pc = 0;
  cp = buf;
  while( *cp != '}' && *cp != '\0' ) {
    cp = removeWhiteSpaces( cp );
    if(( *cp == '{') || ( *cp == ',' )) {
      cp++;
      cp = removeWhiteSpaces( cp );
      if( *cp == '"') {
        cp++;
        cc = 0;
        while( *cp != '"' ) {
          p_buf[cc] = *cp;
          cp++;
          cc++;
        }
        p_buf[cc] = '\0';
        cp++;
        cp = removeWhiteSpaces( cp );
        if( *cp == ':') {
          cp++;
          cp = removeWhiteSpaces( cp );
          if( *cp == '[') {
            while( *cp != ']') {
              if( *cp != '{' ) {
                cp++;
              }
              pc = parseCurleyBrackets(p_buf, cp);
              if( pc == -1 ) {
                return -1;
              }
              cp += pc;
            }
            cp++;
          } else if ( *cp == '"') {
            cp++;
            cc = 0;
            while( *cp != '"') {
              v_buf[cc] = *cp;
              cp++;
              cc++;
            }
            v_buf[cc] = '\0';
            selected = false;
            if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "frequency", 9) == 0 )) {
              selected = true;
              frequency = atoi( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "GHS 5s", 6) == 0 )) {
              selected = true;
              ghs_5s = atof( v_buf );
            }
            if( selected && console_logging_enabled == true ) {
              Serial.printf( "[%7s][%28s] = [\"%s\"]\n", g_buf, p_buf, v_buf );
            }
            cp++;
          } else {  // Variable to parse
            cc = 0;
            while( *cp != ',' && *cp != '}') {
              v_buf[cc] = *cp;
              cp ++;
              cc++;
            }
            v_buf[cc] = '\0';
              
            selected = false;
            if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "GHS av", 6) == 0 )) {
              selected = true;
              ghs_av = atof( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "Elapsed", 7) == 0 )) {
              selected = true;
              uptime = atoi( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "fan3", 4) == 0 )) {
              selected = true;
              fan3 = atoi( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "fan6", 4) == 0 )) {
              selected = true;
              fan6 = atoi( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "temp6", 5) == 0 )) {
              selected = true;
              temp6 = atoi( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "temp7", 5) == 0 )) {
              selected = true;
              temp7 = atoi( v_buf );
            } else if(( strncasecmp( g_buf, "STATS", 5) == 0 ) && ( strncasecmp( p_buf, "temp8", 5) == 0 )) {
              selected = true;
              temp8 = atoi( v_buf );
            }
            if( selected && console_logging_enabled == true ) {
              Serial.printf( "[%7s][%28s] = [%s]\n", g_buf, p_buf, v_buf );
            }
          }
          cp = removeWhiteSpaces( cp );         
        } else {
          Serial.printf( "Delimiter missing : %s\n", cp );
          return -1;
        }
      } else {
        Serial.printf( "Parameter Missing Expecting \" found : [%02x] [%c] \n", *cp, *cp );
        return -1;
      }
    } else {
      Serial.printf( "Expecting { or , found : [%02x] [%c] \n", *cp, *cp );
      return -1;
    }
    cp = removeWhiteSpaces( cp );         
  }
  cp++;
  cp = removeWhiteSpaces( cp );         
  return (int)(cp - buf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  AntMiner::dataTimeStamp() {

  Serial.printf("AntMiner%d Online     =    %s \n", miner_id, online ? "Yes" : "No" );
  Serial.printf("AntMiner%d Uptime     =    %4d Seconds\n", miner_id, uptime );
  Serial.printf("AntMiner%d GH/s Ave   = %9.2f Gh/s\n", miner_id, ghs_av );
  Serial.printf("AntMiner%d GH/s 5s    = %9.2f Gh/s\n", miner_id, ghs_5s );
  Serial.printf("AntMiner%d Fan3       =    %4d rpm\n", miner_id, fan3 );
  Serial.printf("AntMiner%d Fan6       =    %4d rpm\n", miner_id, fan6 );
  Serial.printf("AntMiner%d Temp6      =    %4d °C\n", miner_id, temp6 );
  Serial.printf("AntMiner%d Temp7      =    %4d °C\n", miner_id, temp7 );
  Serial.printf("AntMiner%d Temp8      =    %4d °C\n", miner_id, temp8 );
}
