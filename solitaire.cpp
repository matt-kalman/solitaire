#include <algorithm>
#include <ctime>
#include "cards.h"

struct Game {
    Pile tableau[7], foundation[4], stock[2];

    Game() {
        for (auto& i : tableau) {
            i.rect.y = ch+30;
        } for (auto& i : foundation) {
            i.rect.y = 15;
        } for (auto& i : stock) {
            i.rect.y = 15;
        }

        for (int rank = 0; rank < 13; rank++) {
            for (int suit = 0; suit < 4; suit++) {
                stock[0].add(new Card(rank,suit));
            }
        }

        random_shuffle(begin(stock[0].cards), end(stock[0].cards));

        int i = 0;
        for (auto& t : tableau) {
            for (int x = 0; x <= i; x++) {
                stock[0].give(stock[0].cards.front(), t);
            }
            t.cards.at(i)->flip();
            i++;
        }

        for (auto& card : stock[0].cards) {
            card->flip();
        }
    }
};

SDL_Window* window; SDL_Event event;
SDL_Rect reset = {0,0,104,31}, undo = {0,0,85,31}, solve = {0,0,103,31};
int w = 1000, h = 700, vw = 1000, offset[2];
bool done, solving;
Card deal = Card(0,0);
Pile* pile; Card* card;
Game game; vector<Game> undoarray;

void drawrect(SDL_Rect &rect) {
    SDL_SetRenderDrawColor(renderer,25,145,95,255);
    SDL_RenderFillRect(renderer, &rect);
}

void drawtext(SDL_Rect &rect, int i) {
    // reset: 54, undo: 55, solve: 56
    SDL_Surface* text = surface(i, rect.w, rect.h, 3);

    texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_FreeSurface(text);
    text = NULL;

    SDL_RenderCopy(renderer, texture, 0, &rect);
    SDL_DestroyTexture(texture);
    texture = NULL;
}

int m(int i) {
    SDL_Point point;
    SDL_GetMouseState(&point.x, &point.y);
    if (!i) {
        return point.x;
    } else {
        return point.y;
    }
}
bool pos(SDL_Rect v) {
    SDL_Point point = {m(0),m(1)};
    return SDL_PointInRect(&point,&v);
}

void setdim() {
    game.stock[0].rect.x = (w-vw)/2+15;
    deal.rect.x = (w-vw)/2+15;
    deal.rect.y = 15;

    int i = 0;
    for (auto& t : game.tableau) {
        int fu = 0, n = 0;
        t.rect.x = (w-vw-cw)/2 + (2*i+1)*vw/14;

        for (auto& x : t.cards) {
            x->rect.x = (w-vw-cw)/2 + (2*i+1)*vw/14;
            x->rect.y = ch+30 + 10*n + 15*fu;
            if (x->faceUp) {
                fu++;
            }
            n++;
        }
        i++;
    }

    i = 1;
    for (auto& f : game.foundation) {
        f.rect.x = (w+vw)/2 - cw*i-15*i;

        for (auto& x : f.cards) {
            x->rect.x = (w+vw)/2 - cw*i-15*i;
            x->rect.y = 15;
        }
        i++;
    }

    for (auto& x : game.stock[1].cards) {
        x->rect.x = (w-vw)/2 + cw+30;
        x->rect.y = 15;
    }

    reset.x = (w-vw)/2+8;
    reset.y = h-reset.h-8;

    undo.x = (w+vw)/2-undo.w-8;
    undo.y = h-undo.h-8;

    solve.x = (w-vw)/2+cw+30;
    solve.y = (ch-solve.h)/2+15;

    done = false;

    if (game.stock[0].cards.empty() && game.stock[1].cards.empty()) {
        done = true;
        for (auto& t : game.tableau) {
            if (!done) {
                break;
            } for (auto& i : t.cards) {
                if (!i->faceUp) {
                    done = false;
                    break;
                }
            }
        }
    }

    if (done) {
        for (auto& f : game.foundation) {
            if (f.cards.size() == 13) {
                done = false;
            } else {
                done = true;
                break;
            }
        }
    }
}

void each(Pile i, int r=1) {
    if (r) {
        drawrect(i.rect);
    }
    for (auto& x : i.cards) {
        if (x != card) {
            x->show();
        }
    }
}

int index() {
    auto it = find(begin(pile->cards), end(pile->cards), card);
    return distance(begin(pile->cards), it);
}

void show() {
    SDL_SetRenderDrawColor(renderer,15,112,71,255);
    SDL_RenderClear(renderer);

    if (!game.stock[0].cards.empty()) {
        deal.show();
    } else {
        drawrect(game.stock[0].rect);
    }

    for (auto& t : game.tableau) {
        each(t);
    } for (auto& f : game.foundation) {
        each(f);
    } each(game.stock[1],0);
    
    if (card) {
        auto it = find(begin(pile->cards), end(pile->cards), card);
        if (it != end(pile->cards)) {
            for (int i = index(); i < pile->cards.size(); i++) {
                pile->cards.at(i)->show();
            }
        }
    }

    drawtext(reset, 54);
    drawtext(undo, 55);

    if (done) {
        drawtext(solve, 56);
    }

    SDL_RenderPresent(renderer);
}

void get(Pile* i) {
    for (auto& x : i->cards) {
        if (pos(x->rect)) {
            pile = i, card = x;
            offset[0] = m(0)-card->rect.x;
            offset[1] = m(1)-card->rect.y;
        }
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));
    game = Game();

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Solitaire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_RESIZABLE|SDL_WINDOW_MAXIMIZED);
    SDL_SetWindowIcon(window, surface(53, 64, 64, 4));
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    setdim();
    while (true) {
        pile = NULL, card = NULL;
        show();
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        } if (event.type == SDL_WINDOWEVENT) {
            SDL_GetWindowSize(window, &w, &h);
            setdim();
        } if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            for (auto& t : game.tableau) {
                get(&t);
            } for (auto& f : game.foundation) {
                get(&f);
            } get(&game.stock[1]);

            while ((card && card->faceUp) || pos(deal.rect) ||
            pos(reset) || pos(undo) || pos(solve)) {
                if (card) {
                    for (int i = index(); i < pile->cards.size(); i++) {
                        pile->cards.at(i)->rect.x = m(0)-offset[0];
                        pile->cards.at(i)->rect.y = m(1)-offset[1]+25*(i-index());
                    }
                }
                show();

                SDL_WaitEvent(&event);
                if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                    for (auto& t : game.tableau) {
                        Card* back = NULL;
                        if (!t.cards.empty()) {
                            back = t.cards.back();
                        }

                        if ((!t.cards.empty() && pos(back->rect) && back->rank == card->rank+1 && back->color != card->color) ||
                        (t.cards.empty() && pos(t.rect) && card->rank == 12)) {
                            undoarray.push_back(game);
                            int length = pile->cards.size(), ind = index();
                            for (int i = ind; i < length; i++) {
                                pile->give(pile->cards.at(ind),t);
                            }
                        }
                    }

                    for (auto& f : game.foundation) {
                        if (pos(f.rect)) {
                            Card* back = NULL;
                            if (!f.cards.empty()) {
                                back = f.cards.back();
                            }

                            if ((!f.cards.empty() && back->rank == card->rank-1 && back->suit == card->suit) ||
                                (f.cards.empty() && card->rank == 0)) {
                                undoarray.push_back(game);
                                pile->give(card,f);
                            }
                        }
                    }

                    if (pos(deal.rect)) {
                        undoarray.push_back(game);
                        if (!game.stock[0].cards.empty()) {
                            game.stock[0].give(game.stock[0].cards.front(), game.stock[1]);
                        } else {
                            int length = game.stock[1].cards.size();
                            for (int i = 0; i < length; i++) {
                                game.stock[1].give(game.stock[1].cards.front(), game.stock[0]);
                            }
                        }
                    }

                    if (pos(reset)) {
                        undoarray.push_back(game);
                        game = Game();
                    }

                    if (pos(undo) && !undoarray.empty()) {
                        game = undoarray.back();
                        undoarray.pop_back();
                    }

                    if (pos(solve) && done) {
                        undoarray.push_back(game);
                        do {
                            solving = false;
                            for (auto& f : game.foundation) {
                                for (auto& t : game.tableau) {
                                    if (!t.cards.empty()) {
                                        Card* back = t.cards.back();
                                        if ((!f.cards.empty() && back->rank == f.cards.back()->rank+1 && back->suit == f.cards.back()->suit) ||
                                        (f.cards.empty() && back->rank == 0)) {
                                            t.give(back, f);
                                        } 
                                        solving = true;
                                    }
                                }
                            }
                        } while (solving);
                    }

                    if (pile && !pile->cards.empty() && !pile->cards.back()->faceUp) {
                        pile->add(new Card(*pile->cards.back()));
                        pile->cards.erase(end(pile->cards)-2);
                        pile->cards.back()->flip();
                    }

                    setdim();
                    break;
                }
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}