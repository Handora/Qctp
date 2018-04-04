/******************************************************************************
* loog.c                                                                      *
*                                                                             *
* Description: This function is used by Liso to log some information such as  *
*              errors or infos. Do this by following such rules.              *
*    1. Handle creation of a log file as specified in the "project handout.“  *
*    2. Make API functions for formatted (up to you; we may specify this      *
*       later) writing to this log file (not thread-safe).                    *
*    3. Make an API function for gracefully closing this log file.            *
*    4. Expose and use this module/API within the Checkpoints instead of      *
*       using stderr or stdout.                                               *
*    5. Log IP addresses and browser information/settings as coming from      *
*       connected clients.                                                    *
*    6. Log all errors encountered to the log file.                           *
*                                                                             *
* Authors: QianChen <qcdsr970209@gmail.edu>                                   *
*                                                                             *
*******************************************************************************/

#include "log/loog.h"
#include <stdio.h>
#include <time.h>

FILE* logFile;

/* @ Description: init the log file with filename and print the time
 * @ input:       filename
 * @ output:      if success return the FILE * stream, else set the errno and return Null
 */
int
loogInit(char *filename) {
    time_t t;
    char timestamp[50];
    logFile = fopen(filename, "a");
    if (logFile == 0) {
        return -1;
    }
    time(&t);
    sprintf(timestamp, "Now time: %s", ctime(&t));
    loog(timestamp);
    loog("******** loog Designed by Handora ********");
    return 0;
}

/* @ Description: log some custom message
 * @ input:       the File * logFile stream and custom message
 * @ output:      None
 */
void
loog(char *message) {
#ifndef PRODUCTION
    fprintf(logFile, "%s\n", message);
    fflush(logFile);
#endif
}

/* @ Description: log connection message
 * @ input:       the File * logFile stream, customer ip Addr, customer ip port and customer descriptor
 * @ output:      None
 */
void
loogConnection(char *ipAddr, int ipPort, int connfd) {
    fprintf(logFile, "Connection with %s:%d(fd: %d)\n", ipAddr, ipPort, connfd);
    fflush(stdout);
    fflush(logFile);
}

/* @ Description: log closure of connection
 * @ input:       the File * logFile stream and customer descriptor
 * @ output:      None
 */
void
loogClosure(int connfd) {
    fprintf(logFile, "Closing connection with fd: %d\n", connfd);
    fflush(stdout);
    fflush(logFile);
}

/* @ Description: log the errorMessage of the customers
 * @ input:       the File * logFile stream and errorMessage
 * @ output:      None
 */
void
loogError(char *errorMessage) {
    fprintf(logFile, "Big Error!!!!:");
    fprintf(logFile, "%s\n", errorMessage);
    fflush(logFile);
}
