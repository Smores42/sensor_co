/*
 * This class tests interprocess socket communication and the mongdb insert and update
 * If you input from client, a 1xx then you can test the insert mongdb     
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "./mongoDB_c/mongodb_api.c"
#include "./socket_conn/socket_conn.c"
#include "./packets/packet_parser.c"
#include <syslog.h>

int main(int argc, char **argv[])
{
        int sockFd = socket_conn();

        //Define the bufferIn that will accept the data packet with length of 127 bytes       
        int buffSize = 19; // Length of bufferIn in = 6
        char bufferIn[buffSize];
        
        int readRes;
        bool quit = false; //Loop Lookinf for q to exit
        int sensor_id, sensor_status, sensor_value;

        while(quit != true){

                quit = false;
                readRes = read(sockFd,bufferIn,buffSize);
                printf("Naked Data from Client: %s \n",bufferIn);        

                if(readRes < 0)
                        handle_error("SOCKET READING ERR");
                if(bufferIn[0] == 'q')
                {
                        printf("Quitting\n");
                        quit = true;
                }
                if(bufferIn[0] == 'e'){
                       printPacket(bufferIn);
                       write(sockFd,"eeeeeeeeeeeeeeeeee",buffSize);
                       //syslog(LOG_INFO,"Wrote eee...!");
                }
                // Insert Command Received
                if(bufferIn[14] == 'i'){ //0 right now for clarity 14
                    printf("Instruction:Insert\n");
                    printf("Checking to see if sensor exists...\n");
                    char *id,*cInstr,*cData,*sInstr,*sData;
                    char *val;
                    id = malloc(4);
                    val = malloc(4);
                    cInstr = malloc(4);
                    cData = malloc(4);
                    sInstr = malloc(4);
                    sData = malloc(4);

                    id = getId(bufferIn);
                    val = getVal(bufferIn);
                    cInstr = getCInstr(bufferIn);
                    cData = getCData(bufferIn);
                    sInstr = getSInstr(bufferIn);
                    sData = getSData(bufferIn);
                    if(getCollSensors(id) == 1){
                        printf("Sensor Exists!\n");
                        write(sockFd,"eeeeeeeeeeeeeeeeee",buffSize);
                    }
                    else{
                        printf("Sensor Does not Exist,\nInserting...\n");
                        insertSensor(id, val, cInstr, cData, sInstr, sData);
                        write(sockFd,"eeeeeeeeeeeeeeeeee",buffSize);
                    }


                }
                if(bufferIn[14] == 'u'){
                    printf("Instr: i\n");
                    char *id, *val;
                    id = malloc(4);
                    val = malloc(4);
                    id = getId(bufferIn);
                    val = getVal(bufferIn);
                    printf("Updating ID: %s with Val of: %s \n",id,val);
                    if(getCollSensors(id) == 1){
                        printf("Sensor Exists!\n");
                        updateSensor(id,val);
                        write(sockFd,"uuuuuuuuuuuuuuuuuu",buffSize);
                    }
                    else{
                        printf("Sensor Does not Exist\n");
                        write(sockFd,"uuuuuuuuuuuuuuuuuu",buffSize);
                    }
                    


                }
                // Test the get sensor
                if(bufferIn[14] == 'g'){
                    printf("Checking to see if sensor exists...\n");
                    char *id;
                    id = malloc(4);
                    id = getId(bufferIn);
                    if(getCollSensors(id) == 1)
                        return printf("Sensor Exists!\n");
                    else
                        return printf("Sensor Does not Exist\n");
                    //return getCollSensors();

                }
                else                       
                        write(sockFd,"ssssssssssssssssss",buffSize);
                }

        printf("\nClosing connection...\n");
        printf("Unlinking...");
        unlink("/tmp/mysocket");
        printf("Done\n");
        socket_close();
        return 0;
}
