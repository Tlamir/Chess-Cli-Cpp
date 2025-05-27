#include <iostream>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <string>
#include <algorithm>

/*
  Chess Board bitboard
  64-bit number (1 bit per square)
  a8 = 56  b8 = 57  c8 = 58  d8 = 59  e8 = 60  f8 = 61  g8 = 62  h8 = 63
  a7 = 48  b7 = 49  c7 = 50  d7 = 51  e7 = 52  f7 = 53  g7 = 54  h7 = 55
  a6 = 40  b6 = 41  c6 = 42  d6 = 43  e6 = 44  f6 = 45  g6 = 46  h6 = 47
  a5 = 32  b5 = 33  c5 = 34  d5 = 35  e5 = 36  f5 = 37  g5 = 38  h5 = 39
  a4 = 24  b4 = 25  c4 = 26  d4 = 27  e4 = 28  f4 = 29  g4 = 30  h4 = 31
  a3 = 16  b3 = 17  c3 = 18  d3 = 19  e3 = 20  f3 = 21  g3 = 22  h3 = 23
  a2 = 8   b2 = 9   c2 = 10  d2 = 11  e2 = 12  f2 = 13  g2 = 14  h2 = 15
  a1 = 0   b1 = 1   c1 = 2   d1 = 3   e1 = 4   f1 = 5   g1 = 6   h1 = 7
*/


static const inline uint64_t pos(int square)
{
	return 1ULL << square;
}

class Board
{
private:
	uint64_t wKing, wQueen, wRook, wBishop, wKnight, wPawn,
		bKing, bQueen, bRook, bBishop, bKnight, bPawn;

	std::unordered_map<char, uint64_t*> pieceBitboards;

	const int notationToIndex(const std::string& square)
	{
		if (square.length() != 2) return -1;

		char file = tolower(square[0]);
		char rank = square[1];

		if (file < 'a' || file > 'h') return -1;
		if (rank < '1' || rank > '8') return -1;

		int fileIndex = file - 'a';
		int rankIndex = rank - '1';

		return rankIndex * 8 + fileIndex;
	}

public:
	Board() :
		// Fill board
		wKing(pos(4)),
		wQueen(pos(3)),
		wRook(pos(0) | pos(7)),
		wBishop(pos(2) | pos(5)),
		wKnight(pos(1) | pos(6)),
		wPawn(0xFF00),

		bKing(pos(60)),
		bQueen(pos(59)),
		bRook(pos(56) | pos(63)),
		bBishop(pos(58) | pos(61)),
		bKnight(pos(57) | pos(62)),
		bPawn(0xFF000000000000)
	{
		pieceBitboards =
		{
			{'K', &wKing}, {'Q', &wQueen}, {'R', &wRook}, {'B', &wBishop}, {'N', &wKnight}, {'P', &wPawn},
			{'k', &bKing}, {'q', &bQueen}, {'r', &bRook}, {'b', &bBishop}, {'n', &bKnight}, {'p', &bPawn}
		};
	}

	bool isSquareFull(int square) const
	{
		return getAllFullSquares() & pos(square);
	}

	char getPieceAt(int square) const
	{
		for (const auto& it : pieceBitboards)
		{
			if (*(it.second) & pos(square))
				return it.first;
		}
		return '-';
	}

	void clearSquare(int square)
	{
		uint64_t mask = ~pos(square);
		for (auto& it : pieceBitboards)
		{
			*(it.second) &= mask;
		}
	}

	void setSquare(int square, char piece)
	{
		clearSquare(square);
		if (pieceBitboards.count(piece))
			*pieceBitboards[piece] |= pos(square);

	}

	void movePiece(int from, int to)
	{
		char piece = getPieceAt(from);
		if (piece != '-')
		{
			clearSquare(from);
			setSquare(to, piece);
		}
	}

	bool processMove(const std::string& from, const std::string& to,bool isWhitePlaying)
	{
		int fromIndex = notationToIndex(from);
		int toIndex = notationToIndex(to);

		if (fromIndex == -1 || toIndex == -1)
		{
			std::cout << "Invalid square notation.\n";
			return false;
		}

		if (!isSquareFull(fromIndex))
		{
			std::cout << "No piece at " << from << ".\n";
			return false;
		}

		char piece = getPieceAt(fromIndex);

		// Check piece color 
		if (isWhitePlaying && !isupper(piece))
		{
			std::cout << "It's White's turn. Move a white piece.\n";
			return false;
		}
		if (!isWhitePlaying && !islower(piece))
		{
			std::cout << "It's Black's turn. Move a black piece.\n";
			return false;
		}

		movePiece(fromIndex, toIndex);
		return true;
	}

	void drawBoard() const
	{
		for (int r = 7; r >= 0; --r)
		{
			std::cout << (r + 1) << " ";
			for (int c = 0; c < 8; ++c)
			{
				std::cout << getPieceAt(r * 8 + c) << ' ';
			}
			std::cout << std::endl;
		}
		std::cout << "  a b c d e f g h\n";
	}

	uint64_t getAllFullSquares() const
	{
		uint64_t result = 0;
		for (const auto& it : pieceBitboards)
		{
			result |= *(it.second);
		}
		return result;
	}
};

class Game
{
private:
	std::unique_ptr<Board> m_Board = std::make_unique<Board>();

public:
	Game() : m_Board(std::make_unique<Board>()) {}
	~Game() {};

	void start()
	{
		std::cout << "\n    Welcome to Chess\n" << "Enter moves in format 'e2 e4'\n\n";
		static bool isWhitePlaying = true;
		while (true)
		{
			std::cout << std::endl;
			m_Board->drawBoard();
			isWhitePlaying ? std::cout << "White Move: " : std::cout << "Black move: ";
			std::string input;
			std::getline(std::cin, input);

			// Remove extra spaces
			input.erase(std::unique(input.begin(), input.end(),
				[](char a, char b) { return a == ' ' && b == ' '; }),
				input.end());

			size_t spacePos = input.find(' ');
			if (spacePos != std::string::npos && spacePos > 0 && spacePos < input.length() - 1)
			{
				std::string from = input.substr(0, spacePos);
				std::string to = input.substr(spacePos + 1);

				// Check move  
				if (m_Board->processMove(from, to, isWhitePlaying))
					isWhitePlaying = !isWhitePlaying;
			}
			else
				std::cout << "Invalid input format. Please use format like 'e2 e4'.\n";
		}
	}


};


int main()
{
	Game chessGame;
	chessGame.start();

	return 0;
}