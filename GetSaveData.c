#include <windows.h>
#include <stdio.h>
#include <time.h>


char date[20]; // Buffer to hold date string

int main() {

    FILE *fpt;    //file pointer variable
    fpt = fopen("MyFile.csv", "w+");   //open file in write mode


    HANDLE hSerial;
    unsigned long bytesRead;
    char buffer[256];
    char line[256];
    int idx = 0;

    hSerial = CreateFile(
        "\\\\.\\COM3",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening COM port\n");
        return 1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    printf("Reading from COM3...\n");

    fprintf(fpt,"timestamp;ammount\n");
    while (1) {
        if (ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) {
            for (unsigned long i = 0; i < bytesRead; i++) {
                char c = buffer[i];

                if (c == '\n') {  
                    line[idx] = '\0';


                    char dateStr[20];    //test
                    time_t now = time(NULL);    //test 

                    printf("message: %s\n", line);
                    fprintf(fpt, "%lld;%s\n", (long long)now, line); //check if time works as intended 
                    

                    idx = 0;
                } else {
                    line[idx] = c;
                    idx++;
                    if (idx >= sizeof(line)) idx = 0; // to avoid overflow
                }
            }
        }
    }

    CloseHandle(hSerial);
    return 0;
}


// open png
// write date + ammount of water used in terminal human readable

