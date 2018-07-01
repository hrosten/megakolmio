///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////

using namespace std;

///////////////////////////////////////////////////////////////////////////////

// NEIGHBORS:
// Puzzle is filled starting from position 0 to 8 in order. Placing first cards
// somewhere in the middle of the puzzle (as opposed to placing 0 on the top of
// the board) will greatly improve the performance since it yields less 
// solutions that end up non-completing late in the game.
// Below IDs are used as indexes to GameState::mCardsOnBoard array.
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
typedef unsigned char sint;
static unordered_map<string,sint> NEIGHBORMAP{
    {"01", 0},
    {"02", 1},
    {"06", 2},
    {"15", 2},
    {"23", 2},
    {"34", 0},
    {"37", 1},
    {"45", 1},
    {"58", 0}    // cards on positions 5 and 8 have a common edge 0
};

static const sint PRINTORDER[] = {6,2,0,1,7,3,4,5,8};

///////////////////////////////////////////////////////////////////////////////

class Card {
    public:
    string mName;
    vector<string> mEdges;

    Card(string name, vector<string> edges) {
        mName = name;
        mEdges = edges;
    }
};

///////////////////////////////////////////////////////////////////////////////

struct Deck {
    static const Card cards[];
};

const Card Deck::cards[] = {
    Card("P1",{"FH","FB","DH"}),
    Card("P2",{"DH","FB","RB"}),
    Card("P3",{"DH","FB","FH"}),
    Card("P4",{"DH","DB","FB"}),
    Card("P5",{"DH","RB","DB"}),
    Card("P6",{"RB","FB","RH"}),
    Card("P7",{"FB","RH","FH"}),
    Card("P8",{"RH","DH","RB"}),
    Card("P9",{"FB","DB","DH"})
};

static const int CARDS_IN_DECK = sizeof(Deck::cards)/sizeof(Deck::cards[0]); 
static const int EDGES_IN_CARD = Deck::cards[0].mEdges.size();

///////////////////////////////////////////////////////////////////////////////

class PlayedCard {
    public:
    sint mRotation;
    sint mPosition;
    const Card *mCard;

    PlayedCard(const Card *card = NULL, sint position = 0, sint rotation = 0) {
        mCard = card;
        mPosition = position;
        mRotation = rotation;
    }

    bool matchesNeighbor(PlayedCard *other) {
        if (mCard == NULL) { return false; }
        sint common_edge = NEIGHBORMAP[to_string(mPosition)+to_string(other->mPosition)];
        sint own_rotated_common_edge = (common_edge + mRotation) % EDGES_IN_CARD;
        sint other_rotated_common_edge = (common_edge + other->mRotation) % EDGES_IN_CARD;
        string e1 = this->mCard->mEdges[own_rotated_common_edge];
        string e2 = other->mCard->mEdges[other_rotated_common_edge];
        if(e1[0] == e2[0] && e1[1] != e2[1] ) {
            return true;
        }
        else {
            return false;
        }
    }

    bool rotate() {
        if (mRotation >= 2) {
            return false;
        }
        else {
            mRotation += 1;
            return true;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

class GameState {
    public:
    sint mNextOnBoard;
    sint mTopOfTheDeck;
    PlayedCard* mCardsOnBoard[CARDS_IN_DECK];
    
    GameState() {
        memset(mCardsOnBoard, 0, sizeof(mCardsOnBoard));
        mNextOnBoard = mTopOfTheDeck = 0;
    }

    ~GameState() {
        for(sint i=0; i<CARDS_IN_DECK; i++) {
            if(mCardsOnBoard[i]) {
                delete mCardsOnBoard[i];
            }
        }
    }

    GameState* replicate() {
        GameState *newstate = new GameState();
        newstate->mNextOnBoard = this->mNextOnBoard;
        newstate->mTopOfTheDeck = this->mTopOfTheDeck;
        for(sint i=0; i<CARDS_IN_DECK; i++) {
            if(this->mCardsOnBoard[i] == NULL) break;
            newstate->mCardsOnBoard[i] = 
            new PlayedCard(
                this->mCardsOnBoard[i]->mCard,
                this->mCardsOnBoard[i]->mPosition,
                this->mCardsOnBoard[i]->mRotation);
        }
        return newstate;
    }

    bool isSolved(bool partial=false) {
        PlayedCard *c1, *c2;
        for(const auto& i : NEIGHBORMAP)
        {
            string key = i.first;
            sint n1 = key[0] - '0'; // '1' to 1 etc.
            sint n2 = key[1] - '0';
            c1 = mCardsOnBoard[n1];
            c2 = mCardsOnBoard[n2];
            if (c1 == NULL || c2 == NULL) {
                if (partial) {
                    continue;
                }
                else {
                    return false;
                }
            }
            if (not c1->matchesNeighbor(c2)) {
                return false;
            }
        }
        return true;
    }

    void output() {
        cout << "[";
        for(sint i=0; i<CARDS_IN_DECK; i++) {
            cout << mCardsOnBoard[PRINTORDER[i]]->mCard->mName;
            if (i < CARDS_IN_DECK-1) cout << ",";
        }
        cout << "]" << endl;
    }

    bool isCardOnBoard(const Card* value) {
        for (sint i=0; i<CARDS_IN_DECK; ++i) {
            if (mCardsOnBoard[i] != NULL && 
                mCardsOnBoard[i]->mCard != NULL &&
                mCardsOnBoard[i]->mCard == value) {
                return true;
            }
        }
        return false;
    }

    const Card* nextFromDeck() {
        if(mTopOfTheDeck >= CARDS_IN_DECK) {
            return NULL;
        }
        const Card* fromdeck = NULL;
        for(sint i=mTopOfTheDeck; i<CARDS_IN_DECK; i++) {
            fromdeck = &Deck::cards[i];
            if(not isCardOnBoard(fromdeck)) {
                mTopOfTheDeck = i;
                return fromdeck;
            }

        }
        return NULL;
    }

    bool addNewCard() {
        const Card *fromdeck = this->nextFromDeck();
        if(fromdeck == NULL) {
            return false;
        }
        PlayedCard *newcard = new PlayedCard(fromdeck, mNextOnBoard);
        mCardsOnBoard[newcard->mPosition] = newcard;
        mNextOnBoard++;
        return true;
    }

    bool replaceCard(PlayedCard *card) {
        const Card *fromdeck = this->nextFromDeck();
        if(fromdeck == NULL) {
            return false;
        }
        card->mCard = fromdeck;
        card->mRotation = 0;
        return true;
    }

    PlayedCard* getLastAdded() {
        sint last = mNextOnBoard - 1;
        return mCardsOnBoard[last];
    }

    GameState* first() {
        GameState *newstate = this->replicate();
        newstate->mTopOfTheDeck = 0;
        if(!newstate->addNewCard()) {
            delete newstate;
            return NULL;
        }
        return newstate;
    }

    GameState* next() {
        GameState *newstate = this->replicate();
        PlayedCard *card = newstate->getLastAdded();
        if (card->rotate()) {
            return newstate;
        }
        else if (!newstate->replaceCard(card)) {
            delete newstate;
            return NULL;
        }
        return newstate;
    }
};

///////////////////////////////////////////////////////////////////////////////

void solve(GameState* game) {
    if(!game->isSolved(true)) {
        return;
    }

    if(game->isSolved()) {
        game->output();
    }

    GameState *tempstate = NULL;
    GameState *newstate = game->first();
    while(newstate != NULL) {
        solve(newstate);
        tempstate = newstate;
        newstate = newstate->next();
        delete tempstate;
    }
}

///////////////////////////////////////////////////////////////////////////////

int main() {
    GameState* game = new GameState();
    solve(game);
    delete game;
}

///////////////////////////////////////////////////////////////////////////////
