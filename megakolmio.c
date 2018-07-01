///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

// NEIGHBORS:
// Puzzle is filled starting from position 0 to 8 in order. Placing first cards
// somewhere in the middle of the puzzle (as opposed to placing 0 on the top of
// the board) will greatly improve the performance since it yields less 
// solutions that end up non-completing late in the game.
// Below IDs are used as indexes to cardsOnBoard array.
//
//                 / \
//                /   \
//               /     \
//              /   6   \
//             /         \
//             -----------
//           / \         / \
//          /   \   0   /   \
//         /     \     /     \
//        /   2   \   /   1   \
//       /         \ /         \
//       ----------- -----------
//     / \         / \         / \
//    /   \   3   /   \   5   /   \
//   /     \     /     \     /     \
//  /   7   \   /   4   \   /   8   \
// /         \ /         \ /         \
// ----------- ----------- -----------

// EDGES:
// Below IDs are used to identify edges:
//
//   Up:             Down:
//       / \         -----------
//      /   \        \    2    /
//     /0   1\        \       /
//    /       \        \1   0/
//   /    2    \        \   /
//   -----------         \ /
//

// Neighbor relations and common edges:
#define NMAP_SIZE 9
#define NMAP_FIRST 0
#define NMAP_SECOND 1
#define NMAP_COMMON_EDGE 2
typedef unsigned char sint;
static const sint NEIGHBORMAP[NMAP_SIZE][3] = 
{
    { 0,  1,  0 },
    { 0,  2,  1 },
    { 0,  6,  2 },
    { 1,  5,  2 },
    { 2,  3,  2 },
    { 3,  4,  0 },
    { 3,  7,  1 },
    { 4,  5,  1 },
    { 5,  8,  0 } 
};

static const sint PRINTORDER[] = {6,2,0,1,7,3,4,5,8};

///////////////////////////////////////////////////////////////////////////////

#define CARD_NAME_LEN 2
#define EDGES_IN_CARD 3

typedef struct Card {
    const char name[CARD_NAME_LEN];
    const char* edges[EDGES_IN_CARD];
} Card;

///////////////////////////////////////////////////////////////////////////////

static const Card Deck[] = {
    "P1",{"FH","FB","DH"},
    "P2",{"DH","FB","RB"},
    "P3",{"DH","FB","FH"},
    "P4",{"DH","DB","FB"},
    "P5",{"DH","RB","DB"},
    "P6",{"RB","FB","RH"},
    "P7",{"FB","RH","FH"},
    "P8",{"RH","DH","RB"},
    "P9",{"FB","DB","DH"}
};

#define CARDS_IN_DECK (sizeof(Deck)/sizeof(Deck[0]))

///////////////////////////////////////////////////////////////////////////////

typedef struct PlayedCard {
    sint rotation;
    sint position;
    const Card* card;
} PlayedCard;

sint getCommonEdge(PlayedCard* this, PlayedCard* other) {
    for(sint i=0; i<NMAP_SIZE; i++) {
        if( NEIGHBORMAP[i][NMAP_FIRST]  == this->position &&
            NEIGHBORMAP[i][NMAP_SECOND] == other->position) {
            return NEIGHBORMAP[i][NMAP_COMMON_EDGE];
        }
    }
    printf("[+] ERROR: getCommonEdge(): not neighbors");
    exit(0);
}

bool matchesNeighbor(PlayedCard* this, PlayedCard* other) {
    if(this->card == NULL ) return false;
    sint common_edge = getCommonEdge(this, other);
    sint own_rotated_common_edge = (common_edge + this->rotation) % EDGES_IN_CARD;
    sint other_rotated_common_edge = (common_edge + other->rotation) % EDGES_IN_CARD;
    const char* e1 = this->card->edges[own_rotated_common_edge];
    const char* e2 = other->card->edges[other_rotated_common_edge];
    if(e1[0] == e2[0] && e1[1] != e2[1] ) {
        return true;
    }
    else {
        return false;
    }
}

bool rotate(PlayedCard* this) {
    if(this->rotation >= 2) {
        return false;
    }
    else {
        this->rotation += 1;
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////

typedef struct GameState {
    sint nextOnBoard;
    sint topOfTheDeck;
    PlayedCard* cardsOnBoard[CARDS_IN_DECK];
} GameState;

GameState* replicate(GameState* this) {
    GameState *newstate = calloc(1,sizeof(GameState));
    newstate->nextOnBoard = this->nextOnBoard;
    newstate->topOfTheDeck = this->topOfTheDeck;
    for(sint i=0; i < CARDS_IN_DECK; i++) {
        if(this->cardsOnBoard[i] == NULL) break;
        newstate->cardsOnBoard[i] = malloc(sizeof(PlayedCard));
        memcpy(newstate->cardsOnBoard[i],this->cardsOnBoard[i],sizeof(PlayedCard));
    }
    return newstate;
}

void delete(GameState* this) {
    for(sint i=0; i<CARDS_IN_DECK; i++) {
        if(this->cardsOnBoard[i]) {
            free(this->cardsOnBoard[i]);
        }
    }
    free(this);
}

bool isSolved(GameState* this, bool partial) {
    PlayedCard *c1, *c2;
    sint n1, n2;
    for(sint i=0; i<NMAP_SIZE; i++) {
        c1 = this->cardsOnBoard[NEIGHBORMAP[i][NMAP_FIRST]];
        c2 = this->cardsOnBoard[NEIGHBORMAP[i][NMAP_SECOND]];
        if (c1 == NULL || c2 == NULL) {
            if (partial) {
                continue;
            }
            else {
                return false;
            }
        }
        if (!matchesNeighbor(c1, c2)) {
            return false;
        }
    }
    return true;
}

void output(GameState* this) {
    printf("[");
    for(sint i=0; i<CARDS_IN_DECK; i++) {
        printf("%s", this->cardsOnBoard[PRINTORDER[i]]->card->name);
        if (i < CARDS_IN_DECK-1) printf(",");
    }
    printf("]\n");
}

bool isCardOnBoard(GameState* this, const Card* value) {
    for (sint i=0; i<CARDS_IN_DECK; ++i) {
        if (this->cardsOnBoard[i] != NULL && 
            this->cardsOnBoard[i]->card != NULL &&
            this->cardsOnBoard[i]->card == value) {
            return true;
        }
    }
    return false;
}

const Card* nextFromDeck(GameState* this) {
    if(this->topOfTheDeck >= CARDS_IN_DECK) {
        return NULL;
    }
    const Card* fromdeck = NULL;
    for(sint i=this->topOfTheDeck; i<CARDS_IN_DECK; i++) {
        fromdeck = &Deck[i];
        if(!isCardOnBoard(this, fromdeck)) {
            this->topOfTheDeck = i;
            return fromdeck;
        }

    }
    return NULL;
}

bool addNewCard(GameState* this) {
    const Card *fromdeck = nextFromDeck(this);
    if(fromdeck == NULL) {
        return false;
    }
    PlayedCard *newcard = malloc(sizeof(PlayedCard));
    newcard->position = this->nextOnBoard;
    newcard->rotation = 0;
    newcard->card = fromdeck;
    this->cardsOnBoard[newcard->position] = newcard;
    this->nextOnBoard++;
    return true;
}

bool replaceCard(GameState* this, PlayedCard *card) {
    const Card *fromdeck = nextFromDeck(this);
    if(fromdeck == NULL) {
        return false;
    }
    card->card = fromdeck;
    card->rotation = 0;
    return true;
}

PlayedCard* getLastAdded(GameState* this) {
    sint last = this->nextOnBoard - 1;
    return this->cardsOnBoard[last];
}

GameState* first(GameState* this) {
    GameState *newstate = replicate(this);
    newstate->topOfTheDeck = 0;
    if(!addNewCard(newstate)) {
        delete(newstate);
        return NULL;
    }
    return newstate;
}

GameState* next(GameState* this) {
    GameState *newstate = replicate(this);
    PlayedCard *card = getLastAdded(newstate);
    if (rotate(card)) {
        return newstate;
    }
    else if (!replaceCard(newstate, card)) {
        delete(newstate);
        return NULL;
    }
    return newstate;
}

///////////////////////////////////////////////////////////////////////////////

void solve(GameState* game) {
    if(!isSolved(game,true)) {
        return;
    }

    if(isSolved(game,false)) {
        output(game);
    }

    GameState *tempstate = NULL;
    GameState *newstate = first(game);
    while(newstate != NULL) {
        solve(newstate);
        tempstate = newstate;
        newstate = next(newstate);
        delete(tempstate);
    }
}

///////////////////////////////////////////////////////////////////////////////

int main() {
    GameState* game = calloc(1,sizeof(GameState));
    solve(game);
    delete(game);
}

///////////////////////////////////////////////////////////////////////////////
