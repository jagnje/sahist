#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define EMPTY 0
#define WPAWN 1
#define WKNIGHT 2
#define WBISHOP 3
#define WROOK 4
#define WQUEEN 5
#define WKING 6
#define BPAWN -1
#define BKNIGHT -2
#define BBISHOP -3
#define BROOK -4
#define BQUEEN -5
#define BKING -6

typedef struct {
    int from;
    int to;
} Move;

typedef struct {
    Move moves[256];
    int count;
} MoveList;

int board[64];

// utility
int same_color(int a, int b) {
    if (a == EMPTY || b == EMPTY) return 0;
    return (a > 0 && b > 0) || (a < 0 && b < 0);
}

int is_white(int p) { return p > 0; }
int is_black(int p) { return p < 0; }

int in_bounds(int sq) {
    return sq >= 0 && sq < 64;
}
int file_of(int sq) { return sq % 8; }
int rank_of(int sq) { return sq / 8; }

void print_board() {
    printf("\n    a   b   c   d   e   f   g   h\n");
    printf("  +---+---+---+---+---+---+---+---+\n");
    for (int r = 7; r >= 0; r--) {
        printf("%d |", r+1);
        for (int f = 0; f < 8; f++) {
            int p = board[r*8+f];
            char c='.';
            switch(p){
                case WPAWN: c='P'; break;
                case WKNIGHT: c='N'; break;
                case WBISHOP: c='B'; break;
                case WROOK: c='R'; break;
                case WQUEEN: c='Q'; break;
                case WKING: c='K'; break;
                case BPAWN: c='p'; break;
                case BKNIGHT: c='n'; break;
                case BBISHOP: c='b'; break;
                case BROOK: c='r'; break;
                case BQUEEN: c='q'; break;
                case BKING: c='k'; break;
            }
            printf(" %c |", c);
        }
        printf(" %d\n", r+1);
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h\n\n");
}

// add move helper
void add_move(MoveList *list, int from, int to) {
    list->moves[list->count].from = from;
    list->moves[list->count].to = to;
    list->count++;
}

// generate sliding moves (bishop, rook, queen)
void gen_slides(MoveList *list, int sq, int piece, const int *dirs, int dirCount) {
    for (int d = 0; d < dirCount; d++) {
        int step = dirs[d];
        int to = sq;
        while (1) {
            int fileBefore = file_of(to);
            int rankBefore = rank_of(to);

            to += step;
            if (!in_bounds(to)) break;

            // disallow wrap-around across files
            int df = file_of(to) - fileBefore;
            int dr = rank_of(to) - rankBefore;
            if (abs(df) > 1 || abs(dr) > 1) break;

            if (board[to] == EMPTY) {
                add_move(list, sq, to);
            } else {
                if (!same_color(piece, board[to])) {
                    add_move(list, sq, to);
                }
                break;
            }
        }
    }
}

// knight moves
void gen_knight(MoveList *list, int sq, int piece) {
    int offs[8] = { 17,15,10,6,-17,-15,-10,-6 };
    for (int i=0;i<8;i++){
        int to = sq + offs[i];
        if (!in_bounds(to)) continue;

        // must also make sure we didn't wrap files too far
        int df = abs(file_of(to)-file_of(sq));
        int dr = abs(rank_of(to)-rank_of(sq));
        if (!((df==1 && dr==2)||(df==2 && dr==1))) continue;

        if (board[to] == EMPTY || !same_color(piece, board[to])) {
            add_move(list, sq, to);
        }
    }
}

// king moves
void gen_king(MoveList *list, int sq, int piece) {
    for (int rf=-1; rf<=1; rf++) {
        for (int ff=-1; ff<=1; ff++) {
            if (rf==0 && ff==0) continue;
            int f = file_of(sq)+ff;
            int r = rank_of(sq)+rf;
            if (f<0||f>7||r<0||r>7) continue;
            int to = r*8+f;
            if (board[to]==EMPTY || !same_color(piece, board[to])) {
                add_move(list,sq,to);
            }
        }
    }
}

// pawn moves (no en passant for simplicity)
void gen_pawn(MoveList *list, int sq, int piece) {
    int dir = is_white(piece) ? 1 : -1; // white moves up ranks (toward 8), black down
    int r = rank_of(sq);
    int f = file_of(sq);

    int forward = (r+dir)*8 + f;
    if (r+dir>=0 && r+dir<=7) {
        // forward move
        if (board[forward] == EMPTY) {
            add_move(list, sq, forward);

            // double push from base rank
            if ((is_white(piece) && r==1) || (is_black(piece) && r==6)) {
                int forward2 = (r+2*dir)*8 + f;
                if (board[forward2]==EMPTY) {
                    add_move(list,sq,forward2);
                }
            }
        }
        // captures
        if (f>0) {
            int capL = (r+dir)*8 + (f-1);
            if (in_bounds(capL) && board[capL]!=EMPTY && !same_color(piece, board[capL])) {
                add_move(list, sq, capL);
            }
        }
        if (f<7) {
            int capR = (r+dir)*8 + (f+1);
            if (in_bounds(capR) && board[capR]!=EMPTY && !same_color(piece, board[capR])) {
                add_move(list, sq, capR);
            }
        }
    }
}

// main move generator for a side
void generate_moves(int whiteToMove, MoveList *list) {
    list->count = 0;

    for (int sq=0; sq<64; sq++) {
        int p = board[sq];
        if (p == EMPTY) continue;
        if (whiteToMove && !is_white(p)) continue;
        if (!whiteToMove && !is_black(p)) continue;

        int absP = p>0?p:-p;

        switch(absP) {
            case WPAWN: gen_pawn(list,sq,p); break;
            case WKNIGHT: gen_knight(list,sq,p); break;
            case WKING: gen_king(list,sq,p); break;
            case WBISHOP: {
                const int dirs[4]={9,7,-9,-7};
                gen_slides(list,sq,p,dirs,4);
            } break;
            case WROOK: {
                const int dirs[4]={8,-8,1,-1};
                gen_slides(list,sq,p,dirs,4);
            } break;
            case WQUEEN: {
                const int dirs[8]={9,7,-9,-7,8,-8,1,-1};
                gen_slides(list,sq,p,dirs,8);
            } break;
        }
    }
}

// apply move
void make_move(Move m) {
    board[m.to] = board[m.from];
    board[m.from] = EMPTY;
}

// parse simple coordinate move like "e2e4"
int parse_move(const char *s, MoveList *list, Move *out) {
    if (strlen(s) < 4) return 0;
    int f1 = s[0]-'a';
    int r1 = s[1]-'1';
    int f2 = s[2]-'a';
    int r2 = s[3]-'1';
    if (f1<0||f1>7||f2<0||f2>7||r1<0||r1>7||r2<0||r2>7) return 0;
    int from = r1*8+f1;
    int to   = r2*8+f2;

    for (int i=0;i<list->count;i++){
        if (list->moves[i].from==from && list->moves[i].to==to){
            *out = list->moves[i];
            return 1;
        }
    }
    return 0;
}

// pick a random legal move for AI
int pick_ai_move(MoveList *list, Move *out) {
    if (list->count == 0) return 0;
    int idx = rand() % list->count;
    *out = list->moves[idx];
    return 1;
}

void setup_startpos() {
    int start[64] = {
        BROOK,  BKNIGHT, BBISHOP, BQUEEN, BKING,  BBISHOP, BKNIGHT, BROOK,
        BPAWN,  BPAWN,   BPAWN,   BPAWN,  BPAWN,  BPAWN,   BPAWN,   BPAWN,
        EMPTY,  EMPTY,   EMPTY,   EMPTY,  EMPTY,  EMPTY,   EMPTY,   EMPTY,
        EMPTY,  EMPTY,   EMPTY,   EMPTY,  EMPTY,  EMPTY,   EMPTY,   EMPTY,
        EMPTY,  EMPTY,   EMPTY,   EMPTY,  EMPTY,  EMPTY,   EMPTY,   EMPTY,
        EMPTY,  EMPTY,   EMPTY,   EMPTY,  EMPTY,  EMPTY,   EMPTY,   EMPTY,
        WPAWN,  WPAWN,   WPAWN,   WPAWN,  WPAWN,  WPAWN,   WPAWN,   WPAWN,
        WROOK,  WKNIGHT, WBISHOP, WQUEEN, WKING,  WBISHOP, WKNIGHT, WROOK
    };
    for (int i=0;i<64;i++) board[i]=start[i];
}

int main() {
    srand((unsigned)time(NULL));
    setup_startpos();

    int whiteToMove = 1;
    char input[32];

    printf("Simple C Chess. You are White. Enter moves like e2e4.\n");
    printf("No castling / en passant / promotion logic yet.\n");

    while (1) {
        print_board();

        MoveList list;
        generate_moves(whiteToMove, &list);

        // check game end
        if (list.count == 0) {
            if (whiteToMove)
                printf("No legal moves for White. Game over.\n");
            else
                printf("No legal moves for Black. Game over.\n");
            break;
        }

        if (whiteToMove) {
            printf("Your move (e.g. e2e4): ");
            if (!fgets(input, sizeof(input), stdin)) break;

            // trim newline
            for (int i=0; input[i]; i++) {
                if (input[i]=='\n' || input[i]=='\r') { input[i]=0; break; }
            }

            Move m;
            if (!parse_move(input, &list, &m)) {
                printf("Illegal move.\n");
                continue;
            }
            make_move(m);
        } else {
            Move aiMove;
            pick_ai_move(&list, &aiMove);
            printf("AI plays: %c%d%c%d\n",
                   'a'+file_of(aiMove.from), 1+rank_of(aiMove.from),
                   'a'+file_of(aiMove.to),   1+rank_of(aiMove.to));
            make_move(aiMove);
        }

        whiteToMove = !whiteToMove;
    }

    return 0;
}
