/*
    James M
    14/07/2023
    Program to model a game of hex
*/

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;

unsigned int UINT_MAX = std::numeric_limits<unsigned int>::max();

// Convert an unsigned integer to a hexadecimal character
char ToHex(unsigned int i)
{
    switch (i % 16)
    {
    default: return i;
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
    case 10: return 'a';
    case 11: return 'b';
    case 12: return 'c';
    case 13: return 'd';
    case 14: return 'e';
    case 15: return 'f';
    }
}

// Convert a hexadecimal character to an unsigned integer
unsigned int FromHex(char c)
{
    switch (std::tolower(c))
    {
    default: return UINT_MAX;
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
    }
}

enum class Team : char { None = '.', Red = 'R', Blue = 'B' };

// Returns the opposite team given
Team operator!(const Team team)
{
    return team == Team::Blue ? Team::Red : Team::Blue;
}

ostream& operator<<(ostream& os, const Team& team)
{
    return team == Team::Blue ? cout << "Blue" : cout << "Red";
}

class Tile;
using Tiles = vector<shared_ptr<Tile>>;

class Tile
{
public:
    Team team{};
    Tiles neighbours{};
    unsigned int id{};

    Tile(Team _team, Tiles _neighbours, unsigned int _id) : team(_team), neighbours(_neighbours), id(_id) {}
};

// Algorithm to recursively search a graph
class DepthFirstSearch
{
private:
    map<int, bool> tileVisits{};

    bool Recurse(const Tiles& tiles, const shared_ptr<Tile> current, const Tiles& endpoints, Team team)
    {
        tileVisits[current->id] = true;

        for (auto& neighbour : current->neighbours)
        {
            if (neighbour->team != team)
                continue;

            if (tileVisits[neighbour->id])
                continue;

            for (auto& endpoint : endpoints)
                if (neighbour->id == endpoint->id)
                    return true;

            return Recurse(tiles, neighbour, endpoints, team);
        }

        return false;
    }

public:
    bool IsPath(const Tiles& tiles, const Tiles& origins, const Tiles& endpoints, Team team)
    {
        // reset visit status
        for (auto& tile : tiles)
            tileVisits[tile->id] = false;

        // check if any starting point is valid, if true then recurse to find a path
        for (auto& origin : origins)
            if (origin->team == team)
                if (Recurse(tiles, origin, endpoints, team))
                    return true;

        return false;
    }
};

class Board
{
public:
    unsigned int rows {};
    unsigned int columns{};
    unsigned int size{}; // number of tiles
    Tiles tiles{};
    Tiles blueStarts{};
    Tiles blueEnds{};
    Tiles redStarts{};
    Tiles redEnds{};

    Board(unsigned int _rows, unsigned int _columns) : rows(_rows), columns(_columns)
    {
        size = rows * columns;

        // // initialise tiles
        for (unsigned int tile = 0; tile < size; tile++)
            tiles.emplace_back(make_shared<Tile>(Team::None, Tiles{}, tile));

        // // connect neighbours
        for (unsigned int tile = 0; tile < size; tile++)
        {
            if ((tile + 1) % columns != 0)   // not the last 'column'
                tiles[tile]->neighbours.push_back(tiles[tile + 1]);   // right

            if (tile + columns < size)    // not the last 'row'
                tiles[tile]->neighbours.push_back(tiles[tile + columns]); // below

            if (tile % columns != 0 && tile + columns < size)    // not the first 'column' or the last 'row'
                tiles[tile]->neighbours.push_back(tiles[tile + columns - 1]);   // diagonal down left

            if (tile % columns != 0)    // not the first 'column'
                tiles[tile]->neighbours.push_back(tiles[tile - 1]);  // left

            if (tile >= columns)  // not the first 'row'
                tiles[tile]->neighbours.push_back(tiles[tile - columns]); // above

            if (tile >= columns && (tile + 1) % columns != 0)   // not the first 'row' or the last 'column'
                tiles[tile]->neighbours.push_back(tiles[tile - columns + 1]);   // diagonal up right
        }

        // setup start and end points
        // blue = left to right
        // red = top to bottom
        for (unsigned int i = 0; i < size; i += columns)
            blueStarts.push_back(tiles[i]); // first column
        for (unsigned int i = columns - 1; i < size; i += columns)
            blueEnds.push_back(tiles[i]);   // last column
        for (unsigned int i = 0; i < columns; i++)
            redStarts.push_back(tiles[i]);  // first row
        for (unsigned int i = size - columns; i < size; i++)
            redEnds.push_back(tiles[i]);    // last row
    }
};

class Game
{
public:
    Team player{};
    Team ai{};
    bool over{};
    Board board;
    DepthFirstSearch depthFirstSearch{};

    Game(Team _player, Board& _board) : player(_player), ai(!_player), board(_board) {}

    // Update game state for a given tile
    bool Update(unsigned int id, const Team& team)
    {
        if (over)
            return true;

        bool hints{ team == player ? true : false };

        if (id == UINT_MAX)
        {
            cout << team << " has forfeit!";
            cout << !team << " team has won!" << endl;
            over = true;
            return true;
        }
            

        if (id >= board.size)   // out of bounds
        {
            if (hints)
            {
                cout << "Invalid tile!" << endl;
                cout << "Row must be between [0-" << board.rows - 1 << "]" << endl;
                cout << "Column must be between [0-" << board.columns - 1 << "]" << endl;
            }
            return false;
        }

        shared_ptr<Tile> tile { board.tiles[id] };

        // tile is unassigned
        if (tile->team == Team::None)
        {
            tile->team = team;

            Tiles starts{ team == Team::Blue ? board.blueStarts : board.redStarts };
            Tiles ends{ team == Team::Blue ? board.blueEnds : board.redEnds };

            // find path from any start to any end
            if (depthFirstSearch.IsPath(board.tiles, starts, ends, team))
            {
                cout << team << " team has won!" << endl;
                over = true;
            }

            return true;
        }

        if (hints)
            cout << "Tile unavailable. It has already been assigned to a team." << endl;
        return false;
    }
};

class AsciiRenderer
{
public:
    // prints board to console.
    void Display(const Board& board)
    {
        // Format:
        //  . - . - .
        //   \ / \ / \
        //    . - . - .
        //     \ / \ / \
        //      . - . - .

        // print column id
        for (unsigned int i = 0; i < board.columns; i++)
            cout << ToHex(i) << "   ";
        cout << endl;

        // print board
        int maxRowIndex = board.rows - 1;
        int maxColumnIndex = board.columns - 1;
        for (unsigned int row = 0; row < board.rows; row++)
        {
            unsigned int spaceMultipler{ row << 1 };

            // print tiles
            string buffer(spaceMultipler, ' ');   // buffer to reduce cout calls
            for (unsigned int column = 0; column < board.columns; column++)
                buffer += string{static_cast<char>(board.tiles[row * board.columns + column]->team)} + (column == maxColumnIndex ? "" : " - ");
            cout << buffer << " " << ToHex(row) << endl;    // prints row id

            if (row == maxRowIndex)
                break;  // skip last row connections

            // print connections
            buffer = string(spaceMultipler + 1, ' ');
            for (int column = 0; column < maxColumnIndex; column++)
                buffer += "\\ / ";
            cout << buffer << "\\" << endl;
        }
    }
};

class Player
{
public:
    Team SelectTeam()
    {
        char c;
        cout << "Will you be red? Y to accept." << endl;
        cin >> c;
        Team team = c == 'Y' || c == 'y' ? Team::Red : Team::Blue;
        cout << "You are " << team << endl;
        cout << "Connect a path from " << (team == Team::Blue ? "right to left" : "top to bottom") << endl;
        return team;
    }

    unsigned int SelectTile(unsigned int rows)
    {
        unsigned char row, column;
        cout << "Tile? ";
        cin >> row;
        cin >> column;
        cout << "Tile: " << row << " " << column << endl;
        return FromHex(row) * rows + FromHex(column);
    }
};

class AI
{
    virtual unsigned int SelectTile() = 0;
};

class MonteCarloAI : AI
{
private:
    shared_ptr<Game> game;
    default_random_engine generator;
    Tiles starts, ends;

    // Calculate if the given move results in victory
    bool Predict(vector<unsigned int> legalMoves, unsigned int firstMove)
    {
        game->board.tiles[firstMove]->team = game->ai;
        Team current = game->player;

        // random shuffle to emulate random plays
        shuffle(begin(legalMoves), end(legalMoves), generator);

        for (auto& move : legalMoves)
        {
            if (move == firstMove)
                continue;

            game->board.tiles[move]->team = current;
            current = !current;
        }

        return game->depthFirstSearch.IsPath(game->board.tiles, starts, ends, game->ai);
    }

public:
    MonteCarloAI(shared_ptr<Game> _game) : game(_game) {}

    unsigned int SelectTile() override
    {
        constexpr int ITERATIONS{ 1200 };

        // store state of the game
        vector<Team> sourceBoard {};
        for (auto& tile : game->board.tiles)
            sourceBoard.push_back(tile->team);

        // get all legal moves
        vector<unsigned int> legalMoves {};
        for (auto& tile : game->board.tiles)
            if (tile->team == Team::None)
                legalMoves.emplace_back(tile->id);

        starts = game->ai == Team::Blue ? game->board.blueStarts : game->board.redStarts;
        ends = game->ai == Team::Blue ? game->board.blueEnds : game->board.redEnds;

        // iterate for a statistically significant sample
        map<unsigned int, unsigned int> wins {};

        for (auto& legalMove : legalMoves)
            for (int i = 0; i < ITERATIONS; i++)
                if (Predict(legalMoves, legalMove))
                    wins[legalMove]++;
                    
        for (auto& tile : game->board.tiles)
            tile->team = sourceBoard[tile->id];

        // select the outcome with the highest win ratio;
        // which is highest wins since the iterations are fixed size
        if (wins.size() == 0)   // checkmate
            return UINT_MAX;

        unsigned int maxWins = wins[0];
        unsigned int maxWinsIndex = 0;
        for (unsigned int i = 1; i < wins.size(); i++)
        {
            if (wins[i] > maxWins)
            {
                maxWins = wins[i];
                maxWinsIndex = i;
            }
        }

        return maxWinsIndex;
    }
};

int main(void)
{
    constexpr unsigned int rows{ 11 };
    constexpr unsigned int columns{ 11 };
    Player player{};
    Board board{ rows, columns };
    shared_ptr<Game> game { make_shared<Game>(player.SelectTeam(), board) };
    MonteCarloAI ai{ game };
    AsciiRenderer renderer{};

    do
    {
        renderer.Display(game->board);
        while (!game->Update(player.SelectTile(rows), game->player));
        while (!game->Update(ai.SelectTile(), game->ai));
    } while (!game->over);

    renderer.Display(game->board);
}