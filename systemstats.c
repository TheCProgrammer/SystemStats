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
#include <stdbool.h>

/* You may want to change those values depending on the filepath of those files */

char *tempfile = "/sys/class/thermal/thermal_zone0/temp";
char *acstatus = "/sys/class/power_supply/AC/online";
char *batcapacity_file = "/sys/class/power_supply/BAT0/capacity";

/* Global variables */

int batcapacity = 0;
FILE* batpercentfp = NULL;
FILE* acstatusfp = NULL;

FILE* openfp(char *mode) { // shouldn't need to worry about the warning (about not returning anything) here because we will always supply the function with a mode
  if (strcmp(mode, "acstatus") == 0) {
    return fopen(acstatus, "r");   
  }

  else {
    return fopen(batcapacity_file, "r");       
  }
} 

void checkfp_error(FILE* fp) {
  if (!fp) {
    perror("fopen");
    exit(EXIT_FAILURE);    
  }
}

int checkbatpercent() { // returns 1 if the battery capacity is <= 10, returns 0 otherwise
  batpercentfp = openfp("batcapacity"); 
  checkfp_error(batpercentfp);

  char buffer[4]; // max 3 numbers + NULL terminator (e.g. 100, 20, 10, ...)
  fread(buffer, sizeof(buffer), 1, batpercentfp);

  batcapacity = atoi(buffer);

  if (batcapacity == 0) { // atoi() returns 0 on error, The percentage will never be 0 because then the device would have already shutdowned
    fprintf(stderr, "Cannot convert the string (from the battery file) to an integer, It is un-likely for this error to happen\n"); // it will never do that unless your CPU temp is 0 (which will never happen)
    exit(EXIT_FAILURE);
  }

  if (batcapacity >= 10) {
    fclose(batpercentfp);  
    return 1; 
  }

  else {
    fclose(batpercentfp);      
    return 0;
  }
}

void handlebattery() {    
  acstatusfp = openfp("acstatus");
  checkfp_error(acstatusfp);

  char buffer[2]; // 0 or 1 + NULL terminator
  fread(buffer, sizeof(buffer), 1, acstatusfp);

  if (strcmp(buffer, "0") == 0) { // if AC is not connected
    if (checkbatpercent() == 1) {
      char warnmessage[200];
      snprintf(warnmessage, sizeof(warnmessage), "%s %d %s", "espeak-ng \"WANING: System on low battery", batcapacity, "\""); // don't forget the quotes

      system(warnmessage);

      printf("WARNING: System on low battery %d\n", batcapacity);
    }    
  }

  fclose(acstatusfp);
}

void handlehightemp(int temp) {
  char warnmessage[100];
  snprintf(warnmessage, sizeof(warnmessage), "%s %d %s %s", "espeak-ng \"High CPU temprature warning: ", temp, "°Celsius", "\""); // don't forget the quotes
 
  system(warnmessage);
  printf("High CPU temprature warning: %d°C\n", temp);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "USAGE: %s <laptop-or-desktop>\n", argv[0]);
    return -1;
  }

  char *mode = argv[1];
  bool runningLaptop = false;
  
  if (strcmp(mode, "laptop") == 0) {
    runningLaptop = true;
  }

  else if (strcmp(mode, "laptop") != 0 && strcmp(mode, "desktop") != 0) {
    fprintf(stderr, "Invalid mode\nUSAGE: %s <laptop-or-desktop>\n", argv[0]);    
    return -1;
  }
   
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

    if (runningLaptop) {
      handlebattery();
    }

    fclose(tempfp);    
    usleep(2000); // sleep for 2 seconds to reduce CPU usage
  }

  fclose(tempfp);
  return 0;
}
