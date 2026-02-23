/* SystemStats -- A powerful tool that notifies you when something is not right
 * Licensed under the GPL3.0, Following it's terms
 * Made by Yahia Loay 
 * Please note that this application uses espeak (espeak-ng) to make voice that notifies the user
 * Please note that this application contains a limited set of checks of what the user may want to know or "classify" as a problem
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* You may want to change those values depending on the filepath of those files */

char *tempfile = "/sys/class/thermal/thermal_zone0/temp";
char *acstatus = "/sys/class/power_supply/AC/online";

FILE* openacfp() {
  return fopen(acstatus, "r");
}

void checkfp_error(FILE* fp) {
  if (!fp) {
    perror("fopen");
    exit(EXIT_FAILURE);    
  }
}

void handlebattery() {
  FILE* fp = openacfp();
  checkfp_error(fp);

  char buffer[2]; // 1 or 2 + NULL terminator
  fread(buffer, sizeof(buffer), 1, fp);

  if (strstr(buffer, "0")) { // if AC is not connected
    
  }
}


void handlehightemp(int temp) {
  char warnmessage[100];
  snprintf(warnmessage, sizeof(warnmessage), "%s %d %s %s", "espeak-ng \"High CPU temprature warning: ", temp, "°Celsius", "\""); // don't forget the quotes
 
  system(warnmessage);
  printf("High CPU temprature warning: %d°C\n", temp);
}

int main() {
  printf("SystemStats -- A powerful system stats warning application\n");

  char tempbuffer[30];
  printf("Monitoring...\n");

  FILE* tempfp = NULL;
    
  while (1) {
    tempfp = fopen(tempfile, "r"); // keep re-opening the file to keep reading the temps from it
    
    if (!tempfp) {
      perror("tempfp: fopen");
      return -1;
    }

    fread(tempbuffer, sizeof(tempbuffer), 1, tempfp); 

    int millidegree_temp = atoi(tempbuffer); // convert the string tempbuffer to an integer because that's what the file should contain

    if (millidegree_temp == 0) { // atoi returns 0 on error, while the file may just contain 0 and atoi() didn't actually fail,
      fprintf(stderr, "Cannot convert the string (from the temperature file) to an integer, It is un-likely for this error to happen\n"); // it will never do that unless your CPU temp is 0 (which will never happen)
      return -1;
    }

    int temp = millidegree_temp / 1000;     

    if (temp >= 80) {
      handlehightemp(temp);
    }

    handlebattery();

    fclose(tempfp);    
    usleep(2000); // sleep for 2 seconds to reduce CPU usage
  }

  fclose(tempfp);
  return 0;
}
