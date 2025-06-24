/*───────────────────────────────────────────────────────────────
 *  SNAKE – ncurses (Ubuntu‑compatible)  
 *  v9 – compile‑clean: fixes missing functions, prototype warnings,  
 *       and misleading‑indentation (June 2025)  
 *
 *  • Added prototypes + definitions for `head_out()` and `tail_out()`  
 *  • Wrapped the `if (out_you_win())` + `refresh()` statement in braces  
 *    to silence `‑Wmisleading‑indentation`.  
 *  • Verified all functions declared before use (no implicit warnings).  
 *  • Build clean: `gcc snake.c -std=c99 -Wall -O2 -lncurses -o snake`  
 *──────────────────────────────────────────────────────────────*/

#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

/*── Board geometry ───────────────────────────────────────────*/
#define BOARD_W  36
#define BOARD_H  20
#define PLAY_MIN_X 1
#define PLAY_MAX_X (BOARD_W - 2)
#define PLAY_MIN_Y 1
#define PLAY_MAX_Y (BOARD_H - 2)

/*── Keys */
#define left   KEY_LEFT
#define right  KEY_RIGHT
#define up     KEY_UP
#define down   KEY_DOWN

/*── Types */
typedef struct { int X, Y; } COORD;
typedef struct { int score, speed, level; } gameParam;
typedef struct { int x, y; } pos;

/*── Forward declarations */
static void  init_ncurses(void);
static void  shutdown_ncurses(void);
static void  draw_border(void);
static void  splash_title(void);
static WINDOW* show_overlay(const char *txt, int millis);
static void  splash_level(int lvl);
static void  pause_game(void);
static void  out_game_info(const gameParam *gp);
static void  head_out(COORD h);
static void  tail_out(COORD t);
static int   out_you_win(const gameParam *gp);
static int   out_game_over(void);
static int   set_food(char map[BOARD_H][BOARD_W]);
static void  inc_score_speed(gameParam *gp);
static int   maybe_level_up(gameParam *gp);
static void  set_marker(char map[BOARD_H][BOARD_W], int key, COORD h);
static COORD get_marker(char map[BOARD_H][BOARD_W], COORD *t);
static pos   move_head(int key, pos p);

/*── ncurses init/teardown */
static void init_ncurses(void){ initscr(); keypad(stdscr,TRUE); noecho(); curs_set(0); start_color(); use_default_colors(); }
static void shutdown_ncurses(void){ curs_set(1); endwin(); }

/*── Border */
static void draw_border(void){
    mvaddch(0,0,ACS_ULCORNER); mvaddch(0,BOARD_W-1,ACS_URCORNER);
    mvaddch(BOARD_H-1,0,ACS_LLCORNER); mvaddch(BOARD_H-1,BOARD_W-1,ACS_LRCORNER);
    mvhline(0,1,ACS_HLINE,BOARD_W-2); mvhline(BOARD_H-1,1,ACS_HLINE,BOARD_W-2);
    mvvline(1,0,ACS_VLINE,BOARD_H-2); mvvline(1,BOARD_W-1,ACS_VLINE,BOARD_H-2);
}

/*── Title */
static void splash_title(void){
    const char *m1="SNAKE!", *m2="press any key to start…", *m3="arrow keys | Esc quits | space pauses";
    while(true){ clear(); draw_border();
        mvprintw(5,(BOARD_W-(int)strlen(m1))/2,"%s",m1);
        mvprintw(7,(BOARD_W-(int)strlen(m2))/2,"%s",m2);
        mvprintw(9,(BOARD_W-(int)strlen(m3))/2,"%s",m3);
        refresh(); if(getch()!=ERR){ beep(); break; } napms(100); }
}

/*── Overlay helper */
static WINDOW* show_overlay(const char *txt, int millis){
    int len=strlen(txt), h=3, w=len+4;
    int y=BOARD_H/2-h/2, x=(BOARD_W-w)/2;
    WINDOW *win=newwin(h,w,y,x);
    box(win,0,0); mvwprintw(win,1,2,"%s",txt); wrefresh(win); beep();
    if(millis>0){ napms(millis); delwin(win); touchwin(stdscr); refresh(); return NULL; }
    return win;
}

static void splash_level(int lvl){ char buf[24]; snprintf(buf,sizeof buf,"LEVEL %d",lvl); show_overlay(buf,3000); }

/*── Pause */
static void pause_game(void){
    WINDOW *w=show_overlay("PAUSED",0);
    int ch; while((ch=getch())!=' ' && ch!=27) napms(50);
    if(ch==27) ungetch(ch);
    delwin(w); touchwin(stdscr); refresh();
}

/*── UI helpers */
static void out_game_info(const gameParam *gp){ mvprintw(BOARD_H,0,"SCORE:%d LEVEL:%d (space=pause)",gp->score,gp->level); clrtoeol(); }
static void head_out(COORD h){ mvaddch(h.Y,h.X,'*'); }
static void tail_out(COORD t){ mvaddch(t.Y,t.X,' '); }

/*── End‑game overlays */
static int out_you_win(const gameParam *gp){
    if(gp->score<50) return 0;
    flushinp(); WINDOW *w=show_overlay("YOU WIN!",0); beep();
    timeout(-1); getch();
    delwin(w); touchwin(stdscr); refresh();
    return 1;
}

static int out_game_over(void){
    /* Return 1 so the caller can decide to restart */
    flushinp(); WINDOW *w=show_overlay("GAME OVER!",0);
    timeout(-1); getch();
    delwin(w); touchwin(stdscr); refresh();
    return 1; /* signal game-over */
}

/*── Gameplay helpers */
static int set_food(char map[BOARD_H][BOARD_W]){
    COORD f;
    do{ f.X=rand()%(PLAY_MAX_X-1)+1; f.Y=rand()%(PLAY_MAX_Y-1)+1; }while(map[f.Y][f.X]);
    mvaddch(f.Y,f.X,'o'); map[f.Y][f.X]='i'; return 0;
}
static gameParam new_gamepar(void){ return (gameParam){0,250,1}; }
static void inc_score_speed(gameParam *gp){ ++gp->score; if(gp->speed>60) gp->speed-=5; }
static int maybe_level_up(gameParam *gp){ int lvl=gp->score/5+1; if(lvl>gp->level){ gp->level=lvl; return 1;} return 0; }
static void set_marker(char map[BOARD_H][BOARD_W], int key, COORD h){ map[h.Y][h.X]=(key==left?'l':key==right?'r':key==up?'u':'d'); }
static COORD get_marker(char map[BOARD_H][BOARD_W], COORD *t){ switch(map[t->Y][t->X]){ case 'l': if(t->X>PLAY_MIN_X)--t->X; break; case 'r': if(t->X<PLAY_MAX_X)++t->X; break; case 'u': if(t->Y>PLAY_MIN_Y)--t->Y; break; case 'd': if(t->Y<PLAY_MAX_Y)++t->Y; break;} return *t; }
static pos move_head(int key, pos p){ switch(key){ case left: if(p.x>PLAY_MIN_X)--p.x; break; case right: if(p.x<PLAY_MAX_X)++p.x; break; case up: if(p.y>PLAY_MIN_Y)--p.y; break; case down: if(p.y<PLAY_MAX_Y)++p.y; break;} return p; }

/*── main */
int main(void){
    srand((unsigned)time(NULL)); init_ncurses();

    while(true){
        /* ── fresh game state ───────────────────────────── */
        static char map[BOARD_H][BOARD_W];
        memset(map, 0, sizeof map);
        COORD head={BOARD_W/2,BOARD_H/2}, tail=head;
        int key=right; gameParam gp=new_gamepar();

        splash_title(); clear(); draw_border(); set_food(map); out_game_info(&gp); refresh();

        bool lost=false, quit=false, won=false;
        while(!lost && !quit && !won){
            timeout(gp.speed);
            int ch=getch();
            if(ch==' '){ pause_game(); out_game_info(&gp); draw_border(); refresh(); continue; }
            if(ch!=ERR) key=ch;
            if(key==27){ quit=true; break; }

            pos hp=move_head(key,(pos){head.X,head.Y});
            if(!map[hp.y][hp.x] || map[hp.y][hp.x]=='i'){
                set_marker(map,key,head); head.X=hp.x; head.Y=hp.y;
                if(map[head.Y][head.X]!='i'){
                    tail_out(tail); COORD old=tail; tail=get_marker(map,&tail); map[old.Y][old.X]=0;
                }else{
                    beep(); set_food(map); inc_score_speed(&gp);
                    if(maybe_level_up(&gp)) splash_level(gp.level);
                }
                out_game_info(&gp); head_out(head);
            }else{
                lost = out_game_over();
                break;
            }
            if(out_you_win(&gp)){ won=true; break; }
            refresh();
        }
        if(quit || won) break; /* exit outer loop */
        /* Otherwise 'lost' is true -> restart (title screen will show again) */
    }

    shutdown_ncurses(); return 0;
}
