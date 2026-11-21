#include <cctype>
#include <sstream>

#include "search/MoveExecutor.h"
#include "search/header/epd.h"

bool Epd::parse(const std::string& line, EpdEntry& out) {
    out.id.clear();
    out.bm.clear();
    out.am.clear();

    std::istringstream in(line);
    std::string fields[4];
    for (std::string& f : fields) {
        if (!(in >> f)) return false;
    }
    out.fen = fields[0] + " " + fields[1] + " " + fields[2] + " " + fields[3];

    std::string rest;
    std::getline(in, rest);

    // shortcut: splits on ';' so a ';' inside a quoted operand would be cut, no WAC line has one, upgrade if a suite does
    std::istringstream ops(rest);
    std::string op;
    while (std::getline(ops, op, ';')) {
        std::istringstream tokens(op);
        std::string opcode;
        if (!(tokens >> opcode)) continue;

        std::vector<std::string> operands;
        std::string tok;
        while (tokens >> tok) operands.push_back(tok);

        if (opcode == "bm") {
            out.bm = operands;
        } else if (opcode == "am") {
            out.am = operands;
        } else if (opcode == "id") {
            std::string id;
            for (size_t i = 0; i < operands.size(); i++) {
                if (i > 0) id += ' ';
                for (char c : operands[i]) {
                    if (c != '"') id += c;
                }
            }
            out.id = id;
        }
    }
    return true;
}

Epd::San Epd::sanToMove(const Game& game, const std::string& san, Move& move) {
    std::string s;
    for (char c : san) {
        if (c != '+' && c != '#' && c != '!' && c != '?') s += c;
    }

    const std::vector<Move> legal = MoveExecutor::generateMoves(game);
    const Board& board = game.getBoard();
    std::vector<Move> candidates;

    if (s == "O-O" || s == "O-O-O") {
        const int delta = (s == "O-O") ? 2 : -2;
        for (const Move& m : legal) {
            const Piece* p = board.getPiece(m.fromRow, m.fromCol);
            if (p && std::toupper(p->symbol()) == 'K' && m.toCol - m.fromCol == delta) {
                candidates.push_back(m);
            }
        }
    } else {
        char piece = 'P';
        size_t start = 0;
        if (!s.empty() && std::string("NBRQK").find(s[0]) != std::string::npos) {
            piece = s[0];
            start = 1;
        }

        // shortcut: only the "e8=Q" promotion spelling is parsed, none occurs in WAC, upgrade if a suite writes "e8Q"
        char promo = 0;
        size_t eq = s.find('=');
        if (eq != std::string::npos) {
            promo = (eq + 1 < s.size()) ? s[eq + 1] : 0;
            s = s.substr(0, eq);
        }
        if (promo == 'R' || promo == 'B' || promo == 'N') return San::Underpromotion;

        if (s.size() < start + 2) return San::NoMatch;

        const char file = s[s.size() - 2];
        const char rank = s[s.size() - 1];
        if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return San::NoMatch;
        const int toCol = file - 'a';
        const int toRow = 8 - (rank - '0');

        int fromRow = -1, fromCol = -1;
        for (size_t i = start; i < s.size() - 2; i++) {
            const char c = s[i];
            if (c == 'x') continue;
            if (c >= 'a' && c <= 'h') fromCol = c - 'a';
            else if (c >= '1' && c <= '8') fromRow = 8 - (c - '0');
            else return San::NoMatch;
        }

        for (const Move& m : legal) {
            const Piece* p = board.getPiece(m.fromRow, m.fromCol);
            if (!p || std::toupper(p->symbol()) != piece) continue;
            if (m.toRow != toRow || m.toCol != toCol) continue;
            if (fromCol >= 0 && m.fromCol != fromCol) continue;
            if (fromRow >= 0 && m.fromRow != fromRow) continue;
            if (piece == 'P' && fromCol < 0 && m.fromCol != m.toCol) continue;
            candidates.push_back(m);
        }
    }

    if (candidates.size() != 1) return San::NoMatch;
    move = candidates[0];
    return San::Ok;
}
