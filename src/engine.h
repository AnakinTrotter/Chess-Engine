#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "hasher.h"
#include "thc.h"

using namespace thc;
using namespace std;
using namespace hasher;

typedef struct pos_info {
    static constexpr int LOWER = 0;
    static constexpr int EXACT = 1;
    static constexpr int UPPER = 2;

    int best_val;
    Move best_move;
    int bound_type;
    int depth;
} P;

void gen_non_quiet(ChessEvaluation &board, vector<Move> &non_quiet_moves);
bool is_non_quiet(Move &mv);
int evaluate(ChessEvaluation &board);
void update_trans_table(ChessEvaluation &board, int alpha, int beta, int depth, int hi, Move &mv);
void update_pos_info(P &pos, int val, Move &mv, int type, int depth);
void push_move(ChessEvaluation &board, Move &mv);
void pop_move(ChessEvaluation &board, Move &mv);
int qsearch(ChessEvaluation &board, int depth, int alpha, int beta);
int negamax(ChessEvaluation &board, int depth, int alpha, int beta);
Move negamax_root(ChessEvaluation &board, vector<Move> &moves, int depth,
                  int alpha, int beta);
Move get_best_move(ChessEvaluation &board);
void display_position(thc::ChessRules &cr, const std::string &description);

int depth_iter;

static constexpr int POS_INF = 1000000;
static constexpr int NEG_INF = -1000000;
static constexpr int MIN_DEPTH = 2;
static constexpr int MAX_DEPTH = 3;
static constexpr int Q_MIN_DEPTH = 2;
static constexpr int Q_MID_DEPTH = 5;
static constexpr int Q_MAX_DEPTH = 100;
static constexpr int MATE = 300000;

unordered_map<uint64_t, P> trans_table;

static bool in_check(ChessEvaluation &board) {
    return board.AttackedPiece(board.wking_square) ||
           board.AttackedPiece(board.bking_square);
}