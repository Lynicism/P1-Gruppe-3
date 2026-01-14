#include <windows.h> // Giver adgang til Windows-funktioner (fx COM-porte)
#include <stdio.h>
#include <time.h>  // Bruges til tid og dato


char date[20]; // Buffer der kan gemme en dato som tekst

int main() {

    FILE *fpt;    // Opretter en fil-pointer
    fpt = fopen("MyFile.csv", "a+");   // Åbner (eller opretter) CSV-filen i "append"-tilstand. "a+" betyder: tilføj data uden at slette det gamle
    fseek(fpt, 0, SEEK_END); // Går til slutningen af filen
    long size = ftell(fpt); // Finder størrelsen af filen

    HANDLE hSerial; // Håndtag til seriel port (COM-port)
    unsigned long bytesRead; // Antal bytes læst fra COM-porten
    char buffer[256]; // Buffer til at gemme data læst fra COM-porten
    char line[256]; // Buffer til at samle en hel linje
    int idx = 0; // Index til at holde styr på position i line-bufferen

    hSerial = CreateFile(// Åbner COM3-porten
        "\\\\.\\COM3", // COM-port (Arduino er tilsluttet her)
        GENERIC_READ, // Kun læsning
        0, // Ingen deling
        NULL, 
        OPEN_EXISTING, // Porten skal allerede eksistere
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {// Hvis COM-porten ikke kunne åbnes
        printf("Error opening COM port\n");
        return 1;
    }

    DCB dcbSerialParams = {0}; // Struktur til seriel port konfiguration(vi sætter alle verdier til 0 først)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams); // Sætter længden af strukturen
    GetCommState(hSerial, &dcbSerialParams); // Henter nuværende konfiguration af COM-porten

    dcbSerialParams.BaudRate = CBR_9600; // Sætter baudrate til 9600
    dcbSerialParams.ByteSize = 8; // 8 databits
    dcbSerialParams.StopBits = ONESTOPBIT; // 1 stopbit
    dcbSerialParams.Parity   = NOPARITY; // Ingen paritet(ingen fejlkontrol)
    SetCommState(hSerial, &dcbSerialParams); // Anvender den nye konfiguration

    printf("Reading from COM3...\n");

    if (size == 0){ // Hvis filen er tom
    fprintf(fpt,"timestamp;ammount\n"); // Skriver CSV-header (kolonnenavne)
    }
    while (1) { // Uendeligt loop – programmet stopper aldrig
        if (ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) { // Læser data fra COM-porten
            for (unsigned long i = 0; i < bytesRead; i++) { // Går gennem hver læst byte
                char c = buffer[i]; //gemmer en karakter ad gangen
 
                if (c == '\n') {   // Hvis vi har nået slutningen af en linje
                    line[idx] = '\0'; // Afslutter strengen


                    char dateStr[20];     // Buffer til dato-streng
                    time_t now = time(NULL);     // Henter nuværende tid

                    printf("message: %s\n", line); // Udskriver den modtagne linje til konsollen
                    fprintf(fpt, "%lld;%s\n", (long long)now, line); // Skriver tidsstempel og data til CSV-filen
                    

                    idx = 0; // Nulstiller index for næste linje
                } else { // Hvis det ikke er slutningen af en linje
                    line[idx] = c; // Tilføjer karakteren til line-bufferen
                    idx++;     // Går videre til næste position
                    if (idx >= sizeof(line)) idx = 0; // to avoid overflow
                }
            }
        }
    }

    CloseHandle(hSerial); // Lukker COM-porten
    return 0;
}


