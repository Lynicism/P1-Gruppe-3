#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>



//======================================== Forward declarations ========================================================//

//C kræver at alle definerede funktioner deklareres enten i starten af programmet eller i slutningen,
//så længe de er udenfor main loop

//Hovedmenu funktion
void display_menu(void);

//Tøm brugerinput hukommelse
void empty_stdin(void);

//Grafmenu
void graph_Menu(void);

//Tankindstillinger menu
void tank_settings(void);

//Analyser .csv filen for den nuværende tilsluttede tank
void analyze_csv(const char *tankCsv);

//Funktion som opdaterer filen "graph_config.json" med valgte graftype samt startdato og slutdato for det tidsrum der ønskes en graf over
void write_graph_config(const char *start_date, const char *end_date, const char *graph_type);

//Funktion som opdaterer "graph_config.json" med nuværende dato
void write_daily_config(void);

//Funktion som opdaterer "graph_config.json" med forrige uge fra nuværende dato
void write_weekly_config(void);

//Funktion som opdaterer "graph_config.json" med forrige år fra nuværende dato
void write_yearly_config(void);

//Funktion som lagrer information om tanken i "tank_settings.json" når en ny tank tilføjes
void write_tank_dimensions(int tank_type, int length, int width, int height,int indexPosition, int maxVolume);

//Funktion som begynder opsætningen af en ny tank
void tank_setup(void);

//Funktion som åbner grafbilledet som python programmet generer
void open_graph_image(const char *graph_type);

//Funktion som læser den tilsluttede tanks oplysninger og printer dem i konsollen
void read_tank_settings(void);

//Funktion som nulstiller filen "tank settings"
void reset_tank_settings(void);



//======================================== Global Variables ========================================================//

// Variable som flere funktioner bruger og derfor er defineret globalt

//interger værdi som oplyser hvilken tank programmet har indlæst på nuværende tidspunkt:
// -1 = ikke oplyst (programmet er netop initialiseret) 1-8 = numerisk navn for tank
int tankNumber = -1;

//Index for hvilke tanke programmet har adgang til at tilgå. bruges til at give hver tank en 'plads'
int tankIndex[8] = {0,0,0,0,0,0,0,0}; 

//char array som bruges til at indikere den valgte graftype
char graphType[256];

//integer værdi som oplyser om serieport er tilsluttet: 
//-1 = ikke oplyst (programmet er netop initialiseret) 0 = ikke tilsluttet 1 = tilsluttet
int comConnection = -1;



//======================================== File Dependency Functions ========================================================//

//Funktion som opdaterer filen "graph_config.json" med valgte graftype samt startdato og slutdato
//for det tidsrum der ønskes en graf over
//funktionen bruger parametrene start_date, end_date og graph_type som input. Parametrene defineres udfra hvilken
//funktion eksekveres i grafmenuen (daglig, ugentlig, årlig eller brugerdefineret)

void write_graph_config(const char *start_date, const char *end_date, const char *graph_type) {

//kør tank setup hvis ingen nuværende tank
    if (tankNumber == -1) {
        tank_setup();
    }    

//lav en fil ved navn "graph_config.json" med størrelse:
    char tankFile[50];
    snprintf(tankFile, sizeof(tankFile), "graph_config.json");
    
//åbn filen
    FILE *fp = fopen(tankFile, "w");

//hvis filen ikke kan åbnes, sender programmet til funktion "tank_setup()"
    if (fp == NULL) {
        fprintf(stderr, "\nError: Cannot write to graph config file\nRetrying tank setup...\n\n");
        tank_setup();
    }

//i filen tilføjes nødvendige informationer som python programmet skal bruge til at vide hvilket tidsrum der skal laves en graf over
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



//======================================== Write Graph Functions ========================================================//


//Funktionen opdaterer "graph_config.json" med nuværende dato
void write_daily_config(void) {

//hent nuværende dato fra UNIX epoch
    time_t now = time(NULL);
//formater datoen til YYYY-MM-DD
    struct tm *tm_info = localtime(&now);
//opret en char array til datoen
    char date[20];
//lav en string med formateret dato
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
//kald funktionen "write_graph_config" med dagens dato som parametre og "daily" som graftype
    write_graph_config(date, date, "daily");
}

//Funktionen opdaterer "graph_config.json" med forrige uge fra nuværende dato
void write_weekly_config(void) {

//se "write_daily_config" for kommentarer
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
//beregn datoen for forrige uge
    time_t week_ago = now - (7 * 86400);
//formater datoen til YYYY-MM-DD
    struct tm *tm_week = localtime(&week_ago);

//opret en char array til datoerne
    char start_date[20], end_date[20];
//lav strings med formaterede start- og slutdato
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", tm_week);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
//kald funktionen "write_graph_config" med de beregnede datoer som parametre og "weekly" som graftype
    write_graph_config(start_date, end_date, "weekly");
}

//Funktionen opdaterer "graph_config.json" med forrige år fra nuværende dato
void write_yearly_config(void) {

//se "write_daily_config" for kommentarer
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
//beregn datoen fra forrige år 
    time_t year_ago = now - (365 * 86400);
//formater datoen til YYYY-MM-DD
    struct tm *tm_year = localtime(&year_ago);
//opret en char array til datoerne
    char start_date[20], end_date[20];
//lav strings med formaterede start- og slutdato
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", tm_year);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
//kald funktionen "write_graph_config" med de beregnede datoer som parametre og "yearly" som graftype
    write_graph_config(start_date, end_date, "yearly");
}


//======================================== Tank Setup Functions ========================================================//

// Funktionen begynder opsætningen af en ny tank
void tank_setup(void) { 

//opret nødvendige variable
    int tankLength = 0, tankWidth = 0, tankHeight = 0, tankDiameter = 0; 
    int maxVolume = 0;
//indexPosition bruges til at vælge en ledig 'plads' i tankIndex arrayet
    int indexPosition = -1;
//cylinderFlag bruges til at vælge tanktype
//0 = rektangulær tank, 1 = cylindrisk tank
    int cylinderFlag = -1;
//tankId bruges til at vælge et numerisk ID for tilsluttet tanken
    int tankId = 0;  // Add tankId variable

    printf("\n=== Tank Setup ===\n");
    printf("Provide a numerical identifier for this tank (1-8):\n");

//hvis brugeren indtaster ugyldigt ID, genstartes funktionen
    if (scanf("%d", &tankId) != 1 || tankId <= 0 || tankId > 8) {
        printf("Invalid ID. Try again.\n");
        empty_stdin();
        return tank_setup();
    }

//global variabel tankNumber opdateres til at matche tankId
    else {
        printf("Tank ID: %d\n\n", tankId);
        tankNumber = tankId;  // Set tankNumber to tankId
    }
    empty_stdin();
    
//brugeren bedes om at vælge en ledig plads i tankIndex arrayet hvor nuværende tank kan gemmes
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

//opdater tankIndex arrayet med tankId på den valgte plads
    } else {
        tankIndex[indexPosition-1] = tankId;
        printf("TankID %d assigned to slot %d\n", tankId, indexPosition);
    }

//åbn tank_settings.json
    FILE *fp = fopen("tank_settings.json", "r+");
//hvis filen kan åbnes, laves en midlertidig fil "tank_settings.json.tmp" 
    if (fp != NULL) {
        char line[512];
        FILE *tmp = fopen("tank_settings.json.tmp", "w");
//hvis midlertidig fil ikke kan oprettes, lukkes tank_settings.json
        if (tmp == NULL) {
            fprintf(stderr, "Error: Cannot create temporary file\n");
            fclose(fp);
//hvis midlertidig fil oprettes, søges der i tank_settings.json efter en string med "currentTank"
        } else {
            while (fgets(line, sizeof(line), fp)) {
//når "currentTank" findes, overskrives linjen med nuværende tankId
                if (strstr(line, "\"currentTank\"")) {
                    fprintf(tmp, "        \"currentTank\": \"%d\",\n", tankNumber);
                } else {
                    fprintf(tmp, "%s", line);
                }
            }
            fclose(fp);
            fclose(tmp);
//den gamle tank_settings.json slettes og tank_settings.json.tmp filen omdøbes til tank_settings.json
            if (remove("tank_settings.json") != 0) {
                fprintf(stderr, "Error: Failed to remove old tank_settings.json\n");
                remove("tank_settings.json.tmp");
            } else if (rename("tank_settings.json.tmp", "tank_settings.json") != 0) {
                fprintf(stderr, "Error: Failed to rename temporary file\n");
                remove("tank_settings.json.tmp");
            }
        }
    }
    
//brugeren bedes vælge tanktype. inputtet definerer cylinderFlag variablen
    printf("Is the tank rectangular or cylindrical?\n0 for rectangular\n1 for cylindrical: ");
    if (scanf("%d", &cylinderFlag) != 1) {
        printf("Invalid input. Try again.\n");
        empty_stdin();
        return tank_setup();
    }
    empty_stdin();

//hvis cylinderFlag er 0, bedes brugeren indtaste længde, bredde og højde for rektangulær tank
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
        
//beregn maksimal volumen for tanken
        printf("\nCalculating maximum volume...\n");
        maxVolume = tankLength * tankWidth * tankHeight;
        printf("Maximum tank volume: %d cubiccm (%.2f liters)\n", maxVolume, maxVolume / 1000.0);

        printf("\nRectangular tank selected.\n\nDimensions:\nLength: %d cm\nWidth: %d cm\nHeight: %d cm\n Max volume: %d cubiccm\n (%.2f liters)\n", tankLength, tankWidth, tankHeight, maxVolume, maxVolume / 1000.0);
//kald funktionen "write_tank_dimensions" med relevante parametre
        printf("\nTank setup complete, saving to tank config...\n\n");
        write_tank_dimensions(0, tankLength, tankWidth, tankHeight, indexPosition, maxVolume);

    }
//hvis cylinderFlag er 1, bedes brugeren indtaste diameter og højde for cylindrisk tank
    else if (cylinderFlag == 1) {
        printf("\nCylindrical tank selected.\n\n");
        printf("Please enter tank dimensions in cm (diameter, height):\n");
        printf("Diameter (cm):   ");
        scanf("%d", &tankDiameter);
        empty_stdin();

        printf("\nHeight (cm):   ");
        scanf("%d", &tankHeight);
        empty_stdin();

//beregn maksimal volumen for tanken
        printf("\nCalculating maximum volume...\n");
        maxVolume = (int)(3.14159 * (tankDiameter / 2.0) * (tankDiameter / 2.0) * tankHeight);
        printf("Maximum tank volume: %d cm³ (%.2f liters)\n", maxVolume, maxVolume / 1000.0);

//kald funktionen "write_tank_dimensions" med relevante parametre
        printf("\nCylindrical tank selected.\n\nDimensions: \nDiameter: %d cm \nHeight: %d cm\n Max volume: %d cubiccm\n (%.2f liters)\n", tankDiameter, tankHeight, maxVolume, maxVolume / 1000.0);
        printf("\nTank setup complete, saving to tank config...\n\n");
        write_tank_dimensions(1, tankDiameter, tankHeight, 0, indexPosition, maxVolume);
    }
//hvis cylinderFlag er hverken 0 eller 1, genstartes funktionen
    else {
        printf("\nInvalid choice. Try again.\n\n");
        return tank_setup();
    }
//Kør append_csv hvis nuværende tank findes

    if (tankNumber != 0) {
        printf("Updating csv file\n");
        system("python append_csv.py");
        if (system("python append_csv.py") != 0) {
            fprintf(stderr, "\nError: Failed to run append_csv.py\n\n");
        }
    }
}

//funktionen lagrer information om tanken i "tank_settings.json" når en ny tank tilføjes
void write_tank_dimensions(int tank_type, int length, int width, int height, int indexPosition, int maxVolume) {
  
//åbn tank_settings.json til læsning  
    FILE *fp_read = fopen("tank_settings.json", "r");
//opret midlertidig fil til skrivning
    FILE *fp_write = fopen("tank_settings.json.tmp", "w");

//hvis filerne ikke kan åbnes, genstartes tank setup
    if (fp_read == NULL || fp_write == NULL) {
        printf("Error: Cannot open tank_settings.json\n\nRetrying tank setup...\n\n");
        if (fp_read) fclose(fp_read);
        if (fp_write) fclose(fp_write);
        tank_setup();
        return;
    }
//forbered hukommelse til linjer og slotId string
    char line[512];
    char slotId[32];
    snprintf(slotId, sizeof(slotId), "\"slot%d\":", indexPosition);

//variable bruges til at styre spring i filen
    int skipping_target_slot = 0;
    int brace_balance = 0;

//variable som indeholder tank dimensioner og volumen i liter samt om sidste tankplads er nået
    int liters = maxVolume / 1000;
    int rect_w = 0, rect_l = 0, rect_h = 0;
    int cyl_d = 0, cyl_h = 0;
    int is_last_slot = (indexPosition == 8);

//hvis første parameter er 0, er tanken rektangulær og dimensionsparametre anordnes herefter
    if (tank_type == 0) {
        rect_w = width;
        rect_l = length;
        rect_h = height;
        cyl_d = 0;
        cyl_h = 0;
//hvis første parameter er 1, er tanken cylindrisk og dimensionsparametre anordnes herefter
    } else {
        cyl_d = length;
        cyl_h = width;
        rect_w = rect_l = rect_h = 0;
    }
//gennemgå hver linje i tank_settings.json. når den valgte plads nås, begynder programmet at tælle 
//krøllede parenteser. når en parantese begyndes, øges tælleren med 1, og når en parentes afsluttes, mindskes tælleren med 1.
//når tælleren når 0 igen, er hele tankobjektet sprunget over. Dette sker kun for den valgte plads
//således undgår programmet at skrive både den gamle og den nye tankinformation ind i tank_settings.json.tmp
    while (fgets(line, sizeof(line), fp_read)) {
        if (!skipping_target_slot && strstr(line, slotId)) {
            for (char *p = line; *p; ++p) {
                if (*p == '{') brace_balance++;
                else if (*p == '}') brace_balance--;
            }
            skipping_target_slot = 1;

//den nye tankinformation skrives ind i den midlertidige fil            
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
//for at filen kan læses korrekt, tilføjes en afsluttende krøllet parentes med eller uden komma afhængigt af om is_last_slot flaget er sat          
            if (!is_last_slot) {
                fprintf(fp_write, "    },\n");
            } else {
                fprintf(fp_write, "    }\n");
            }
//programmet fortsætter til næste linje
            continue;
        }
//se øvre forklaring for denne blok
        if (skipping_target_slot) {
            for (char *p = line; *p; ++p) {
                if (*p == '{') brace_balance++;
                else if (*p == '}') brace_balance--;
            }
            if (brace_balance <= 0) {
                skipping_target_slot = 0;
            }
            continue;
        }
//filens øvrige indhold kopieres uændret til den midlertidige fil
        fputs(line, fp_write);
    }
//filerne lukkes og den gamle tank_settings.json slettes og den midlertidige fil omdøbes til tank_settings.json
    fclose(fp_read);
    fclose(fp_write);

    remove("tank_settings.json");
    rename("tank_settings.json.tmp", "tank_settings.json");

    printf("Parameters updated successfully\n\nTank%d established as current connection\n\n", tankNumber);
}

//funktionen aflæser den tilsluttede tanks oplysninger fra tank_settings.json og printer dem i konsollen
void read_tank_settings(void) {

    FILE *fp = fopen("tank_settings.json", "r");

//hvis filen ikke kan åbnes, nulstilles kaldes reset_tank_settings funktionen
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open tank_settings.json for reading\n");
        printf("\nResetting to tank settings...\n\n");
        reset_tank_settings();
        return;
    }

//forbered hukommelse til linjer og variable
    char line[512];
    int slot = -1;
    int tankId = tankNumber;
    
//gennemgå hver linje i tank_settings.json
    while (fgets(line, sizeof(line), fp)) {

//søg efter "slotX" i tank_settings.json
        if (sscanf(line, "    \"slot%d\": {", &slot) == 1) {
//søg efter "tankId" i tank_settings.json og opdater tankIndex arrayet herefter
//med fundne tankID værdier på respektive pladser
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
        
//der søges efter "currentTank" i tank_settings.json. nuværende tilsluttet tank opdateres herefter
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

//funktion som tillader brugeren at skifte nuværende tank
void switchTank(void) {

//forbered hukkommelse til brugerinput
    int switchId = 0;
    printf("Select tank to switch to (available tanks): ");
//hvis brugerinput er ugyldigt eller tanken ikke er konfigureret, returneres funktionen
    if (scanf("%d", &switchId) != 1 || switchId < 1 || switchId > 8) {
        printf("Invalid Tank ID. Try again.\n");
        empty_stdin();
        return;
    }
    else if (tankIndex[switchId - 1] == 0) {
        printf("Tank ID %d is not configured. Please set up the tank first.\n", switchId);
        empty_stdin();
        return;
    }
//global variabel tankNumber opdateres med brugervalgt tankId
    else {
    tankNumber = switchId;
    empty_stdin();
    }

    FILE *fp = fopen("tank_settings.json", "r+");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open tank_settings.json\n");
        return;
    }

    char line[512];
    FILE *tmp = fopen("tank_settings.json.tmp", "w");
    if (tmp == NULL) {
        fprintf(stderr, "Error: Cannot create temporary file\n");
        fclose(fp);
        return;
    }

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

    printf("Tank switched successfully.\n\n");
    printf("Updating csv file\n");
    printf("\n\nCurrent tank is now tankID %d\n\n", tankNumber);
}

//======================================== Menu Display Functions ========================================================//

//menuvisningsfunktioner. disse printer blot menutexten i konsollen

//print hovedmenuen i konsollen
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

//print grafmenuen i konsollen
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

//print tankindstillinger menuen i konsollen
void tank_settings(void) {
    printf("\n\n=== Tank Settings ===\n");
    printf("1. Tank setup\n");
    printf("2. Manual purge\n");
    printf("3. View tank index\n");
    printf("4. Reset tank settings\n");
    printf("5. Return to main menu\n");
    printf("=====================\n\n");
}

//======================================== File Handling Functions ========================================================//

//funktionen analyserer .csv filen for den nuværende tilsluttet tank
void analyze_csv(const char *tankCsv) {
//åbn den relevante .csv fil til læsning
FILE *fp = fopen(tankCsv, "r");
//hvis filen ikke kan åbnes, sendes en fejlmeddelelse til konsollen
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open %s for reading\n", tankCsv);
        return;
    }
//forbered hukommelse til linjer
    char line[512], line_copy[512];
    
//variabler til beregning af volumen sum, dVsum sum, nuværende volumen, maksimal volumen, antal rækker samt tidsstempler
    double volumeSum = 0, dVsum = 0, currentVolume = 0, maxVolume = 0;
    int count = 0;
    time_t first_timestamp = 0, last_timestamp = 0;

//skip header linje
    fgets(line, sizeof(line), fp);

//gennemgå hver linje i .csv filen
    while (fgets(line, sizeof(line), fp)) {
        // Trim newline if present
        line[strcspn(line, "\n")] = 0;
        
//kopier linjen så strtok ikke ødelægger originalen
        strcpy(line_copy, line);
        
//split linjen ved semikolon for at få alle kolonner
        char *token;
        int col = 0;
        double dVsum_value = 0;
        
        token = strtok(line_copy, ";");
        
        while (token != NULL) {
            // Trim whitespace from token
            while (*token == ' ') token++;
            char *end = token + strlen(token) - 1;
            while (end > token && *end == ' ') end--;
            *(end + 1) = '\0';
            
            switch (col) {
                case 0: // timestamp
                    if (strlen(token) > 0) {
                        time_t ts = (time_t)atoll(token);
                        if (ts > 0) {
                            if (count == 0) first_timestamp = ts;
                            last_timestamp = ts;
                        }
                    }
                    break;
                case 1: // dVperiodical
                    if (strlen(token) > 0) {
                        dVsum_value += atof(token);
                    }
                    break;
                case 2: // dVcompelled
                    if (strlen(token) > 0) {
                        dVsum_value += atof(token);
                    }
                    break;
                case 6: // currentVolume (if available from Python processing)
                    if (strlen(token) > 0) {
                        currentVolume = atof(token);
                        volumeSum += currentVolume;
                    }
                    break;
                case 7: // maxVolume (if available from Python processing)
                    if (strlen(token) > 0) {
                        maxVolume = atof(token);
                    }
                    break;
            }
            token = strtok(NULL, ";");
            col++;
        }
        
        dVsum += dVsum_value;
        count++;
    }

    fclose(fp);

//beregn og print statistik i konsollen
    if (count > 1) {
        double averageVolume = (volumeSum > 0) ? volumeSum / count : 0;
        double timeDiff = difftime(last_timestamp, first_timestamp);
        double average_hourly_rate = 0;
        
        if (timeDiff > 0) {
            average_hourly_rate = (dVsum / timeDiff) * 3600;
        }

        printf("\n===== Current Tank Metrics =====\n");
        printf("Total data entries: %d\n", count - 1);
        printf("Average tank contents: %.2f L\n", averageVolume);
        printf("Average rate of change: %.4f L/hour\n", average_hourly_rate);
        printf("Current volume: %.2f L\n", currentVolume);
        printf("Max volume: %.2f L\n", maxVolume);
        printf("================================\n\n");
    } else {
        printf("No data found in CSV file.\n\n");
    }
}



//======================================== Reset Functions ========================================================//

//funktionen nulstiller filen "tank_settings.json" til standardtilstand (tom)
void reset_tank_settings(void) {
    FILE *fp = fopen("tank_settings.json", "w");
//hvis filen ikke kan åbnes, sendes en fejlmeddelelse til konsollen
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open tank_settings.json for writing\n");
        return;
    }
    
//skriv standardtilstandsindhold til filen
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

//nulstil global variabel tankNumber
    tankNumber = -1;
    
    printf("\ntank_settings.json has been reset to default state.\n");
    printf("tankIndex array cleared.\n\n");
}


//funktionen tømmer brugerinput hukommelse
void empty_stdin (void){
    int c = getchar();
    while (c != '\n' && c != EOF)
    c = getchar();
}

//======================================== Graph Image Function ========================================================//

//funktionen åbner grafbilledet som python programmet generer
void open_graph_image(const char *graph_type) {

//forbered hukommelse til filsti
    char imagePath[256];

//forbered filsti til grafbilledet i forhold til nuværende tanknummer og graftype
    snprintf(imagePath, sizeof(imagePath), "%s_tank%d.png", graphType, tankNumber);
    
//tjek om filen eksisterer
    FILE *check = fopen(imagePath, "r");
    if (check) {
        fclose(check);
    } else {
        printf("Error: Graph file '%s' not found.\n\n", imagePath);
        return;
    }
    
    // Open with default image viewer
    char command[512];
    snprintf(command, sizeof(command), "start %s", imagePath);
    system(command);
    
    printf("Opening graph: %s\n\n", imagePath);
}






//======================================== Main Function ========================================================//

int main() {
//når programmet startes, bedes brugeren vælge om eksisterende tankindstillinger skal bruges
//eller om filen tank_settings.json nulstilles.
    int initChoice = -1;
    printf("Initializing...\n");
    printf("Select option: \n\n0. Use existing tank settings\n1. Reset tank settings\n\n");
    if (scanf("%d", &initChoice) != 1) {
        fputs("Invalid input. Try again.\n\n", stderr);
        empty_stdin();

        initChoice = 1;
    }
//hvis brugeren vælger at nulstille tankindstillinger, kaldes reset_tank_settings funktionen
    if (initChoice == 1) {
        reset_tank_settings();
    }
//hvis brugeren vælger at bruge eksisterende tankindstillinger, kaldes read_tank_settings funktionen
    else {
        printf("\n\nAttempting to load existing tank settings...\n\n");
        read_tank_settings();

    }
//hvis read_tank_settings funktionen ikke finder en nuværende tank (tankNumber = -1), bedes brugeren gennemføre tankopsætningen
    if (tankNumber == -1) {
        fprintf(stderr, "Tank not configured. Please complete tank setup...\n");
        tank_setup();
        read_tank_settings();
    }


//definitioner for serial kommunikation
    #define BUFFER_SIZE 256
    #define LINE_SIZE 256


//opret eller åben en .csv fil for den nuværende tank
    int tankName = tankNumber;
    char tankCsv[50];
    snprintf(tankCsv, sizeof(tankCsv), "data_tank%d.csv", tankName);
    
    FILE *fpt;   
//filen åbnes i "append" tilstand, så nye data tilføjes i bunden af filen
    fpt = fopen(tankCsv, "a+");   
//hvis filen ikke kan åbnes, sendes en fejlmeddelelse til konsollen 
    if (fpt == NULL) {
        fprintf(stderr, "Error: failed to establish data file link\n");
    }
//søg til slutningen af filen
    fseek(fpt, 0, SEEK_END);
//find størrelsen af filen
    long size = ftell(fpt);


//opsætning af serial kommunikation (har ikke helt styr på den her del, har bare kigget online og taget fra tidligere kode)

    HANDLE hSerial;
    unsigned long bytesRead;
    char buffer[BUFFER_SIZE];
    char serialline[LINE_SIZE];
    int idx = 0;


//Serial kommunikations variabel så brugeres manuelt kan angive hvilken COM port der skal forbindes til
    int ComPortInput = -1;
    char portName[32];


//brugeren bedes indtaste COM port nummeret
    while(1){
        printf("Enter the number of the connected COM port:\n");
        if (scanf("%d", &ComPortInput) != 1) {
            fputs("Invalid input. Try again:\n", stderr);
            empty_stdin();
            continue;
        }
        if (ComPortInput >= 0) break;
    }

//Serial kommunikation parametre
    sprintf(portName, "\\\\.\\COM%d", ComPortInput);
        hSerial = CreateFile(
        portName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

//hvis der ikke kan oprettes forbindelse til den angivne COM port, sættes comConnection flaget til 0
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "\nError opening port %s\n", portName);
        fclose(fpt);
        comConnection = 0;
    }

    DCB dcbSerialParams = {0};

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error obtaining comm state\n");
        CloseHandle(hSerial);
        fclose(fpt);
        comConnection = 0;
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
        fprintf(stderr, "Error setting timeouts.\n");
        CloseHandle(hSerial);
        fclose(fpt);
        comConnection = 0;
    }
    else {
        comConnection = 1;
    }
    
//hvis comConnection flaget er 0, sendes en advarsel til konsollen
    if (comConnection == 0){
        printf("\n\n!WARNING! No serial communication link established. Some functions may be unavailable\n\nLoading main menu\n");
    }
//hvis comConnection flaget er 1, sendes en bekræftelse til konsollen
    else if(comConnection == 1){
        printf("Connection established on port %s\n", portName);
        comConnection = 1;
    }

//hovedmenu loop


//readyFlag bruges til at styre serial input loopet
    int readyFlag = -1;
//variable til menu valg    
    int choice = -1;

    while (1) {
//hovedmenu printes i konsollen via display_menu funktionen
        display_menu();
//hvis brugeren indtaster ugyldigt input, genstartes menuen
        if (scanf("%d", &choice) != 1) {
            fputs("Invalid input. Try again.\n", stderr);
            empty_stdin();
            display_menu();
            continue;
        }
        empty_stdin();
//menuvalg 1: start serial input læsning
        if (choice == 1) {
//hvis der ikke er oprettet forbindelse til serieporten, sendes en fejlmeddelelse til konsollen
            if (comConnection == 0){
                printf("Function unavailable: no serial communication link.\n Returning to main menu\n\n");
            }
            else{
//readyFlag sættes til 1 for at starte serial read loopet
                readyFlag = 1;
                printf("Press 'q' to stop reading and return to menu...\n\n");
                printf("Reading serial inputc...\n\n");
                printf("==== Incoming Data ===\n");
//serial input loop
                while (readyFlag == 1) {
//hvis ReadFile fejler eller forbindelsen lukkes, sendes en fejlmeddelelse til konsollen og loopet brydes
                    if (!ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) {
                        fprintf(stderr, "ReadFile failed or connection closed\n\n");
                        break;        
                    }
//hvis filen er tom, skrives CSV headeren                    
                    if (size == 0) {
                        fprintf(fpt, "timestamp;dVperiodical;dVcompelled\n");
                        size = 1;       
                    }     
//hvis bruger trykker 'q', sættes readyFlag til 0 og loopet brydes                        
                    if (_kbhit()) {
                        if (_getch() == 'q') {
                            readyFlag = 0;
                            printf("==== Data Recording halted ====\n\n");
                            printf("Updating csv and returning to main menu...\n\n");
                            system("python append_csv.py");
                            if (system("python append_csv.py") != 0) {
                                fprintf(stderr, "\nError: Failed to run append_csv.py\n\n");        
                            display_menu();
                            break;
                            }
                        }
                    }
//data fra serial buffer behandles tegn for tegn
                    else for (unsigned long i = 0; i < bytesRead; i++) {
//hent aktuelt tegn i buffer
                        char c = buffer[i];
                        
//spring "carriage return" over. dette sker fordi nogle enheder sender både '\r' og '\n' ved linjeskift
                        if (c == '\r') {
                            continue;
                        }
//hvis programmet modtager et newline tegn og den første karakter i serialline er 'P' eller 'C':
//tjek om målingstypen er 'P' eller 'C':
//P = periodical data collection - automatisk hver ~10 minutter
//C = compelled data collection - bruger startede pumpen

                        if (c == '\n' && (serialline[0] == 'C' || serialline[0] == 'P')) {
//tilføj string terminator til serialline og notér tidspunktet for modtagelse.
                            serialline[idx] = '\0'; 
                            time_t now = time(NULL);
                            printf("Data received: %s\n", serialline);
//gem den modtagede værdi som en karakterstreng. hvis der ikke findes ændringsværdi, sættes den til "0"
                            char *value_ptr = strchr(serialline, ';');
                            char *value = (value_ptr != NULL) ? value_ptr + 1 : "0";
                            
//skriv data til den relevante .csv fil afhængigt af målingstypen:
//P = skriv i dVperiodical kolonnen
//C = skriv i dVcompelled kolonnen
                            if (serialline[0] == 'P') {
                                fprintf(fpt, "%lld;%s;\n", (long long)now, value);
                            } else if (serialline[0] == 'C') {
                                fprintf(fpt, "%lld;;%s\n", (long long)now, value);
                            }
//flush filpegeren
                            fflush(fpt);
//nulstil idx tælleren og forbered til næste serialline
                            idx = 0;
                        } 
//hvis de modtagne tegn ikke er newline eller carriage return, tilføjes de til serialline bufferen
                        else if (c == '\n') {
                            continue;
                        }
                        else {
//tilføj tegn til serialline buffer og inkrementer idx tælleren. check for buffer overflow
                            if (idx < (int)sizeof(serialline) - 1) {
                                serialline[idx++] = c;
                            }
                            else {
//håndter buffer overflow ved at nulstille idx tælleren og sende en fejlmeddelelse til konsollen
                                serialline[LINE_SIZE-1] = '\0';
                                fprintf(stderr, "Error: serialline buffer overflow. Data: %s\n", serialline);
                                idx = 0;
                            }
                        }
                    }
//hvis readyFlag er 0, altså hvis brugeren har trykket 'q', brydes serial read loopet og hovedmenuen printes igen
                    if (readyFlag == 0) {
                        display_menu(); 
                    }
        }
        }

//menuvalg 2: analyser den nuværende tanks .csv fil via analyze_csv funktionen
        } else if (choice == 2) {
//kald python scriptet append_csv.py for at sikre at .csv filen er opdateret
            analyze_csv(tankCsv);

//menuvalg 3: graf menu
        } else if (choice == 3) {
            graph_Menu();
//hvis brugeren indtaster ugyldigt input, genstartes grafmenuen
            int graphChoice = -1;
            if (scanf("%d", &graphChoice) != 1) {
                fputs("\nInvalid input. Try again.\n\n", stderr);
                empty_stdin();
                graph_Menu();
            }
            empty_stdin();

//grafmenu valg 1: daglig graf
            if (graphChoice == 1) {
//kald funktionen write_daily_config. se funktionens definition for beskrivelse
                strcpy(graphType, "daily");
                write_daily_config();
                printf("\nDaily graph config saved.\n");
                printf("Generating graph...\n");
//kald python scriptet python_grapher.py
                system("python python_grapher.py"); 
//åbn det genererede grafbillede
                open_graph_image(graphType);  

//grafmenu valg 2: ugentlig graf
            } else if (graphChoice == 2) {
//kald funktionen write_weekly_config. se funktionens definition for beskrivelse
                write_weekly_config();
                strcpy(graphType, "weekly");                
                printf("\nWeekly graph config saved.\n");
                printf("Generating graph...\n");
                system("python python_grapher.py");
                open_graph_image(graphType);

//grafmenu valg 3: årlig graf
            } else if (graphChoice == 3) {
//kald funktionen write_yearly_config. se funktionens definition for beskrivelse
                write_yearly_config();
                strcpy(graphType, "yearly");
                printf("\nYearly graph config saved.\n");
                printf("Generating graph...\n");
                system("python python_grapher.py");
                open_graph_image(graphType);

//grafmenu valg 4: brugerdefineret periode  
//brugeren bedes indtaste start- og slutdato for perioden de ønsker at analysere              
            } else if (graphChoice == 4) {
                strcpy(graphType, "custom");
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
                open_graph_image(graphType);
            }
            else {
                printf("Invalid input. Try again.\n");
            }
//menuvalg 4: tankindstillinger menu
        } else if (choice == 4) {
            tank_settings();
            int tankChoice = -1;
//hvis brugeren indtaster ugyldigt input, genstartes tankindstillinger menuen
            if (scanf("%d", &tankChoice) != 1) {
                fputs("\nInvalid input. Try again.\n\n", stderr);
                empty_stdin();
                tank_settings();                
            }
            empty_stdin();
//tankindstillinger menu valg 1: tankopsætning via tank_setup funktionen
            if (tankChoice == 1) {
                tank_setup();
            } 
//tankindstillinger menu valg 2: manuel tømning af tank (ikke implementeret)
            else if (tankChoice == 2) {
                printf("Are you sure you want to purge tank? Y/N\n");
                char purgeConfirm = getchar();
                if (purgeConfirm == 'Y' || purgeConfirm == 'y') {
                    printf("\nOopsie! Function not implemented yet. We apologize! Returning to main menu...\n\n");
                } else {
                    printf("\nPurge cancelled. Returning to main menu...\n\n");
                } 
                empty_stdin();
            } 
//tankindstillinger menu valg 3: vis tankindeks
            else if (tankChoice == 3) {
                printf("\n\n=== Tank Index ===\n");
//print forbundede tanke jf. tankIndex i konsollen
                printf("\nSlot Configuration:\n");

                for (int slot = 1; slot <= 8; slot++) {
                    if (tankIndex[slot-1] == 0) {
                        printf("Slot %d: Available\n", slot);
                    }
                    else{
                        printf("Slot %d: TankID %d\n", slot, tankIndex[slot-1]);
                    }
                }
                printf("\n==================\n");
                printf("Do you want to switch current tank? (Y/N)\n\n");
                char switchConfirm = getchar();
                if (switchConfirm == 'Y' || switchConfirm == 'y') {
                    switchTank();
                } else {
                    printf("\nReturning to main menu\n\n");
                }
           }
//tankindstillinger menu valg 4: nulstil tankindstillinger via reset_tank_settings funktionen
            else if (tankChoice == 4) {
                reset_tank_settings();
            }
//tankindstillinger menu valg 5: returner til hovedmenu
            else if (tankChoice == 5) {
                printf("\nReturning to main menu\n\n");
                display_menu();
            } 
//ugyldigt tankindstillinger menu valg
            else {
                printf("\nInvalid choice. Try again.\n\n");
            }

//hovedmenu valg 5: afslut program
        } else if (choice == 5) {
            CloseHandle(hSerial);
            fclose(fpt);
            printf("Exiting\n");
            printf("Are you sure? (Y/N)\n");
            char exitConfirm = getchar();
            if (exitConfirm == 'Y' || exitConfirm == 'y') {
                return 1;
            } else {
                printf("\nExit cancelled\n\n");
            }
//hvis hovedmenu valg er ugyldigt, genstartes hovedmenuen
        } else {
            printf("Invalid choice. Try again.\n");
            display_menu();
        }
    }    
}
