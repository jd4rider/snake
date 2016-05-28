#include <stdio.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
#define MAX_SIZE 800

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

struct Position
{
    int x;
    int y;
};

int snake_size = 0;
int speed = 200;
char snake_char_first = 'O';
char snake_char = 'o';
char snake_char_tail = '.';
char food = '@';
int screen_width = 80;
int screen_height = 80;
int direction;
struct Position pos[MAX_SIZE];
struct Position food_position;
struct termios orig_term_attr;
struct termios new_term_attr;
int quit = 0;

int rand_lim(int limit)
{
    int divisor = RAND_MAX/(limit+1);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

int fetch_key()
{
    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    return fgetc(stdin);
}

void fatal(char *reason)
{
    printf("FATAL: %s\n", reason);
    exit(1);
}

void randomize_food()
{
    food_position.x = rand_lim(screen_width);
    food_position.y = rand_lim(screen_height);
}

int check_snake_collision(struct Position px)
{
    int remaining = -1;
    while (remaining++ < snake_size)
    {
        if (pos[remaining].x == px.x && pos[remaining].y == px.y)
            return 1;
    }

    return 0;
}

void generate_food()
{
    randomize_food();
    while (check_snake_collision(food_position))
        randomize_food();
    // draw it
    gotoxy(food_position.x, food_position.y);
    printf("%c", food);
}

void game_over()
{
    gotoxy(screen_width / 2, screen_height / 2);
    printf("* GAME OVER *");
    gotoxy((screen_width / 2) - 10, (screen_height / 2) + 2);
    printf("Press x to exit or n for new game");
    int key = -1;
    while (key == -1)
    {
        key = fetch_key();
        usleep(200);
    }
    printf("%i", key);
    quit = true;
}

void move_snake()
{
    int add_x = 0;
    int add_y = 0;
    switch (direction)
    {
        case RIGHT:
            add_x = 1;
            break;
        case UP:
            add_y = 1;
            break;
        case DOWN:
            add_y = -1;
            break;
        case LEFT:
            add_x = -1;
            break;
    }

    struct Position head = pos[0];
    struct Position tail = pos[snake_size];

    // replace tail with space
    gotoxy(tail.x, tail.y);
    printf(" ");

    // replace head with normal
    gotoxy(head.x, head.y);
    printf("%c", snake_char);

    // shift the snake
    int remaining = snake_size + 1;
    while (--remaining > 0)
    {
        pos[remaining].x = pos[remaining-1].x;
        pos[remaining].y = pos[remaining-1].y;
    }
    pos[0].x += add_x;
    pos[0].y += add_y;

    // check if we fucked up
    if (

    // make new head
    gotoxy(pos[0].x, pos[0].y);
    printf("%c", snake_char_first);
    // make new tail
    gotoxy(pos[snake_size].x, pos[snake_size].y);
    printf("%c", snake_char_tail);
    gotoxy(0, 0);
    fflush(stdout);
}

void draw_snake()
{
    int remaining = -1;
    while (remaining++ < snake_size)
    {
        char symbol = snake_char;
        if (remaining == 0)
            symbol = snake_char_first;
        else if (remaining == snake_size)
            symbol = snake_char_tail;
        // go to position
        gotoxy(pos[remaining].x, pos[remaining].y);
        printf("%c", symbol);
    }
    fflush(stdout);
}

void play()
{
    while(!quit)
    {
        usleep(speed * 1000);
        int key = fetch_key();
        if (key != -1)
        {
            switch (key)
            {
                case 65:
                    direction = DOWN;
                    break;
                case 66:
                    direction = UP;
                    break;
                case 67:
                    direction = RIGHT;
                    break;
                case 68:
                    direction = LEFT;
                    break;
                case 120:
                    quit = 2;
                    break;
            }
        }
        if (!quit)
            move_snake();
    }
}

void new_game()
{
    snake_size = 3;
    // reset for debugging
    int current_pos = -1;
    while (current_pos++ < MAX_SIZE)
    {
        pos[current_pos].x = -10;
        pos[current_pos].y = -10;
    }
    // create new snake in middle of screen
    int start_x = screen_width / 2;
    int start_y = screen_height / 2;
    // generate snake
    int remaining = -1;
    while (remaining++ < snake_size)
    {
       if (start_x < 0)
           fatal("The screen is too small");
       pos[remaining].x = start_x--; 
       pos[remaining].y = start_y; 
    }
    direction = RIGHT;
    clear();
    generate_food();
    draw_snake();
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
    while (!quit)
    {
        new_game();
        play();
    }
    /* restore the original terminal attributes */
    clear();
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
    return 0;
}
