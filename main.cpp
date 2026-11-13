#include <iostream>
#include <vector>
#include <limits>
#include <random>
#include <optional>

enum class Cell
{
    Empty,
    Red,
    Yellow
};

using ConnectFourBoard = std::vector<std::vector<Cell>>;

class Board
{
private:
    ConnectFourBoard board_;
    std::vector<int> columnHeight_;

public:
    // Default Constructor
    Board() : board_(6, std::vector<Cell>(7, Cell::Empty)), columnHeight_(7, 0)
    {
    }

    std::optional<int> dropPiece(int column, Cell player)
    {
        if (player == Cell::Empty || column < 0 || column >= 7)
            return std::nullopt;

        const int maxHeight = 6;
        if (columnHeight_[column] < maxHeight)
        {
            const int row = columnHeight_[column]++;
            board_[row][column] = player;

            return row;
        }

        return std::nullopt;
    }

    bool checkWinFrom(int row, int column) const
    {
        constexpr int rows = 6;
        constexpr int columns = 7;

        if (row < 0 || row >= rows || column < 0 || column >= columns)
            return false;

        const Cell player = board_[row][column];
        if (player == Cell::Empty)
            return false;

        // Horizontal, vertical, and the two diagonals.
        constexpr int directions[4][2] = {
            {0, 1},
            {1, 0},
            {1, 1},
            {1, -1}};

        for (const auto &direction : directions)
        {
            int count = 1; // Include the starting piece.

            for (int sign : {-1, 1})
            {
                for (int step = 1; step <= 3; ++step)
                {
                    const int nextRow = row + sign * step * direction[0];
                    const int nextColumn = column + sign * step * direction[1];

                    if (nextRow < 0 || nextRow >= rows || nextColumn < 0 || nextColumn >= columns)
                    {
                        break;
                    }

                    if (board_[nextRow][nextColumn] != player)
                        break;

                    if (++count >= 4)
                        return true;
                }
            }
        }

        return false;
    }

    bool isFull() const
    {
        constexpr int maxHeight = 6;

        for (auto columnHeight : columnHeight_)
        {
            if (columnHeight < maxHeight)
                return false;
        }

        return true;
    }

    bool canDrop(int column) const
    {
        constexpr int maxHeight = 6;

        return column >= 0 && column < 7 && columnHeight_[column] < maxHeight;
    }

    void print() const
    {
        for (int row = 5; row >= 0; --row)
        {
            for (int column = 0; column < 7; ++column)
            {
                char symbol = '?';
                switch (board_[row][column])
                {
                case Cell::Red:
                    symbol = 'R';
                    break;

                case Cell::Yellow:
                    symbol = 'Y';
                    break;
                case Cell::Empty:
                    symbol = '.';
                    break;
                }

                std::cout << symbol << " ";
            }
            std::cout << '\n';
        }
        std::cout << "0 1 2 3 4 5 6\n";
    }
};

enum class GameStatus
{
    InProgress,
    YellowWins,
    RedWins,
    Draw
};

class ConnectFourGame
{
private:
    Board board_;
    Cell currentPlayer_;
    GameStatus gameStatus_;

public:
    // default constructor
    ConnectFourGame() : currentPlayer_{Cell::Red}, gameStatus_{GameStatus::InProgress}
    {
    }

    Cell getCurrentPlayer() const { return currentPlayer_; }

    GameStatus getGameStatus() const { return gameStatus_; }

    bool playMove(int column)
    {
        if (gameStatus_ != GameStatus::InProgress)
            return false;

        const auto placedRow = board_.dropPiece(column, currentPlayer_);

        if (!placedRow.has_value())
            return false;

        const int row = placedRow.value();
        if (board_.checkWinFrom(row, column))
        {
            gameStatus_ = (currentPlayer_ == Cell::Red) ? GameStatus::RedWins : GameStatus::YellowWins;
        }
        else if (board_.isFull())
        {
            gameStatus_ = GameStatus::Draw;
        }
        else
        {
            currentPlayer_ = (currentPlayer_ == Cell::Red) ? Cell::Yellow : Cell::Red;
        }

        return true;
    }

    std::vector<int> legalMoves() const
    {
        if (gameStatus_ != GameStatus::InProgress)
            return {};

        std::vector<int> result;
        constexpr int numColumn = 7;

        for (int column = 0; column < numColumn; ++column)
        {
            if (board_.canDrop(column))
                result.push_back(column);
        }

        return result;
    }

    void print() const
    {
        board_.print();
    }
};


std::optional<int> chooseRandomMove(const ConnectFourGame &game, std::mt19937 &rng)
{
    const auto moves = game.legalMoves();

    if (moves.empty())
        return std::nullopt;

    std::uniform_int_distribution<int> uniform{0, static_cast<int>(moves.size()) - 1};

    return moves.at(uniform(rng));
}

int main()
{
    ConnectFourGame game;
    std::mt19937 rng{42};
    int moveCount = 0;

    std::cout << "You are Red. The computer is Yellow.\n";

    while (game.getGameStatus() == GameStatus::InProgress)
    {
        std::cout << "Choose a column (0-6): ";

        int myMove;
        if (!(std::cin >> myMove))
        {
            if (std::cin.eof() || std::cin.bad())
            {
                std::cout << "\nGame stopped.\n";
                return 0;
            }

            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Please enter an integer.\n";
            continue;
        }

        if (!game.playMove(myMove))
        {
            std::cout << "Invalid move. Choose a column with space.\n";
            continue;
        }

        ++moveCount;
        game.print();

        // The human's move might have ended the game.
        if (game.getGameStatus() != GameStatus::InProgress)
            break;

        const auto opponentMove = chooseRandomMove(game, rng);

        if (!opponentMove.has_value() || !game.playMove(*opponentMove))
        {
            std::cerr << "Error: the computer could not make a legal move.\n";
            return 1;
        }

        ++moveCount;
        game.print();
        std::cout << "Computer chose column " << *opponentMove << ".\n";
    }

    switch (game.getGameStatus())
    {
    case GameStatus::RedWins:
        std::cout << "You win!\n";
        break;

    case GameStatus::YellowWins:
        std::cout << "Computer wins!\n";
        break;

    case GameStatus::Draw:
        std::cout << "It's a draw!\n";
        break;

    case GameStatus::InProgress:
        break;
    }

    std::cout << "Total moves: " << moveCount << '\n';

    return 0;
}