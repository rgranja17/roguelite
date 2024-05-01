#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ncurses.h>

#include "geracaomapa.h"
#include "state.h"
#include "globals.h"

STATE* s = NULL;
bool has_shield = false;
int shield_row;
int shield_col;
int player_hp = 100;
int mob_hp = 75;
int player_shield = 50;
bool has_trap = false;
int trap_row;
int trap_col;
bool has_sword = false;
int sword_row;
int sword_col;
int trap_coords[NUM_TRAPS][2];
int monster_row[NUM_MOBS];
int monster_col[NUM_MOBS];

/*     
?                    Rodrigo Granja
todo desenha a barra de vida
*/
void draw_health_bar() {
    // Define as coordenadas do canto inferior esquerdo para a barra de vida
    int health_bar_x = 1;
    int health_bar_y = LINES - 1;


    // Calcula o comprimento atual da barra de vida com base na vida do personagem
    int health_bar_length = (COLS - 2) * player_hp / 100 / 10;

    // Desenha o texto "Health:"
    mvprintw(health_bar_y, health_bar_x, "Health:");

    // Desenha a moldura da barra de vida
    mvprintw(health_bar_y, health_bar_x + 6, "[");
    mvprintw(health_bar_y, health_bar_x + health_bar_length + 7, "]");

    // Define a cor para a parte preenchida da barra de vida (verde)
    attron(COLOR_PAIR(4));

    // Desenha a parte preenchida da barra de vida
    for (int i = 0; i < health_bar_length; i++) {
        mvprintw(health_bar_y, health_bar_x + i + 7, " ");
    }

    attroff(COLOR_PAIR(4));

    refresh();
}

/*     
?                    David Costa
todo desenha a barra de vida dos mobs
*/
void draw_health_bar_mob() {
    // Define the coordinates for the bottom-right corner of the health bar
    int health_bar_x = COLS - 100;
    int health_bar_y = LINES - 1;

    // Calculate the current length of the health bar based on the mob's health
    int health_bar_length = (COLS - health_bar_x - 4) * mob_hp / 100 / 10;

    // Draw the text "Mob:"
    mvprintw(health_bar_y, health_bar_x, "Mob:");

    // Draw the health bar frame
    mvprintw(health_bar_y, health_bar_x + 6, "[");
    mvprintw(health_bar_y, health_bar_x + health_bar_length + 7, "]");

    // Set the color for the filled part of the health bar (green)
    attron(COLOR_PAIR(7));

    // Draw the filled part of the health bar
    for (int i = 0; i < health_bar_length; i++) {
        mvprintw(health_bar_y, health_bar_x + i + 7, " ");
    }

    attroff(COLOR_PAIR(7));

    refresh();
}

/*     
?                    Rodrigo Granja
todo desenha a barra de escudo (shield)
*/
void draw_shield_bar() {
    // Define as coordenadas do canto inferior direito para a barra de escudo
    int shield_bar_x = 30;
    int shield_bar_y = LINES - 1;

    // Calcula o comprimento atual da barra de escudo com base no escudo do personagem
    int shield_bar_length = (COLS - 2) * player_shield / 100 / 10;
    
    if (has_shield) {    
    mvprintw(shield_bar_y, shield_bar_x, "Escudo:");

    // Desenha a moldura da barra de escudo
    mvprintw(shield_bar_y, shield_bar_x + 8, "[");
    mvprintw(shield_bar_y, shield_bar_x + shield_bar_length + 9, "]");

    // Define a cor para a parte preenchida da barra de escudo (azul)
    attron(COLOR_PAIR(6));

    // Desenha a parte preenchida da barra de escudo
    for (int i = 0; i < shield_bar_length; i++) {
        mvprintw(shield_bar_y, shield_bar_x + i + 9, " ");
    }

    attroff(COLOR_PAIR(6));

    refresh();
    }
}

/*
?                Tomás Machado
todo Função alterada do codigo fonte dado na blackboard:
todo - Criação das variaveis temporarias (tempx e tempy) para fazer a verificação do proximo movimento do jogador;
todo - Colocada a restrição que a personagem não pode sair do mapa nem ir para cima das paredes, apenas mover-se no chão.
*/  
void do_movement_action(STATE* s, int dx, int dy) {
    extern char map[MAP_HEIGHT][MAP_WIDTH];

    // Calcula as coordenadas do próximo movimento do jogador
    int tempx = s->player_row + dx;
    int tempy = s->player_col + dy;

    if (map[tempx][tempy] == '#')
        return;

    // Atualiza as coordenadas do jogador
    s->player_row = tempx;
    s->player_col = tempy;


}

/*     
?                    Rodrigo Granja
todo Gera espadas no mapa
*/
void generate_sword() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    int x, y;

    do {
        x = rand() % (MAP_WIDTH - 2) + 1;
        y = rand() % (MAP_HEIGHT - 2) + 1;
    } while (map[y][x] == '#' || (s->player_row == y && s->player_col == x) ||
            (shield_row == y && shield_col == x) || (trap_row == y && trap_col == x));

    sword_row = y;
    sword_col = x;

    map[y][x] = 'E';
}

/*     
?                    Rodrigo Granja
todo Gera escudos(shields) no mapa
*/
void generate_shield() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    int x, y;

    do {
        x = rand() % (MAP_WIDTH - 2) + 1;
        y = rand() % (MAP_HEIGHT - 2) + 1;
    } while (map[y][x] == '#' || (s->player_row == y && s->player_col == x) || (trap_row == y && trap_col == x));

    shield_row = y;
    shield_col = x;

    map[y][x] = 'S';
}

/*     
?                    Rodrigo Granja
todo Gera traps no mapa
*/
void generate_trap() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    int i;

    for (i = 0; i < NUM_TRAPS; i++) {
        int x, y;

        do {
            x = rand() % (MAP_WIDTH - 2) + 1;
            y = rand() % (MAP_HEIGHT - 2) + 1;
        } while (map[y][x] == '#' || (s->player_row == y && s->player_col == x) ||
                (shield_row == y && shield_col == x) ||
                (sword_row == y && sword_col == x) ||
                (trap_coords[i][0] == y && trap_coords[i][1] == x));

        trap_coords[i][0] = y;
        trap_coords[i][1] = x;

        map[y][x] = 'T';
    }
}

/*     
?                    Rodrigo Granja e David Costa
todo Gera mobs no mapa
*/
void generate_monster() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    for (int i = 0; i < NUM_MOBS; i++) {
        int x, y;

        do {
            x = rand() % (MAP_WIDTH - 2) + 1;
            y = rand() % (MAP_HEIGHT - 2) + 1;
        } while (map[y][x] == '#' || (s->player_row == y && s->player_col == x) ||
                (shield_row == y && shield_col == x) || (trap_row == y && trap_col == x) ||
                (sword_row == y && sword_col == x) ||
                (i > 0 && monster_row[i - 1] == y && monster_col[i - 1] == x));

        monster_row[i] = y;
        monster_col[i] = x;

        map[y][x] = 'M';
    }
}


/*     
?                    Pedro Argainha e Rodrigo Granja
todo Gera a mensagem de morte final
*/
void print_death_message() {
    initscr(); // Initialize the window
    clear();

    // Set initial color and attributes
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // White text on black background
    init_pair(2, COLOR_BLUE, COLOR_BLACK); // Blue text on black background
    init_pair(3, COLOR_RED, COLOR_BLACK); // Red text on black background

    char *message = "BASTARD. YOU DIED!";

    // Flash and change color every 0.5 seconds for a total of 3 times
    for (int i = 0; i < 3; ++i) {
        int color;

        // Set the color based on the iteration
        switch (i) {
        case 0:
            color = 1; // White
            break;
        case 1:
            color = 2; // Blue
            break;
        case 2:
            color = 3; // Red
            break;
        }

        // Print the message
        attron(A_BOLD | COLOR_PAIR(color));
        mvprintw(LINES / 2, (COLS - strlen(message)) / 2, "%s", message);
        refresh();
        usleep(500000); // Wait for 0.5 seconds
        attroff(A_BOLD | COLOR_PAIR(color)); // Turn off attribute and color

        // Clear the message
        mvprintw(LINES / 2, (COLS - strlen(message)) / 2, "%*s", (int)strlen(message), "");
        refresh();
        usleep(500000); // Wait for 0.5 seconds
    }

    usleep(100000); // Wait for 0.1 seconds
    endwin(); // Close the window
}

/*     
?                    Rodrigo Granja
todo Verfifica se o próximo movimento do mob é válido
*/
bool is_valid_move(int row, int col, int monster_row[], int monster_col[], int num_mobs) {
    extern char map[MAP_HEIGHT][MAP_WIDTH];

    if (map[row][col] == '#')
        return false;

    if (row == s->player_row && col == s->player_col)
        return false;

    if (row == shield_row && col == shield_col)
        return false;

    if (row == trap_row && col == trap_col)
        return false;

    if (row == sword_row && col == sword_col)
        return false;

    for (int i = 0; i < num_mobs; i++) {
        if (row == monster_row[i] && col == monster_col[i])
            return false;
    }

    return true;
}

/*     
?                   David Costa, Rodrigo Granja e Pedro Argainha
todo Faz com que o mob persiga o jogador
todo Combate com os monstros
*/
void update_monster() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    for (int i = 0; i < NUM_MOBS; i++) {
        int dx = s->player_row - monster_row[i];
        int dy = s->player_col - monster_col[i];

        if ((abs(dx) <= 1 && abs(dy) == 0) || (abs(dx) == 0 && abs(dy) <= 1)) {
            mvprintw(LINES - 1, COLS - 150, "A Wild monster appeared, to fight him press [L] ! ");
            draw_health_bar_mob();
            refresh();

            bool fightInProgress = true;
            int mob_hp = 75; // Inicialize mob_hp com um valor adequado

            while (fightInProgress && mob_hp > 0 && player_hp > 0) {
                char key = getch();

                if (key == 'l') {
                    if (has_sword) {
                        mob_hp -= 25;
                        player_hp -= 5;
                    } else {
                        mob_hp -= 10;
                        player_hp -= 5;
                    }

                    // Deduct player's HP if no shield
                    if (has_shield) {
                        player_shield -= 10;
                    }

                    // Check if player's health reaches 0
                    if (player_hp <= 0) {
                        print_death_message();
                        endwin();
                        exit(0);
                    }

                    draw_health_bar_mob();
                    draw_health_bar();
                    refresh();
                }
            }

            if (mob_hp <= 0) {
                map[monster_row[i]][monster_col[i]] = '.'; // Remove the monster from the map
                monster_row[i] = -1; // Set monster's position as invalid
                monster_col[i] = -1;
                mvprintw(LINES - 1, COLS - 150, "Great job, you defeated the monster! Press any key to continue.");
                refresh();
                getch();  // Wait for any key press to continue
            }
        } else {
            int next_row = monster_row[i];
            int next_col = monster_col[i];

            // Verifica a direção horizontal do jogador em relação ao monstro
            if (dx > 0 && is_valid_move(monster_row[i] + 1, monster_col[i], monster_row, monster_col, NUM_MOBS)) {
                map[monster_row[i]][monster_col[i]] = '.'; // Remove o monstro atual
                next_row += 1; // Move o monstro para baixo
            } else if (dx < 0 && is_valid_move(monster_row[i] - 1, monster_col[i], monster_row, monster_col, NUM_MOBS)) {
                map[monster_row[i]][monster_col[i]] = '.'; // Remove o monstro atual
                next_row -= 1; // Move o monstro para cima
            } else {
                // Movimento bloqueado, tenta mover-se aleatoriamente na mesma direção vertical
                if (dy > 0 && is_valid_move(monster_row[i], monster_col[i] + 1, monster_row, monster_col, NUM_MOBS)) {
                    map[monster_row[i]][monster_col[i]] = '.'; // Remove o monstro atual
                    next_col += 1; // Move o monstro para a direita
                } else if (dy < 0 && is_valid_move(monster_row[i], monster_col[i] - 1, monster_row, monster_col, NUM_MOBS)) {
                    map[monster_row[i]][monster_col[i]] = '.'; // Remove o monstro atual
                    next_col -= 1; // Move o monstro para a esquerda
                } else {
                    // Ambos os movimentos estão bloqueados, não se move
                    continue;
                }
            }

            // Atualiza as coordenadas do monstro
            monster_row[i] = next_row;
            monster_col[i] = next_col;

            // Define o novo valor na matriz de mapa
            map[monster_row[i]][monster_col[i]] = 'M';
        }
    }
}

/*     
?                    Rodrigo Granja
todo Função inicial + funcionalidades de traps e shields
*/
void update(STATE* s) {
    extern char map[MAP_HEIGHT][MAP_WIDTH];

    int key = getch();
    
    mvaddch(s->player_row, s->player_col, ' ');


    switch (key) {
    case KEY_A1:
    case '7':
        do_movement_action(s, -1, -1);
        break;
    case KEY_UP:
    case '8':
        do_movement_action(s, -1, +0);
        break;
    case KEY_A3:
    case '9':
        do_movement_action(s, -1, +1);
        break;
    case KEY_LEFT:
    case '4':
        do_movement_action(s, +0, -1);
        break;
    case KEY_B2:
    case '5':
        break;
    case KEY_RIGHT:
    case '6':
        do_movement_action(s, +0, +1);
        break;
    case KEY_C1:
    case '1':
        do_movement_action(s, +1, -1);
        break;
    case KEY_DOWN:
    case '2':
        do_movement_action(s, +1, +0);
        break;
    case KEY_C3:
    case '3':
        do_movement_action(s, +1, +1);
        break;
    case 'q':
        endwin();
        exit(0);
        break;
    }
    update_monster();

        if (s->player_row == shield_row && s->player_col == shield_col) {
        if (!has_shield) {
            has_shield = true;
            s->player_shield = 100; // Update the shield to 100
        }
        map[shield_row][shield_col] = '.'; // Removes the shield of the map
    }

    for (int i = 0; i < NUM_TRAPS; i++) {
        if (s->player_row == trap_coords[i][0] && s->player_col == trap_coords[i][1]) { // Usando a notação [i][0], estamos acessando o elemento da matriz trap_coords na posição i,
                                                                                        // mas pegando o primeiro elemento desse par de coordenadas. 
                                                                                        // Da mesma forma, usando a notação [i][1], estamos acessando o segundo elemento desse par de coordenadas.
            if (has_shield) {
                // Perde escudo
                has_shield = false;
                player_shield = 0;
            } else {
                player_hp -= 10;
                if (player_hp <= 0) {
                    print_death_message();
                    endwin();
                    exit(0);
                }
            }
        }
    }

    refresh();
}

/*     
?                    David Costa
todo Calcula as coordenadas inicias do jogador
*/
void generate_player() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    int x, y;

    do {
        // gera coordenadas aleatórias
        x = rand() % (MAP_WIDTH - 2) + 1;
        y = rand() % (MAP_HEIGHT - 2) + 1;
    } while (map[y][x] == '#'); // repete enquanto a posição gerada for uma parede

    s->player_row = y;
    s->player_col = x;
}

/*     
?                    Tomás Machado
todo - Função que desenha a personagem no ecrã com a cor roxa
*/
void draw_player() {
    // draw the player character
    attron(COLOR_PAIR(5));
    mvaddch(s->player_row, s->player_col, '@' | A_BOLD);
    attroff(COLOR_PAIR(5));
}

/*     
?                    Pedro Argainha
todo Calcula as coordenadas inicias do jogador
*/
void desenha_mapa() {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    // Desenha o inventário
    mvprintw(LINES - 1, COLS - 30, "Inventário:");

    // Desenha o escudo no inventário se o jogador tiver um
    if (has_shield) {
        mvprintw(LINES - 1, COLS - 220, "Looks like you found a shield! Useful to protect yourself from the scary monsters out there! Press [E] to pick it up!");
        char key = getch();
            if (key == 'e') {
                mvprintw(LINES - 1, COLS - 18, "Escudo,");
                mvprintw(LINES - 1, COLS - 220, "Nice, now you are ready to fight! Press any key to continue");
                getch();
            }
    }
    if (s->player_row == sword_row && s->player_col == sword_col) {
        if (!has_sword) {
            has_sword = true;
            mvprintw(LINES - 1, COLS - 220, "Looks like you found an element to fighting (Sword)! Press [E] to pick it up!");
            char key = getch();
                if (key == 'e') {
                    mvprintw(LINES - 1, COLS - 17, "Espada");
                    mvprintw(LINES - 1, COLS - 220, "Good job! Now you can use the sword to fight against the monsters! Press any key to continue");
                    getch();
                }
        }
    }

    refresh();
}


/*      
?                        Tomás Machado
todo Criada a função de limitação da visão do jogador dado um certo raio:
todo - Está a desenhar as células visíveis dentro de um alcance especificado pelo parâmetro 'range'
*/
void limit_vision(STATE* s, int range) {
    extern char map[MAP_HEIGHT][MAP_WIDTH];
    int i, j;

    for (i = s->player_row - range; i <= s->player_row + range; i++) {
        for (j = s->player_col - range; j <= s->player_col + range; j++) {
            if (i >= 0 && i < MAP_HEIGHT && j >= 0 && j < MAP_WIDTH) {
                int dx = abs(s->player_row - i);
                int dy = abs(s->player_col - j);

                if (dx * dx + dy * dy <= range * range) {  //! Se a célula está dentro dos limites do mapa faz o cálculo da distância entre o jogador e a célula atual
                    if (map[i][j] == '#') {
                        //! Se a célula for uma parede, verifica se é uma borda da caverna
                        if ((i > 0 && map[i-1][j] == '.') ||
                            (i < MAP_HEIGHT-1 && map[i+1][j] == '.') ||
                            (j > 0 && map[i][j-1] == '.') ||
                            (j < MAP_WIDTH-1 && map[i][j+1] == '.')) { //! Parede é adjacente ao chão
                            if (dx <= range && dy <= range) {
                                mvaddch(i, j, '#');
                                attron(COLOR_PAIR(8));
                                mvaddch(i, j, '#'); //! Mostra a borda da caverna
                                attroff(COLOR_PAIR(8));
                            }
                        }
                    } else {    
                        //! Se for chão ele mostra
                        mvaddch(i, j, map[i][j]);
                    }
                }
            }
        }
    }
}

/*
?                       Tomás Machado
todo Função que usa a função anterior (limit_vision) para desenhar aquilo que a personagem vê
*/
void draw_light() {
    attron(COLOR_PAIR(2));
    limit_vision(s, 7);
    attroff(COLOR_PAIR(2));
    refresh();
}




/*     
?                   Grupo todo
todo Adicionamos as funcionalidades todas à main conforme íamos fazendo o trabalho
*/
int main() {
    initscr();
    
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    srand(time(NULL));

    s = (STATE*) malloc(sizeof(STATE));

    extern char map[MAP_HEIGHT][MAP_WIDTH];
    initialize_map();
    generate_map();
    generate_player();
    generate_shield();
    generate_trap();
    generate_sword();
    generate_monster();


    init_pair (1, COLOR_BLACK, COLOR_BLACK);
    init_pair (2, COLOR_WHITE, COLOR_BLACK);
    init_pair (3, COLOR_RED, COLOR_BLACK);
    init_pair (4, COLOR_GREEN, COLOR_GREEN);
    init_pair (5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair (6, COLOR_BLUE, COLOR_BLUE);
    init_pair (7, COLOR_RED, COLOR_RED);
    init_pair (8, COLOR_YELLOW, COLOR_BLACK);

    attron(COLOR_PAIR(5));
    mvprintw(LINES / 2 - 3, (COLS - 18) / 2, "Welcome to RogueLite!");
    attroff(COLOR_PAIR(5));

    const char* options[NUM_OPTIONS] = {"Play", "Exit Game"};
    int choice = 0;
    int highlight = 0;

    while (1) {
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == highlight)
                attron(COLOR_PAIR(2));
            else
                attron(COLOR_PAIR(3));
            attron(A_BOLD);
            mvprintw(LINES / 2 + i, (COLS - strlen(options[i])) / 2, "%s", options[i]);
            attroff(A_BOLD);
            if (i == highlight)
                attroff(COLOR_PAIR(2));
            else
                attroff(COLOR_PAIR(3));
        }

        choice = getch();

        switch (choice) {
            case KEY_UP:
                highlight--;
                if (highlight < 0)
                    highlight = 0;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight >= NUM_OPTIONS)
                    highlight = NUM_OPTIONS - 1;
                break;
            case '\n':
                if (highlight == 0) {
                    // Play game
                    while (1) {
                        clear();
                        draw_light();
                        draw_player();
                        draw_health_bar();
                        draw_shield_bar();
                        desenha_mapa();
                        update(s);
                        refresh();
                    }
                } else if (highlight == 1) {
                    // Exit game
                    endwin();
                    exit(0);
                }
                break;
        }
    }
    
    endwin();
    return 0;
}

