#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>



//Forward declarations
void display_menu(void);
void empty_stdin(void);
void graph_Menu(void);
void tank_settings(void);
void analyze_csv(const char *tankCsv);
void write_graph_config(const char *start_date, const char *end_date, const char *graph_type);
void write_daily_config(void);
void write_weekly_config(void);
void write_yearly_config(void);
void write_tank_dimensions(int tank_type, int length, int width, int height,int indexPosition, int maxVolume);
void tank_setup(void);
void open_graph_image(const char *graph_type);
void read_tank_settings(void);
void reset_tank_settings(void);

// Global variables
int tankNumber = -1;
int tankIndex[8] = {0,0,0,0,0,0,0,0};  // Stores tankId at each slot position

//===========================================================================// Dependency file setup functions

// Function to write graph configuration to JSON
void write_graph_config(const char *start_date, const char *end_date, const char *graph_type) {
    if (tankNumber == -1) {
        tank_setup();
    }    

    char tankFile[50];
    snprintf(tankFile, sizeof(tankFile), "graph_config.json");
    
    FILE *fp = fopen(tankFile, "w");
    if (fp == NULL) {
        fprintf(stderr, "\nError: Cannot write to graph config file\nRetrying tank setup...\n\n");
        tank_setup();
    }
    else {
    fprintf(fp, "{\n");
    fprintf(fp, "    \"daily\": {\n");
    fprintf(fp, "        \"start_date\": \"%s\",\n", start_date);
    fprintf(fp, "        \"end_date\": \"%s\"\n", end_date);
    fprintf(fp, "    },\n");
    fprintf(fp, "    \"weekly\": {\n");
    fprintf(fp, "        \"start_date\": \"%s\",\n", start_date);
    fprintf(fp, "        \"end_date\": \"%s\"\n", end_date);
    fprintf(fp, "    },\n");
    fprintf(fp, "    \"yearly\": {\n");
    fprintf(fp, "        \"start_date\": \"%s\",\n", start_date);
    fprintf(fp, "        \"end_date\": \"%s\"\n", end_date);
    fprintf(fp, "    },\n");
    fprintf(fp, "    \"custom\": {\n");
    fprintf(fp, "        \"start_date\": \"%s\",\n", start_date);
    fprintf(fp, "        \"end_date\": \"%s\"\n", end_date);
    fprintf(fp, "    },\n");
    fprintf(fp, "    \"selected_graph_type\": \"%s\"\n", graph_type);
    fprintf(fp, "}\n");
    fclose(fp);
    }
}

void write_daily_config(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
    write_graph_config(date, date, "daily");
}

void write_weekly_config(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    time_t week_ago = now - (7 * 86400);
    struct tm *tm_week = localtime(&week_ago);
    
    char start_date[20], end_date[20];
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", tm_week);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
    write_graph_config(start_date, end_date, "weekly");
}

void write_yearly_config(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    time_t year_ago = now - (365 * 86400);
    struct tm *tm_year = localtime(&year_ago);
    
    char start_date[20], end_date[20];
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", tm_year);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
    write_graph_config(start_date, end_date, "yearly");
}



void tank_setup(void) { 
    int tankLength = 0, tankWidth = 0, tankHeight = 0, tankDiameter = 0; 
    int maxVolume = 0;
    int indexPosition = -1;
    int cylinderFlag = -1;
    int tankId = 0;  // Add tankId variable

    printf("\n=== Tank Setup ===\n");
    printf("Provide a numerical identifier for this tank (1-8):\n");
    if (scanf("%d", &tankId) != 1 || tankId <= 0 || tankId > 8) {
        printf("Invalid ID. Try again.\n");
        empty_stdin();
        return tank_setup();
    }
    else {
        printf("Tank ID: %d\n\n", tankId);
        tankNumber = tankId;  // Set tankNumber to tankId
    }
    empty_stdin();
    
    printf("Select an available save slot:\n");
    for (int i = 1; i <= 8; i++) {
        if (tankIndex[i-1] == 0) {
            printf("Slot %d: Available\n", i);
        } else {
            printf("Slot %d: Occupied (TankID %d)\n", i, tankIndex[i-1]);
        }
    }
    printf("Enter slot number (1-8): ");
    if (scanf("%d", &indexPosition) != 1 || indexPosition < 1 || indexPosition > 8 || tankIndex[indexPosition-1] != 0) {
        printf("Invalid or occupied slot. Try again.\n");
        empty_stdin();
        return tank_setup();
    } else {
        tankIndex[indexPosition-1] = tankId;  // Store tankId in the slot
        printf("TankID %d assigned to slot %d\n", tankId, indexPosition);
    }
    // UPDATE: Set currentTank in JSON
    FILE *fp = fopen("tank_settings.json", "r+");
    if (fp != NULL) {
        char line[512], temp_file[50];
        FILE *tmp = fopen("tank_settings.json.tmp", "w");
        
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "\"currentTank\"")) {
                fprintf(tmp, "        \"currentTank\": \"%d\",\n", tankNumber);
            } else {
                fprintf(tmp, "%s", line);
            }
        }
        fclose(fp);
        fclose(tmp);
        remove("tank_settings.json");
        rename("tank_settings.json.tmp", "tank_settings.json");
    }
    

    printf("Is the tank rectangular or cylindrical?\n0 for rectangular\n1 for cylindrical: ");
    if (scanf("%d", &cylinderFlag) != 1) {
        printf("Invalid input. Try again.\n");
        empty_stdin();
        return tank_setup();
    }
    empty_stdin();
    
    if (cylinderFlag == 0) {
        printf("\nRectangular tank selected.\n\n");
        printf("Please enter tank dimensions in cm (length, width, height):\n");
        printf("\nLength (cm):  ");
        scanf("%d", &tankLength);
        empty_stdin();

        printf("\nWidth (cm):   ");
        scanf("%d", &tankWidth);
        empty_stdin(); 

        printf("\nHeight (cm):  ");
        scanf("%d", &tankHeight);
        empty_stdin();
        
        printf("\nCalculating maximum volume...\n");
        maxVolume = tankLength * tankWidth * tankHeight;
        printf("Maximum tank volume: %d cm³ (%.2f liters)\n", maxVolume, maxVolume / 1000.0);

        printf("\nRectangular tank selected.\n\nDimensions:\nLength: %d cm\nWidth: %d cm\nHeight: %d cm\n Max volume: %d cubiccm\n (%.2f liters)\n", tankLength, tankWidth, tankHeight, maxVolume, maxVolume / 1000.0);

        printf("\nTank setup complete, saving to tank config...\n\n");
        write_tank_dimensions(0, tankLength, tankWidth, tankHeight, indexPosition, maxVolume);

    }
    else if (cylinderFlag == 1) {
        printf("\nCylindrical tank selected.\n\n");
        printf("Please enter tank dimensions in cm (diameter, height):\n");
        printf("Diameter (cm):   ");
        scanf("%d", &tankDiameter);
        empty_stdin();

        printf("\nHeight (cm):   ");
        scanf("%d", &tankHeight);
        empty_stdin();

        printf("\nCalculating maximum volume...\n");
        maxVolume = (int)(3.14159 * (tankDiameter / 2.0) * (tankDiameter / 2.0) * tankHeight);
        printf("Maximum tank volume: %d cm³ (%.2f liters)\n", maxVolume, maxVolume / 1000.0);

        printf("\nCylindrical tank selected.\n\nDimensions: \nDiameter: %d cm \nHeight: %d cm\n Max volume: %d cubiccm\n (%.2f liters)\n", tankDiameter, tankHeight, maxVolume, maxVolume / 1000.0);
        printf("\nTank setup complete, saving to tank config...\n\n");
        write_tank_dimensions(1, tankDiameter, tankHeight, 0, indexPosition, maxVolume);
    }
    else {
        printf("\nInvalid choice. Try again.\n\n");
        return tank_setup();
    }
}


// Function to write graph configuration to JSON
void write_tank_dimensions(int tank_type, int length, int width, int height, int indexPosition, int maxVolume) {
    FILE *fp_read = fopen("tank_settings.json", "r");
    FILE *fp_write = fopen("tank_settings.json.tmp", "w");

    if (fp_read == NULL || fp_write == NULL) {
        printf("Error: Cannot open tank_settings.json\n\nRetrying tank setup...\n\n");
        if (fp_read) fclose(fp_read);
        if (fp_write) fclose(fp_write);
        tank_setup();
        return;
    }

    char line[512];
    char slotId[32];
    snprintf(slotId, sizeof(slotId), "\"slot%d\":", indexPosition);

    int skipping_target_slot = 0;
    int brace_balance = 0;

    // Values to write
    int liters = maxVolume / 1000;
    int rect_w = 0, rect_l = 0, rect_h = 0;
    int cyl_d = 0, cyl_h = 0;
    int is_last_slot = (indexPosition == 8);

    if (tank_type == 0) {
        // rectangular
        rect_w = width;
        rect_l = length;
        rect_h = height;
        cyl_d = 0;
        cyl_h = 0;
    } else {
        // cylindrical: function is called with (length=diameter, width=height)
        cyl_d = length;
        cyl_h = width;
        rect_w = rect_l = rect_h = 0;
    }

    while (fgets(line, sizeof(line), fp_read)) {
        if (!skipping_target_slot && strstr(line, slotId)) {
            // We found the slot header line for the target; start skipping original block
            // Initialize brace balance using this line to catch opening brace
            for (char *p = line; *p; ++p) {
                if (*p == '{') brace_balance++;
                else if (*p == '}') brace_balance--;
            }
            skipping_target_slot = 1;

            // Write replacement slot block with the same structure and key order
            fprintf(fp_write, "    \"slot%d\": {\n", indexPosition);
            fprintf(fp_write, "        \"tankId\": \"%d\",\n", tankNumber);
            fprintf(fp_write, "        \"nameCsv\": \"data_tank%d.csv\",\n", tankNumber);

            if (tank_type == 0) {
                fprintf(fp_write, "        \"tankType\": {\"0\": \"rectangular\"},\n");
            } else {
                fprintf(fp_write, "        \"tankType\": {\"1\": \"cylindrical\"},\n");
            }

            fprintf(fp_write, "        \"cylindrical\": {\"diameter\": \"%d\", \"height\": \"%d\"},\n", cyl_d, cyl_h);
            fprintf(fp_write, "        \"rectangular\": {\"width\": \"%d\", \"length\": \"%d\", \"height\": \"%d\"},\n", rect_w, rect_l, rect_h);
            fprintf(fp_write, "        \"volume\": {\"maxCcm\": \"%d\", \"maxLiters\": \"%d\"}\n", maxVolume, liters);
            if (!is_last_slot) {
                fprintf(fp_write, "    },\n");
            } else {
                fprintf(fp_write, "    }\n");
            }
            // Do not write the original header line; continue to skip the original slot contents
            continue;
        }

        if (skipping_target_slot) {
            // Keep skipping until we close the slot object we started on
            for (char *p = line; *p; ++p) {
                if (*p == '{') brace_balance++;
                else if (*p == '}') brace_balance--;
            }
            if (brace_balance <= 0) {
                // Finished skipping the original slot block
                skipping_target_slot = 0;
            }
            continue; // skip writing original lines of the target slot
        }

        // Non-target lines are copied as-is
        fputs(line, fp_write);
    }

    fclose(fp_read);
    fclose(fp_write);

    remove("tank_settings.json");
    rename("tank_settings.json.tmp", "tank_settings.json");

    printf("Parameters updated successfully\n\nTank%d established as current connection\n\n", tankNumber);
    printf("Continuing to serial port setup..\n\n");
}

void read_tank_settings(void) {
    FILE *fp = fopen("tank_settings.json", "r");
    
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open tank_settings.json for reading\n");
        printf("\nResetting to tank settings...\n\n");
        reset_tank_settings();
        return;
    }
    
    char line[512];
    int slot = -1;
    int tankId = tankNumber;
    
    // Read and populate tankIndex array
    while (fgets(line, sizeof(line), fp)) {
        // Parse slot number and tankId
        if (sscanf(line, "    \"slot%d\": {", &slot) == 1) {
            // Next line should contain tankId
            if (fgets(line, sizeof(line), fp)) {
                if (sscanf(line, "        \"tankId\": \"%d\"", &tankId) == 1) {
                    if (slot >= 1 && slot <= 8) {
                        tankIndex[slot - 1] = tankId;
                        if (tankId != 0) {
                            printf("Loaded: Slot %d -> TankID %d\n", slot, tankId);
                        }
                    }
                }
            }
        }
        
        // Parse currentTank from index
        if (strstr(line, "\"currentTank\"")) {
            if (sscanf(line, "        \"currentTank\": \"%d\"", &tankId) == 1) {
                tankNumber = tankId;
                if (tankId != 0) {
                    printf("Current Tank: Tank%d\n", tankId);
                }
            }
        }
    }
    
    fclose(fp);
    printf("\nTank settings loaded successfully.\n\n");
}
//===========================================================================// Menu display functions

void display_menu(void) {
    printf("\n\n==== Main Menu ===\n");
    printf("1. Start serial reading\n");
    printf("2. Analyze tank data\n");
    printf("3. Graph menu\n");
    printf("4. Tank settings\n");
    printf("5. Exit\n");
    printf("=================\n\n");
    printf("\nEnter choice: ");
}

void graph_Menu(void) {
    printf("\n\n=== Graph Menu ===\n");
    printf("Display graph for average volume content \nand average rate of change over time.\nPlease select analysis period:\n");
    printf("1. Daily\n");
    printf("2. Weekly\n");
    printf("3. Yearly \n");
    printf("4. Custom period\n");
    printf("5. Exit\n");
    printf("=================\n\n");
    printf("\nEnter choice: ");
}

void tank_settings(void) {
    printf("\n\n=== Tank Settings ===\n");
    printf("1. Tank setup\n");
    printf("2. Manual purge\n");
    printf("3. View tank index\n");
    printf("4. Return to main menu\n");
    printf("=====================\n\n");
}

//===========================================================================// File handling functions

void analyze_csv(const char *tankCsv) {
FILE *fp = fopen(tankCsv, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open %s for reading\n", tankCsv);
        return;
    }

    char line[256];
    double sum = 0;
    double first_value = 0;
    double last_value = 0;
    int count = 0;
    time_t first_timestamp = 0;
    time_t last_timestamp = 0;

    fgets(line, sizeof(line), fp); // skip header

    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ";");
        if (token == NULL) continue;

        time_t timestamp = (time_t)atoll(token);
        token = strtok(NULL, ";");
        if (token == NULL) continue;

        double value = atof(token);

        if (count == 0) {
            first_value = value;
            first_timestamp = timestamp;
        }
        last_value = value;
        last_timestamp = timestamp;
        sum += value;
        count++;
    }

    fclose(fp);

    if (count > 0) {
        double average = sum / count;
        double rate_of_change = 0;
        if (count > 1) {
            rate_of_change = (last_value - first_value) / (difftime(last_timestamp, first_timestamp) + 1);
        }

        printf("\n===== Current Tank Metrics =====\n");
        printf("Total data entries: %d\n", count);
        printf("Average tank volume: %.2f L\n", average);
        printf("Average rate of change: %.4f L/hour\n", (rate_of_change*3600)); // per hour
        printf("Latest measurement: %.2f L (%s)\n", last_value, ctime(&last_timestamp));     
        printf("================================\n\n");
        printf("press 'q' to return to main menu...\n\n");
        if (_kbhit()) {
            if (_getch() == 'q') {
                printf("Returning to main menu...\n\n");
                display_menu();
            }
        }
    } else {
        printf("No data found in CSV file.\n\n");
    }
}


//===========================================================================// Utility functions

void reset_tank_settings(void) {
    FILE *fp = fopen("tank_settings.json", "w");
    
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open tank_settings.json for writing\n");
        return;
    }
    
    // Write default tank_settings.json structure matching the template exactly
    fprintf(fp, "{\n");
    fprintf(fp, "    \"index\": {\n");
    fprintf(fp, "        \"currentTank\": \"0\",\n");
    fprintf(fp, "        \"previousTank\": \"0\"\n");
    fprintf(fp, "    },\n");
    
    for (int i = 1; i <= 8; i++) {
        fprintf(fp, "    \"slot%d\": {\n", i);
        fprintf(fp, "        \"tankId\": \"0\",\n");
        fprintf(fp, "        \"nameCsv\": \"data_tank0.csv\",\n");
        fprintf(fp, "        \"tankType\": {\"0\": \"rectangular\"},\n");
        fprintf(fp, "        \"cylindrical\": {\"diameter\": \"0\", \"height\": \"0\"},\n");
        fprintf(fp, "        \"rectangular\": {\"width\": \"0\", \"length\": \"0\", \"height\": \"0\"},\n");
        fprintf(fp, "        \"volume\": {\"maxCcm\": \"0\", \"maxLiters\": \"0\"}");
//            "slot2": {
//        "tankId": "0",
//        "nameCsv": "data_tank0.csv",
//        "tankType": {"0": "rectangular"},
//        "cylindrical": {"diameter": "0", "height": "0"},
//        "rectangular": {"width": "0", "length": "0", "height": "0"},
//        "volume": {"maxCcm": "0", "maxLiters": "0"}
        if (i < 8) {
            fprintf(fp, "    },\n");
        } else {
            fprintf(fp, "    }\n");
        }
    }
    
    fprintf(fp, "}\n");
    fclose(fp);
    
    for (int i = 0; i < 8; i++) {
        tankIndex[i] = 0;
    }

    tankNumber = -1;
    
    printf("\ntank_settings.json has been reset to default state.\n");
    printf("tankIndex array cleared.\n\n");
    printf("resuming setup\n\n");
}

// purge stdin buffer
void empty_stdin (void){
    int c = getchar();
    while (c != '\n' && c != EOF)
    c = getchar();
}

void open_graph_image(const char *graph_type) {
    char imagePath[256];
    
    // Match Python naming: {graph_type}_tank{tankNumber}.png
    snprintf(imagePath, sizeof(imagePath), "%s_tank%d.png", graph_type, tankNumber);
    
    // Check if file exists
    FILE *check = fopen(imagePath, "r");
    if (check) {
        fclose(check);
    } else {
        printf("Error: Graph file '%s' not found.\n", imagePath);
        printf("Please run the Python grapher first.\n\n");
        return;
    }
    
    // Open with default image viewer
    char command[512];
    snprintf(command, sizeof(command), "start %s", imagePath);
    system(command);
    
    printf("Opening graph: %s\n\n", imagePath);
}
//===========================================================================// Main function

int main() {
    
    int initChoice = -1;
    printf("Initializing...\n");
    printf("Select option: \n\n0. Use existing tank settings\n1. Reset tank settings\n\n");
    if (scanf("%d", &initChoice) != 1) {
        fputs("Invalid input. Try again.\n\n", stderr);
        empty_stdin();

        initChoice = 1;
    }
    if (initChoice == 1) {
        reset_tank_settings();
    }
    else {
        printf("\n\nAttempting to load existing tank settings...\n\n");
        read_tank_settings();

    }
    if (tankNumber == -1) {
        fprintf(stderr, "Tank not configured. Please complete tank setup...\n");
        tank_setup();
        read_tank_settings();
    }

//Buffer size definitions
    #define BUFFER_SIZE 256
    #define LINE_SIZE 256


//Establish data storage file

    int tankName = tankNumber;
    char tankCsv[50];
    snprintf(tankCsv, sizeof(tankCsv), "data_tank%d.csv", tankName);
    
    FILE *fpt;    //file pointer variable
    fpt = fopen(tankCsv, "a+");   //open file in append mode

    if (fpt == NULL) {
        fprintf(stderr, "Error: failed to establish data file link\n");
        return 1;
    }

    fseek(fpt, 0, SEEK_END);
    long size = ftell(fpt);


//Serial port handle definition setup

    HANDLE hSerial;
    unsigned long bytesRead;
    char buffer[BUFFER_SIZE];
    char serialline[LINE_SIZE];
    int idx = 0;


//Serial port opening and settings

    int ComPortInput = -1;
    char portName[32];
    
    while(1){
        printf("Enter the number of the connected COM port:\n");
        if (scanf("%d", &ComPortInput) != 1) {
            fputs("Invalid input. Try again:\n", stderr);
            empty_stdin();
            continue;
        }
        if (ComPortInput >= 0) break;
    }

//Serial communication parameters

    sprintf(portName, "\\\\.\\COM%d", ComPortInput);
        hSerial = CreateFile(
        portName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

//Failed connection error report

    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening port %s\nExiting program\n\n", portName);
        fclose(fpt);
        return 1;
    }

    DCB dcbSerialParams = {0};

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error obtaining comm state\nExiting program\n\n");
        CloseHandle(hSerial);
        fclose(fpt);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
    dcbSerialParams.XonChar  = 0x11;
    dcbSerialParams.XoffChar = 0x13;

    COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;

    if(!SetCommTimeouts(hSerial, &timeouts)){
        fprintf(stderr, "Error setting timeouts.\nExiting program\n\n");
        CloseHandle(hSerial);
        fclose(fpt);
        return 1;
    }
    

//Successful connection report

    printf("Connection established on port %s\n", portName);


// Main user control loop

    int readyFlag = -1;
    int choice = -1;

    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            fputs("Invalid input. Try again.\n", stderr);
            empty_stdin();
            display_menu();
            continue;
        }
        empty_stdin();

        if (choice == 1) {
            readyFlag = 1;
            printf("Press 'q' to stop reading and return to menu...\n\n");
            printf("Reading serial input...\n\n");
            printf("==== Incoming Data ===\n");
            while (readyFlag == 1) {
    // Serial read loop
                if (!ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) {
                    fprintf(stderr, "ReadFile failed or connection closed\n\n");
                    break;        
                }
                if (size == 0) {
                    fprintf(fpt, "timestamp;dVperiodical;dVcompelled\n");
                    size = 1;       
                }     
                     
                if (_kbhit()) {
                    if (_getch() == 'q') {
                        readyFlag = 0;
                        printf("==== Data Recording halted ====\n\n");
                        printf("Returning to main menu...\n\n");
                        display_menu();
                        break;
                    }
                }
                else for (unsigned long i = 0; i < bytesRead; i++) {
                    char c = buffer[i];
                    
                    // Skip carriage return
                    if (c == '\r') {
                        continue;
                    }

                    // Check if measurement type is 'C' or 'P'
                    // C = compelled data collection - user started pump
                    // P = periodical data collection - automatic every ~10 minutes
                    if (c == '\n' && (serialline[0] == 'C' || serialline[0] == 'P')) {
                        serialline[idx] = '\0';  // Null terminate the string
                        time_t now = time(NULL);
                        printf("Data received: %s\n", serialline);

                        char *value_ptr = strchr(serialline, ';');
                        char *value = (value_ptr != NULL) ? value_ptr + 1 : "0";
                        
                        // Write to CSV based on type
                        if (serialline[0] == 'P') {
                            fprintf(fpt, "%lld;%s;\n", (long long)now, value);
                        } else if (serialline[0] == 'C') {
                            fprintf(fpt, "%lld;;%s\n", (long long)now, value);
                        }
                        fflush(fpt);
                        idx = 0;  // Reset for next serialline
                    } 
                    else if (c == '\n') {
                        // Newline - end of serialline, but only save if we got C or P
                        continue;
                    }
                    else {
                        // Iterate idx counter and check for buffer overflow
                        if (idx < (int)sizeof(serialline) - 1) {
                            serialline[idx++] = c;
                        }
                        else {
                            serialline[LINE_SIZE-1] = '\0';
                            fprintf(stderr, "Error: serialline buffer overflow. Data: %s\n", serialline);
                            idx = 0;
                        }
                    }
                }
            }
        if (readyFlag == 0) {
            display_menu(); 
     }
        } else if (choice == 2) {
            analyze_csv(tankCsv);
        } else if (choice == 3) {
            graph_Menu();
            int graphChoice = -1;
            if (scanf("%d", &graphChoice) != 1) {
                fputs("\nInvalid input. Try again.\n\n", stderr);
                empty_stdin();
                graph_Menu();
            }
            empty_stdin();
            
            if (graphChoice == 1) {
                write_daily_config();
                printf("\nDaily graph config saved.\n");
                printf("Generating graph...\n");
                system("python python_grapher.py");  // Run Python script
                open_graph_image("daily");  // Open the generated PNG
            } else if (graphChoice == 2) {
                write_weekly_config();
                printf("\nWeekly graph config saved.\n");
                printf("Generating graph...\n");
                system("python python_grapher.py");
                open_graph_image("weekly");
            } else if (graphChoice == 3) {
                write_yearly_config();
                printf("\nYearly graph config saved.\n");
                printf("Generating graph...\n");
                system("python python_grapher.py");
                open_graph_image("yearly");
            } else if (graphChoice == 4) {
                char start_date[20], end_date[20];
                printf("\n\nEnter period start date (YYYY-MM-DD): ");
                scanf("%19s", start_date);
                empty_stdin();
                printf("\n\nEnter period end date (YYYY-MM-DD): ");
                scanf("%19s", end_date);
                empty_stdin();
                write_graph_config(start_date, end_date, "custom");
                printf("\nCustom graph config saved.\n");
                printf("Generating graph...\n");
                system("python python_grapher.py");
                open_graph_image("custom");
            }
            else {
                printf("Invalid input. Try again.\n");
            }
        } else if (choice == 4) {
            tank_settings();
            int tankChoice = -1;
            if (scanf("%d", &tankChoice) != 1) {
                fputs("\nInvalid input. Try again.\n\n", stderr);
                empty_stdin();
                tank_settings();                
            }
            empty_stdin();
            
            if (tankChoice == 1) {
                tank_setup();
            } 
            else if (tankChoice == 2) {
                printf("Are you sure you want to purge tank? Y/N\n");
                char purgeConfirm = getchar();
                if (purgeConfirm == 'Y' || purgeConfirm == 'y') {
                    printf("\nOopsie! Function not implemented yet. We apologize! Returning to main menu...\n\n");
                    display_menu();
                } else {
                    printf("\nPurge cancelled. Returning to main menu...\n\n");
                    display_menu();
                } 
                empty_stdin();
            } 
            else if (tankChoice == 3) {
                printf("\n\n=== Tank Index ===\n");
                
                // Read tank_settings.json
                FILE *settings_fp = fopen("tank_settings.json", "r");
                if (settings_fp == NULL) {
                    fprintf(stderr, "\nError: Cannot open tank_settings.json\n\nReturning to main menu...\n\n");
                } else {
                    char line[512];
                    
                    printf("\nSlot Configuration:\n");
                    for (int slot = 1; slot <= 8; slot++) {
                        if (tankIndex[slot-1] == 0) {
                            printf("Slot %d: Available\n", slot);
                        } else {
                            printf("Slot %d: TankID %d\n", slot, tankIndex[slot-1]);
                        }
                    }
                    
                    printf("\nTank Dimensions:\n");
                    
                    int current_tank = -1;
                    double diameter = 0, height = 0, width = 0, length = 0;
                    
                    while (fgets(line, sizeof(line), settings_fp)) {
                        // Parse slot number
                        if (sscanf(line, "    \"slot%d\":", &current_tank) == 1) {
                            // Check if this slot has a tank assigned
                            if (tankIndex[current_tank-1] != 0) {
                                printf("\nSlot %d (TankID %d):\n", current_tank, tankIndex[current_tank-1]);
                            }
                        }
                        
                        // Parse dimensions
                        sscanf(line, "            \"diameter\": \"%lf\"", &diameter);
                        sscanf(line, "            \"height\": \"%lf\"", &height);
                        sscanf(line, "            \"width\": \"%lf\"", &width);
                        sscanf(line, "            \"length\": \"%lf\"", &length);
                        
                        // Print tank info when we hit the end of a tank section
                        if (current_tank > 0 && strstr(line, "    }") && !strchr(line, '{')) {
                            if (tankIndex[current_tank-1] != 0) {
                                // Determine tank type and calculate max volume
                                if (diameter > 0 && height > 0) {
                                    // Cylindrical tank
                                    double radius = diameter / 2;
                                    double max_volume = 3.14159 * radius * radius * height;
                                    printf("  Type: Cylindrical\n");
                                    printf("  Diameter: %.1f cm\n", diameter);
                                    printf("  Height: %.1f cm\n", height);
                                    printf("  Max Volume: %.2f cm³ (%.2f L)\n", max_volume, max_volume / 1000);
                                } else if (length > 0 && width > 0 && height > 0) {
                                    // Rectangular tank
                                    double max_volume = length * width * height;
                                    printf("  Type: Rectangular\n");
                                    printf("  Length: %.1f cm\n", length);
                                    printf("  Width: %.1f cm\n", width);
                                    printf("  Height: %.1f cm\n", height);
                                    printf("  Max Volume: %.2f cm³ (%.2f L)\n", max_volume, max_volume / 1000);
                                }
                            }
                            // Reset for next tank
                            diameter = 0;
                            height = 0;
                            width = 0;
                            length = 0;
                            current_tank = -1;
                        }
                    }
                    
                    fclose(settings_fp);
                }
                
                printf("\n==================\n\n");
            }
            else if (tankChoice == 4) {
                printf("\nReturning to main menu...\n\n");
                display_menu();
            } 
            else {
                printf("\nInvalid choice. Try again.\n\n");
            }

        } else if (choice == 5) {
            CloseHandle(hSerial);
            fclose(fpt);
            printf("Exiting...\n");
            return 1;
        } else {
            printf("Invalid choice. Try again.\n");
            display_menu();
        }
    }    
}
