#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <time.h>

struct {
    int x;
    int y;
    unsigned char brightness;
} led[5][5];

#define NUM_BUTTONS 3
#define SAVE 0
#define LOAD 1
#define CLEAR 2

const char load_text[] = "Load";
const char save_text[] = "Save";
const char clear_text[] = "Clear";
int active = -1; //It's used to say on which button the user clicked

struct {
    int x;
    int y;
    int width;
    int height;
    char* text;
    /*every button has a pointer of function
    and in init() I give a different function
    for every button and when I clicked it, it will know
    what function to call*/
    void (*action) ();
} button[NUM_BUTTONS];

//Set the brightness 0(turn off the LED's)
void clear_leds() {
    int i, j;
    for(i = 0; i < 5; ++i) {
        for(j = 0; j < 5; ++j) {
            led[i][j].brightness = 0;
        }
    }
}

//The role of this function is to save the image created by the LED's
void save_leds() {
    int x = 0;
    int y = 2;
    int i, j;
    int col;
    char name[256];
    char filename[256];
    name[0] = '\0';
    col = getmaxx(stdscr);

    attron(COLOR_PAIR(3));
    mvprintw(y, x, "Name: ");
    for(i = 0; i < col; ++i)
        mvaddch(y, x + 6 + i, ' ');
    refresh();

    // --
    curs_set(1);
    echo();

    mvscanw(y, x + 6, "%253s", name);

    curs_set(0);
    noecho();
    // --

    if(name[0] == '\0')
        goto end;

    // confirmation
    strcpy(filename, name);
    strcat(filename, ".c");

    for(i = 0; i < col; ++i)
        mvaddch(y, x + i, ' ');
    mvprintw(y, x, "Save as %s? (Y / N)", filename);

    int choice = getch();
    if(choice == 'n' || choice == 'N')
        goto end;
    // ---

    // file save
    FILE * out = fopen(filename, "w");

    fprintf(out, "const char * const %s = \"\\\n\t", name);
    for(i = 0; i < 5; ++i) {
        for(j = 0; j < 5; ++j) {
            if(j == 4) {
                if(i == 4)
                    fprintf(out, "%3d\\n\";", led[j][i].brightness);
                else
                    fprintf(out, "%3d\\n\\\n\t", led[j][i].brightness);
            }
            else
                fprintf(out, "%3d,", led[j][i].brightness);
        }
    }

    fclose(out);

    for(i = 0; i < col; ++i)
        mvaddch(y, x + i, ' '); //add a character into the given window at the given coordinates
    mvprintw(y, x, "Image saved!");
    getch();
    // ----

end:
    attroff(COLOR_PAIR(3));
    for(i = 0; i < col; ++i)
        mvaddch(y, i, ' ');
    refresh();
}

//The role of this function is to load an image which can be created by the LED'S
void load_leds() {
    int x = 0;
    int y = 2;
    int i, j;
    int col;
    char name[256];
    char temp[256];
    name[0] = '\0';
    col = getmaxx(stdscr);  //get the size of the screen

    attron(COLOR_PAIR(3));
    mvprintw(y, x, "Name: ");
    for(i = 0; i < col; ++i)
        mvaddch(y, x + 6 + i, ' '); //add a character at the given coordinates
    refresh();

    // --
    curs_set(1);  //make the cursor normal
    echo();

    mvscanw(y, x + 6, "%253s", name); //It gets the string and uses the resulting line for a scan.

    curs_set(0);  //make the cursor invisible
    noecho();
    // --

    if(name[0] == '\0')
        goto end;

    // check for file existance
    // and load
    strcpy(temp, name);
    strcat(temp, ".c");

    FILE * in = fopen(temp, "r");

    if(in == NULL) {
        for(i = 0; i < col; ++i)
            mvaddch(y, x + i, ' '); ////add a character into the given window at the given coordinates
        mvprintw(y, x, "\'%s\' is not an existing image!", name);
        getch();     //wait for user input
        goto end;
    }

    //reads the file and separates what it's useless and loads the numbers into the LED's
    i = 0; j = 0;
    while(fgetc(in) != '\t');
    while(fgets(temp, 20, in)) {
        char* buffer;
        buffer = strtok(temp, ",");
        while(buffer) {
            led[j][i].brightness = atoi(buffer);
            buffer = strtok(NULL, ",");
            j++;
        }

        j = 0;
        i++;
        fgets(temp, 5, in);  //reads the line and stores it into the string pointed by temp
        fgetc(in);
    }

    fclose(in);

    for(i = 0; i < col; ++i)
        mvaddch(y, i, ' ');  //add a character at the given coordinates
    mvprintw(y, x, "Image '%s' loaded!", name);
    print();
    refresh();
    getch();   //wait for user input
    // ----

end:
    attroff(COLOR_PAIR(3));
    for(i = 0; i < col; ++i)
        mvaddch(y, i, ' '); //add a character into the given window at the given coordinates
    refresh();
}

void init() {
    int i,j;
    int row, col;
    getmaxyx(stdscr, row, col);  //get the number of rows and columns
    col = col/2 - 6; //get on the first position
    row = row/2 - 4;

    //Set the LEDs in their position
    for(i = 0; i < 5; ++i) {
        for(j = 0; j < 5; ++j) {
            led[i][j].brightness = 0;
            led[i][j].x = col + i*3;
            led[i][j].y = row + j*2;
        }
    }

    // button init
    button[SAVE].x = 0;
    button[SAVE].y = 0;
    button[SAVE].height = 1;
    button[SAVE].text = save_text;
    button[SAVE].width = strlen(save_text);
    button[SAVE].action = save_leds;

    button[LOAD].x = button[SAVE].x + button[SAVE].width + 1;
    button[LOAD].y = 0;
    button[LOAD].height = 1;
    button[LOAD].text = load_text;
    button[LOAD].width = strlen(load_text);
    button[LOAD].action = load_leds;


    button[CLEAR].x = button[LOAD].x + button[LOAD].width + 1;
    button[CLEAR].y = 0;
    button[CLEAR].height = 1;
    button[CLEAR].text = clear_text;
    button[CLEAR].width = strlen(clear_text);
    button[CLEAR].action = clear_leds;
    // --------

    init_pair(1, COLOR_GREEN, COLOR_GREEN);
    init_pair(2, COLOR_BLACK, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    assume_default_colors(COLOR_WHITE, COLOR_CYAN);

    mouse_on(ALL_MOUSE_EVENTS);
    mouseinterval(0);
}

//Set the colors of the LEDs ON or OFF
void print() {
    int i,j;

    attron(COLOR_PAIR(2)); //switches on the attribute given to it
    for(i = 0; i < 5; ++i) {
        for(j = 0; j < 5; ++j) {
            if(led[i][j].brightness)
                attrset(COLOR_PAIR(1) | A_BOLD); //It overrides whatever attribute previously had and sets it to the new attribute
            mvaddch( led[i][j].y, led[i][j].x, 219);  //Is used to move the cursor to a given point and then print
            attrset(COLOR_PAIR(2));
        }
    }
    attroff(COLOR_PAIR(2));

     // button print
    attron(COLOR_PAIR(3));
    for(i = 0; i < NUM_BUTTONS; ++i) {
        if(active == i) {
            attron(A_REVERSE);
            mvprintw(button[i].y, button[i].x, button[i].text);
            attroff(A_REVERSE);
        }
        else
            mvprintw(button[i].y, button[i].x, button[i].text);
    }
    attroff(COLOR_PAIR(3));
    // -----
}
    //this function sets the LEDs and checks if the user clicked on a LED and set it ON/OFF
int try_toggle() {
    //mouse_x and mouse_y take from a global variable the current mouse position
    int mouse_x = Mouse_status.x;
    int mouse_y = Mouse_status.y;
    int i,j;

    //check if the user clicked in the zone which contains the LEDs
    if(mouse_x < led[0][0].x || mouse_x > led[4][0].x)
        return FALSE;
    if(mouse_y < led[0][0].y || mouse_y > led[0][4].y)
        return FALSE;
    //-------------

    i = mouse_x - led[0][0].x; //Substract the x from the first LED to move everything(now it starts from 0 to 12)
    //it calculates the index of the clicked LED
    if(i % 3 != 0)
        return FALSE;
    i = i / 3;

    j = mouse_y - led[0][0].y;
    if(j % 2 != 0)
        return FALSE;
    j = j / 2;
    //-----------

    //set the LED ON or OFF
    if(led[i][j].brightness == 0)
        led[i][j].brightness = 255;
    else
        led[i][j].brightness = 0;

    return TRUE;
}

//This function checks if the user clicked a button from the toolbar
int try_toolbar(){
    int i;
    int left, right;
    int mouse_x = Mouse_status.x;
    int mouse_y = Mouse_status.y;

    for(i = 0; i < NUM_BUTTONS; ++i) {
        left = button[i].x;
        right = left + button[i].width;
        if(mouse_x >= left && mouse_x <= right && mouse_y == button[i].y) {
             //button[i].action();
             active = i;
             return TRUE;
        }
    }
    active = -1;
    return FALSE;
}


void print_toolbar() {
    int i;
    int col, row;
    char title[] = "micro::bit";
    getmaxyx(stdscr, row, col);

    //It colors the first two rows with blue and print the title in the middle
    attron(COLOR_PAIR(3));
    for(i = 0; i < col; ++i)
        mvaddch(0, i, ' ');
    mvprintw(0, (col - strlen(title))/2, title);
    for(i = 0; i < col; ++i)
        mvaddch(1, i, ' ');
    attroff(COLOR_PAIR(3));
    //----------------
}

int main() {
    int exit = FALSE;
    int redraw = TRUE;
    int input;

    initscr();  //Start curses mode
    noecho();   //Don't echo() while we do getch
    raw();      //Line buffering disabled
    keypad(stdscr, TRUE);  //We get F1, F2, arrow keys etc.
    curs_set(0);  //Make the cursor invisible
    start_color();

    init();

    //The program is like a cycle and everytime the user clicks on something the program doesn't end.
    while(!exit) {
        if(redraw) {
            print_toolbar();
            print();
            refresh();
            redraw = FALSE;
        }
    //----

        input = getch(); //It will wait for the user to press a key and then return the corresponding integer
        //check if the user clicked
        if(input == KEY_MOUSE) {
            request_mouse_pos();
            if(BUTTON_STATUS(1) & BUTTON_PRESSED){
            redraw = try_toggle();
            if(!redraw)
                redraw = try_toolbar();
            }
            else {
                if(active != -1) {
                    button[active].action();
                    active = -1;
                }
                redraw = TRUE;
            }
        }
        if(input == 'q' || input == 'Q')
            exit = TRUE;
    }

    endwin();   //end curses mode

    return 0;
}