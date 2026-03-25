#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// keeping track of pings
typedef struct {
    float radius;
} Ping;
typedef struct{
    int x, y;
    int active;
    double spawnTime, pinged, spawnedAt; // timer and stopwatch
    char name[22];
} Blip;

// prototypes
void checkPing(double *lastSpawn, int *pingCount, Ping *ping);
void expandPing(Ping *ping, int *pingCount, float maxRadius);
void crosshair(int centerY, int centerX, struct winsize w);
void drawPing(int centerX, int centerY, int pingCount, Ping *ping, struct winsize w);
void checkBlip(struct winsize w, Blip *blips, double *currentTime);
void drawBlip(int centerX, int centerY, int pingCount, Ping *ping, struct winsize w, Blip *blips, double *currentTime);
void drawMarkers(struct winsize w, float maxRadius, int centerY, int centerX);
void drawRings(int centerX, int centerY, float maxRadius, struct winsize w);
#define MAX_PINGS 10
#define MAX_BLIPS 5
#define M_PI 3.14159265358979323846 // for pi

int main() {
    // declaring values
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); // windows dimensions
    int centerX = w.ws_col / 2; // setting stuff to the middle
    int centerY = w.ws_row / 2;
    Ping ping[MAX_PINGS]; // ping pong ping also this is an array of type ping lol
    Blip blips[MAX_BLIPS];
    int pingCount = 0;
    double lastSpawn = 0;
    float maxRadius = sqrt( // deciding what the max radius is
        (centerX * centerX) +
        (centerY * centerY * 4)
    );

    char radar_char = 'O'; // character to represent middle of radar
    memset(blips, 0, sizeof(blips)); // this resets every data to 0 to avoid garbage
    srand(time(NULL)); // setting rand to null so it starts differently every time

    while (1) {
        printf("\033[H\033[J"); // clear once
        printf("\033[32m"); //  prints everything green (if terminal stuck in green, run 'reset')
        printf("\033[?25l"); // hide cursor
        struct timeval tv; // creates container to use gettimeofday
        gettimeofday(&tv, NULL); // takes current, individual time
        double currentTime = tv.tv_sec + tv.tv_usec / 1000000.0;
        //calling functions in order
        // spawn logic
        checkPing(&lastSpawn, &pingCount, ping);
        //check blips
        // update circles
        expandPing(ping, &pingCount, maxRadius);
        // draw crosshair
        crosshair(centerY, centerX, w);
        drawRings(centerX, centerY, maxRadius, w);
        drawMarkers(w, maxRadius, centerY, centerX);
        // draw pings
        drawPing(centerX, centerY, pingCount, ping, w);
        // check blips
        checkBlip(w, blips, &currentTime);
        drawBlip(centerX, centerY, pingCount, ping, w, blips, &currentTime); // ping and blips are arrays, they dont need pointers
        // move cursor to middle and print radar_char (O)
        printf("\033[%d;%dH%c", centerY, centerX, radar_char); 
        fflush(stdout); // flush the output to ensure it appears immediately
        usleep(120000); // sleep for 120ms before updating the radar (like FPS)
    }
}

void checkPing(double *lastSpawn, int *pingCount, Ping *ping){
    //checks if it can spawn a circle
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double currentTime = tv.tv_sec + tv.tv_usec / 1000000.0;
    if (currentTime - *lastSpawn >= 4.0) { // after 4 seconds spawn a circle
        if (*pingCount < MAX_PINGS) {
            ping[*pingCount].radius = 0;
            (*pingCount)++;
        }
        *lastSpawn = currentTime;
    }
}

void expandPing(Ping *ping, int *pingCount, float maxRadius){

        // expand older circle
    for (int i = 0; i < *pingCount; i++) {
        ping[i].radius += 1.5;
    }
    int newCount = 0;
    for (int i = 0; i < *pingCount; i++) {
        if (ping[i].radius < maxRadius) {
            ping[newCount++] = ping[i];
        }
    }
    *pingCount = newCount;


}

void crosshair(int centerY, int centerX, struct winsize w){
        // horizontal line
    for (int x = 1; x <= w.ws_col; x++) {
        printf("\033[%d;%dH-", centerY, x);
    }
    // vertical line
    for (int y = 1; y <= w.ws_row; y++) {
        printf("\033[%d;%dH|", y, centerX);
    }
}

void drawRings(int centerX, int centerY, float maxRadius, struct winsize w){ // making static rings
    float radii[3] = {maxRadius * 0.25, maxRadius * 0.5, maxRadius * 0.75}; 
    for (int y = 1; y <= w.ws_row; y++) { // for each row
        for (int x = 1; x <= w.ws_col; x++) { // for each colums
            float dx = x - centerX;
            float dy = (y - centerY) * 2; // fixing oval distortion and defining centers
            float dist = sqrt(dx * dx + dy * dy);
            for (int i = 0; i < 3; i++){ // making at least 3 circles
                if (fabs(dist - radii[i]) < 0.5) { // print while the thickness is less than 0.5 (?)
                    printf("\033[%d;%dH.", y, x);
                }
            }
        }
    }
}

void drawMarkers(struct winsize w, float maxRadius, int centerY, int centerX){
    double angle = M_PI / 180.0;
    for (int i = 0; i < 360; i += 180){
        double x = centerX + (maxRadius * 0.9) * cos(i * angle); // formula to find horizontal angle of the circles
        double y = centerY + (maxRadius * 0.9) * sin(i * angle) / 2;
        printf("\033[%d;%dH%d", (int)y, (int)x, i);
    }
}

void drawPing(int centerX, int centerY, int pingCount, Ping *ping, struct winsize w){
    // draw circles/pings
    for (int y = 1; y <= w.ws_row; y++) { // while it hasnt reached max rows
        for (int x = 1; x <= w.ws_col; x++) { // while it hasnt reached max columns
            float dx = x - centerX;
            float dy = (y - centerY) * 2; // fix oval distortion
            float dist = sqrt(dx * dx + dy * dy); // calculate distance
            // check all pings
            for (int i = 0; i < pingCount; i++) {
                if (fabs(dist - ping[i].radius) < 1.2) {
                    printf("\033[%d;%dH.", y, x);
                }
            }
        }
    }
}

void checkBlip(struct winsize w, Blip *blips, double *currentTime) {
    typedef struct { char *name; int weight; } BlipName;
    BlipName nameTable[] = {
        {"[SUB]", 10}, {"[SHIP]", 10}, {"[MINE]", 10},
        {"[UNK]", 10}, {"[TGT]", 10}, {"[WRECK]", 10},
        {"[WHALE]", 10}, {"[DIVER]", 8}, {"[SEAMOUNT]", 8},
        {"[DECOY]", 8}, {"[BOMB]", 5}, {"[GHOST]", 5},
        {"[TORPEDO]", 2}, {"[SOS]", 2},
        {"[JESUS?]", 1}, {"[O_o]", 1}, {"[GARGANTUAN-LEVIATHAN]", 1}
    };
    int nameCount = sizeof(nameTable) / sizeof(nameTable[0]); // auto count names
    int totalWeight = 0; // start at 0
    for (int j = 0; j < nameCount; j++) totalWeight += nameTable[j].weight; // sum all weights
    for (int i = 0; i < MAX_BLIPS; i++){
        // if blip wasnt active before, give it a random cooldown between 5 and 15
        if (blips[i].active == 0 && blips[i].spawnTime == 0)
            blips[i].spawnTime = *currentTime + (rand() % 11 + 5);

        // after 15 seconds theyll be gone
        if (blips[i].active == 1 && (*currentTime - blips[i].spawnedAt) > 15.0){
            blips[i].active = 0;
            blips[i].pinged = 0;
            blips[i].spawnTime = 0;
            blips[i].spawnedAt = 0;
            continue;
        }

        // activates a blip
        if (*currentTime < blips[i].spawnTime || blips[i].active == 1) continue;
        blips[i].active = 1;
        blips[i].x = rand() % (w.ws_col - 4) + 5; // using x and y from the struct itself
        blips[i].y = rand() % (w.ws_row - 4) + 5;
        blips[i].pinged = 0;
        blips[i].spawnedAt = *currentTime; // starts the 15 second countdown

        // roll a random name based on weight
        int roll = rand() % totalWeight;
        int cumulative = 0;
        for (int j = 0; j < nameCount; j++){
            cumulative += nameTable[j].weight;
            if (roll < cumulative){ strncpy(blips[i].name, nameTable[j].name, 22); break; }
        }
    }
}

void drawBlip(int centerX, int centerY, int pingCount, Ping *ping, struct winsize w, Blip *blips, double *currentTime){
    for (int i = 0; i < MAX_BLIPS; i++){
        if (blips[i].active == 1){
            int dx = blips[i].x - centerX; // calculates dx (i think it's the diagonal x wait no that doesnt make sense maybe a second temp x)
            int dy = (blips[i].y - centerY) * 2;
            float blipdistance = sqrt(pow(dx, 2) + pow(dy, 2)); 
            for (int j = 0; j < pingCount; j++){ // not MAX_PINGS because it doesnt take all pings at once, only current ones
                if (fabs(blipdistance - ping[j].radius) < 1.0){ // checks if distance between them is less than 1, also fabs doesn't allow negative numbers
                    blips[i].pinged = *currentTime;
                }
            } // the job of the J for loop is to check if ping hit the blip
            if ((*currentTime - blips[i].pinged) < 1.0){
                printf("\033[%d;%dH%c", blips[i].y, blips[i].x, '@'); // prints strongest blip
                printf("\033[%d;%dH%s", blips[i].y - 1, blips[i].x - 1, blips[i].name); // grabs a random name from the array of strings
            }
            else if ((*currentTime - blips[i].pinged) < 2.0){
                printf("\033[%d;%dH%c", blips[i].y, blips[i].x, 'o'); // prints mid
                printf("\033[%d;%dH%s", blips[i].y - 1, blips[i].x - 1, blips[i].name);
            }
            else if ((*currentTime - blips[i].pinged) < 4.0){
                printf("\033[%d;%dH%c", blips[i].y, blips[i].x, '.'); // prints weak
                printf("\033[%d;%dH%s", blips[i].y - 1, blips[i].x - 1, blips[i].name);
            }
            else{
                printf("\033[%d;%dH ", blips[i].y, blips[i].x); // prints no blip temporarily til ping hits again
            }
        }   
    }
}

// Hi
