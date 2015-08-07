//
//  OVSessionID.c
//  OVCLI
//
//  Created by Daniel Finneran on 21/05/2015.
//  Copyright (c) 2015 Daniel Finneran. All rights reserved.
//

#include "OVUtils.h"
#include "OVOutput.h"
#include "OVHttps.h"

#include "jansson.h"

void createURL(char urlString[], char *address, char *url)
{
    snprintf(urlString, 256, "https://%s/rest/%s", address, url);
}


int writeSessionIDforHost(const char *sessionID, const char *host)
{
    FILE *fp;
    char filepath[1000]; //large file path
    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, host); // Append the SessionID path
    
    fp = fopen(filepath, "w");
    if (fp) { /*file opened succesfully */
        fputs(sessionID, fp);
        fclose(fp);
    } else {
        printf("Error opening file %s \n", filepath);
        return -1;
    }
    
    return 0;
}

int writeSessionID(const char *sessionID)
{
    return writeSessionIDforHost(sessionID, sessionIDPath);
}

char* readSessionIDforHost(const char *host)
{
    FILE *fp;
    char buffer[33]; //size of Session ID
    char filepath[1000]; //large file path
    char *sessionID = NULL;
    sessionID = malloc(sizeof(buffer));

    strcpy(filepath, getenv("HOME")); // copy in the $HOME env into the string
    strcat(filepath, host);

    fp = fopen(filepath, "r");
    if (fp) { /*file opened succesfully */
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);
    } else {
        printf("Error opening file %s \n", filepath);
        return NULL;
    }
    strcpy(sessionID, buffer);
    return sessionID;
}

char* readSessionID(void)
{
    return readSessionIDforHost(sessionIDPath);
}

int ovLogin(char *argument[], char *path)
{
    char urlString[256];
    char *httpData;
    json_t *root;
    json_error_t error;
    
    char *oneViewAddress = argument[1]; // IP Address of HP OneView
    char *username = argument[3]; // Username Parameter
    char *password = argument[4]; // Password Parameter
    
    // Pack the json
    root = json_pack("{s:s, s:s}", "userName", username, "password", password);
    // dump it as a string to pass to the curl libs
    char *json_text = json_dumps(root, JSON_ENCODE_ANY);
    //build URL
    snprintf(urlString, 256, "https://%s/rest/%s", oneViewAddress, "login-sessions");
    
    if (getenv("OV_DEBUG"))
        printf("[INFO] URL:\t %s\n", urlString);
    // Call to HP OneView API
    httpData = postRequestWithUrlAndData(urlString, json_text);
    if (getenv("OV_DEBUG"))
        printf("[INFO] JSON:\t %s\n", json_text);
    free (json_text);
    if(!httpData)
        return 1;
    
    root = json_loads(httpData, 0, &error);
    free(httpData);
    const char* sessionID = json_string_value(json_object_get(root, "sessionID"));
    
    // Write the Session
    if (writeSessionIDforHost(sessionID, path) != 0) {
        printMessage(RED, "ERROR", "The HP OneView Session ID could not be saved");
        return 1;
    }
    printf("[DEBUG] OVID:\t  %s\n",sessionID);
    return 0;
    
}