#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
 
#define left 75
#define right 77
#define up 72
#define down 80

typedef struct{
	int score;
	int speed;
}gameParam;

typedef struct{
	int x;
	int y;	
}headPosition;

typedef struct{
	int x;
	int y;
}tailPosition;

void set_console_size_title();
void hide_cursor();
void out_title_screen();
void out_game_info(gameParam take);
void head_out(COORD head);
void tail_out(COORD tail);
int out_you_win(gameParam take);
int out_game_over();
int set_food(char map[][19]);
int exit_game();
int hit_button(int &key);
int set_marker(char map[][19], int key, COORD head);
COORD get_marker(char map[][19], COORD &tail);
gameParam set_game_par();
gameParam set_new_game_param(gameParam &set_par);
headPosition lock_current_pos_head(COORD &head);
tailPosition lock_current_pos_tail(COORD &tail);
headPosition move_head(int &key, headPosition &set_pos);
 
int main(){
	
    srand(time(0));
	int x=19, y=18;													//start position
	int key;														//variable for button
				
	char map[34][19]={};											//array for marking snake position (move direction, food position)
	
	COORD head={x, y};												//snake's head('*')
	COORD tail={x, y};												//snake's tail(' ')
		
	gameParam set_par;	
		
	headPosition chp;												//current head position
	tailPosition ctp;												//current tail position										
	set_par=set_game_par();											//start parameters
	hide_cursor();
    set_console_size_title();
//--------------------------------------------------------------------------------------
//title screen
//--------------------------------------------------------------------------------------    
    out_title_screen();
//--------------------------------------------------------------------------------------
//gameplay
//--------------------------------------------------------------------------------------    
	set_food(map);													//set first item on the game-field
	do{
		out_game_info(set_par);										//game-info out
		head_out(head);												//snake head out
		chp=lock_current_pos_head(head);	
    	hit_button(key);											//hit the button to start move
    	Sleep(set_par.speed);										//set delay for snake's speed emitation
   	 	move_head(key, chp);									
    
    	if(map[chp.y-1][chp.x-1]==0 || map[chp.y-1][chp.x-1]=='i'){ //check map
			map[head.Y][head.X]=set_marker(map, key, head);		    //on head position set marker
			head.X=chp.x; 											
			head.Y=chp.y;											
			if(key==left || key==right || key==up || key==down){ 	//check button 
				if(map[head.Y-1][head.X-1]!='i'){					//check marker
			 		tail_out(tail); 								
					ctp=lock_current_pos_tail(tail);
					tail=get_marker(map, tail);						//get marker to move tail
					map[ctp.y-1][ctp.x-1]=0;						//clean marker
				}
				else{
					set_food(map);
					set_new_game_param(set_par);
				}
				head_out(head);
    		}
    	}
    	else {
    		out_game_over();
			return 0;  
		}
	out_you_win(set_par);
	}
	while(key!=27);                                             
	exit_game();
}
//-----------------------------------------------------------------------------------------------
void set_console_size_title(){
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle("SNAKE v1.0");							
	system("mode con lines=23 cols=35");					
}
//-----------------------------------------------------------------------------------------------
void hide_cursor(){
	CONSOLE_CURSOR_INFO CCI;								
    CCI.bVisible=false;
    CCI.dwSize=1;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&CCI);
}
//-----------------------------------------------------------------------------------------------
gameParam set_game_par(){
	gameParam set;
	set.score=0;
	set.speed=250;
	return set;
}
//-----------------------------------------------------------------------------------------------
void out_title_screen(){
	do{
		printf("              SNAKE!              \n      press any key to start...\n  to control snake use arrows keys\n        to quit press 'Esc'");
		Sleep(1000);
		system("cls");
	}
	while(!kbhit());
}
//------------------------------------------------------------------------------------------------
void out_game_info(gameParam take){
	COORD info={0, 19};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), info);
	printf("----------------------------------\nSCORE: %i\n", take.score);
}
//------------------------------------------------------------------------------------------------
void head_out(COORD head){
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), head);
	printf("*");
}
//------------------------------------------------------------------------------------------------
void tail_out(COORD tail){
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), tail);
	printf(" ");	
}
//------------------------------------------------------------------------------------------------
int out_you_win(gameParam take){
	if(take.score==50){
		COORD message={13, 10};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), message);
		printf("YOU WIN!\n    THANK YOU FOR PLAYNING!");
		getch();
		return 0;
	}
}
//-------------------------------------------------------------------------------------------------
int out_game_over(){
	COORD message={13, 10};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), message);
	printf("GAME OVER!");
	getch();
	return 0;
}
//-------------------------------------------------------------------------------------------------
int set_food(char map[][19]){
	COORD food={rand()% 33+1, rand()% 18+1};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), food);
    printf("o");
    return map[food.Y-1][food.X-1]='i';
}
//-------------------------------------------------------------------------------------------------
gameParam set_new_game_param(gameParam &set_par){
	set_par.score++;
	set_par.speed-=5;
	return set_par;	
}
//-------------------------------------------------------------------------------------------------
int exit_game(){
	COORD message={13, 10};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), message);
	printf("GOODBYE!"); 
getch();
}
//---------------------------------------------------------------------------------------------------
int hit_button(int &key){
	if(kbhit()){ 
		key=getch(); 
		if(key==224){
			key=getch();
		}
	}
	return key;
}
//---------------------------------------------------------------------------------------------------
headPosition move_head(int &key, headPosition &set_pos){
	    switch (key){
       	 case left:
        	if(set_pos.x>0){
		    	set_pos.x--;
            }
        	else{
        		out_game_over();
        	}
        break;
        case right:
        	if(set_pos.x<35){
		    	set_pos.x++;
            }
        	else{
        		out_game_over();
        		}
        break;
        case up:
    		if(set_pos.y>0){
		    	set_pos.y--;
           	}
        	else{
        		out_game_over();
        	}
        break;
        case down:
    		if(set_pos.y<19){
		    	set_pos.y++;
           	}
        	else{
        		out_game_over();
        	}
        break;
    }
return set_pos;
}
//---------------------------------------------------------------------------------------------------
headPosition lock_current_pos_head(COORD &head){
	headPosition chp;
	chp.x=head.X;
	chp.y=head.Y;
	return chp;
}
//----------------------------------------------------------------------------------------------------
int set_marker(char map[][19], int key, COORD head){
	switch(key){ 
			case left:
				map[head.Y-1][head.X-1]='l';
			break;	
			case right:
				map[head.Y-1][head.X-1]='r';
			break;	
			case up:
				map[head.Y-1][head.X-1]='u'; 
			break;	
			case down:
				map[head.Y-1][head.X-1]='d';
			break;	
		}
	return map[head.Y][head.X];
}
//----------------------------------------------------------------------------------------------------
tailPosition lock_current_pos_tail(COORD &tail){
	tailPosition ctp;
	ctp.x=tail.X;
	ctp.y=tail.Y;
	return ctp;	
}
//----------------------------------------------------------------------------------------------------
COORD get_marker(char map[][19], COORD &tail){
	switch(map[tail.Y-1][tail.X-1]){ 
		case 'l':
			if(tail.X>0){
				tail.X--;
			}
		break;
		case 'r':
			if(tail.X<35){
				tail.X++;
			}
		break;
		case 'u':
			if(tail.Y>0){
				tail.Y--;
			}
		break;
		case 'd':
			if(tail.Y<19){
				tail.Y++;
			}
		break;
	}
return tail;
}
//-----------------------------------------------------------------------------------------------------

