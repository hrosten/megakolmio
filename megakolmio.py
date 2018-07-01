#!/usr/bin/env python

################################################################################

import sys
from copy import copy

################################################################################

# NEIGHBORS:
# Puzzle is filled starting from position 0 to 8 in order. Placing first cards
# somewhere in the middle of the puzzle (as opposed to placing 0 on the top of
# the board) will greatly improve the performance since it yields less solutions
# that end up non-completing late in the game.
# Below IDs are used as indexes to GameState.cardsOnBoard array.
#
#                 / \
#                /   \
#               /     \
#              /   6   \
#             /         \
#             -----------
#           / \         / \
#          /   \   0   /   \
#         /     \     /     \
#        /   2   \   /   1   \
#       /         \ /         \
#       ----------- -----------
#     / \         / \         / \
#    /   \   3   /   \   5   /   \
#   /     \     /     \     /     \
#  /   7   \   /   4   \   /   8   \
# /         \ /         \ /         \
# ----------- ----------- -----------

# EDGES:
# Below IDs are used to identify edges:
#
#   Up:             Down:
#       / \         -----------
#      /   \        \    2    /
#     /0   1\        \       /
#    /       \        \1   0/
#   /    2    \        \   /
#   -----------         \ /
#

# Neighbor relations and common edges:
NEIGHBORMAP = {}
NEIGHBORMAP[0,1] = 0
NEIGHBORMAP[0,2] = 1
NEIGHBORMAP[0,6] = 2
NEIGHBORMAP[1,5] = 2
NEIGHBORMAP[2,3] = 2
NEIGHBORMAP[3,4] = 0
NEIGHBORMAP[3,7] = 1
NEIGHBORMAP[4,5] = 1
NEIGHBORMAP[5,8] = 0    # cards on positions 5 and 8 have a common edge 0

# In the solution printout, the cards need to be printed from top to down and
# left to right:
PRINTORDER = [6,2,0,1,7,3,4,5,8]

################################################################################

class Deck:
    # Deck of cards: each card has a name and three edges
    def __init__(self):
        # F = Fox, D = Deer, R = Raccoon
        # H = Head, B = Body
        self.cards = []
        self.cards.append(Card("P1",["FH","FB","DH"]))
        self.cards.append(Card("P2",["DH","FB","RB"]))
        self.cards.append(Card("P3",["DH","FB","FH"]))
        self.cards.append(Card("P4",["DH","DB","FB"]))
        self.cards.append(Card("P5",["DH","RB","DB"]))
        self.cards.append(Card("P6",["RB","FB","RH"]))
        self.cards.append(Card("P7",["FB","RH","FH"]))
        self.cards.append(Card("P8",["RH","DH","RB"]))
        self.cards.append(Card("P9",["FB","DB","DH"]))
        self.nCardsInDeck = len(self.cards)

################################################################################

class Card:
    def __init__(self,name,edgenames):
        self.name = name
        self.edgeNames = edgenames

################################################################################

class PlayedCard:
    # PlayedCard is a specialized version of card - it includes two additional
    # properties: rotation and position. Position indicates the card's current
    # position on the board.
    # TODO: position in constructor
    # TODO: rotation with modulus
    def __init__(self,card):
        self.rotation = 0   # 0, 120, 240 degrees
        self.position = None
        self.card = card

    def matchesNeighbor(self,other):
        # Card matches another if body matches head, for instance:
        # FH (FoxHead) matches FB (FoxBody)
        if self.position == None:
            return False
        common_edge = NEIGHBORMAP[self.position,other.position]
        e1 = self.card.edgeNames[common_edge]
        e2 = other.card.edgeNames[common_edge]
        if e1[0] == e2[0] and e1[1] != e2[1]:
            return True
        else:
            return False

    def rotate(self):
        # Return False if the card would end up in it's original rotation
        if self.rotation >= 240:
            return False
        else:
            self.rotation += 120
            # Rotate edgeNames array: last becomes first
            self.card.edgeNames=[self.card.edgeNames[-1]]+self.card.edgeNames[0:-1]
            return True

################################################################################

class GameState():
    def __init__(self):
        # Next free position in cardsOnBoard array
        self.nextOnBoard = 0
        # Next card on the deck that will be next placed on the board
        self.topOfTheDeck = 0
        # Cards currently placed on the board
        self.cardsOnBoard = [None]*len(DECK.cards)
        # Card names currently on the board, we track this separately to
        # make it quicker to select the next card from the deck
        self.cardNamesOnBoard = [None]*len(DECK.cards)

    def isSolved(self, partial=False):
        # Return true if current solution is valid. If partial is True,
        # then consider also partial solutions valid.
        for c1_index,c2_index in NEIGHBORMAP:
            c1 = self.cardsOnBoard[c1_index]
            c2 = self.cardsOnBoard[c2_index]
            if None in [c1,c2]:
                if partial: continue
                return False
            if not c1.matchesNeighbor(c2):
                return False
        return True

    def output(self):
        # Output the solution in the specified PRINTORDER
        msg = []
        for i in PRINTORDER:
            msg.append(self.cardsOnBoard[i].card.name)
        print("[%s]" % ",".join(msg))

    def replicate(self):
        # Make a copy of the object
        new = GameState()
        new.nextOnBoard = self.nextOnBoard
        new.topOfTheDeck = self.topOfTheDeck
        new.cardsOnBoard = copy(self.cardsOnBoard)
        new.cardNamesOnBoard = copy(self.cardNamesOnBoard)
        return new

    def nextFromDeck(self):
        # Return the next card from the deck
        if self.topOfTheDeck >= DECK.nCardsInDeck:
            return None
        for i in range(self.topOfTheDeck, DECK.nCardsInDeck):
            fromdeck = DECK.cards[i]
            if fromdeck.name not in self.cardNamesOnBoard:
                self.topOfTheDeck = i
                return fromdeck
        return None

    def addNewCard(self):
        # Add a new card from the deck to the board
        fromdeck = self.nextFromDeck()
        if fromdeck == None:
            return False
        newcard = PlayedCard(fromdeck)
        newcard.position = self.nextOnBoard
        self.cardsOnBoard[newcard.position] = newcard
        self.cardNamesOnBoard[newcard.position] = newcard.card.name
        self.nextOnBoard += 1
        return True

    def replaceCard(self, card):
        # Replace the given card with a new card from the deck
        fromdeck = self.nextFromDeck()
        if fromdeck == None:
            return False
        card.card = fromdeck
        card.rotation = 0
        self.cardNamesOnBoard[card.position] = card.card.name
        return True

    def getLastAdded(self):
        # Return the card that was last added on the board
        last_added_index = self.nextOnBoard - 1
        return self.cardsOnBoard[last_added_index]

    def first(self):
        # Add next free card to the first empty place in the current
        # game state. Return the new game state if card was added on
        # the board. Otherwise return None.
        newstate = self.replicate()
        newstate.topOfTheDeck = 0
        if not newstate.addNewCard():
            return None
        return newstate

    def next(self):
        # If possible, try rotating the card added last on the board
        # in hope of finding a match. Otherwise select a new card.
        # Return new gamestate if card was added on the board. Otherwise
        # return None.
        newstate = self.replicate()
        card = newstate.getLastAdded()
        if card.rotate():
            return newstate
        elif not newstate.replaceCard(card):
            return None
        return newstate

################################################################################

# https://en.wikipedia.org/wiki/Backtracking
def solve(game):

    # The algorithm checks whether the game can be completed to a valid
    # solution. If it cannot, the whole sub-tree rooted at the given game state
    # is skipped (pruned).
    if not game.isSolved(partial=True):
        return

    # Otherwise, the algorithm first checks whether the current state is a valid
    # solution, and if so reports it to the user; ...
    if game.isSolved():
        game.output()


    # ... and recursively enumerates all sub-trees of current state
    newstate = game.first()
    while newstate != None:
        solve(newstate)
        newstate = newstate.next()

################################################################################

if __name__ == "__main__":
    DECK = Deck()
    game = GameState()
    solve(game)

################################################################################
