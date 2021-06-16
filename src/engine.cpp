#include "engine.h"

#include <time.h>

#include <algorithm>
#include <iostream>

Move get_best_move(ChessEvaluation &board) {
    // todo: check tables and book
    vector<Move> moves;
    board.GenLegalMoveListSorted(moves);
    Move best_move = moves.back();
    for (depth_iter = 1; depth_iter <= MAX_DEPTH; depth_iter++) {
        best_move = negamax_root(board, moves, depth_iter, NEG_INF, POS_INF);
        moves.erase(remove(moves.begin(), moves.end(), best_move), moves.end());
        moves.push_back(best_move);
    }
    return best_move;
}

Move negamax_root(ChessEvaluation &board, vector<Move> &moves, int depth,
                  int alpha, int beta) {
    if (in_check(board)) {
        depth++;
    }

    uint64_t trans_key = board.z_hash;

    if (trans_table.count(trans_key) > 0) {
        P info = trans_table.at(trans_key);
        int d = info.depth;
        int t = info.bound_type;
        int v = info.best_val;
        if (d >= depth) {
            if (t == P::EXACT) {
                return info.best_move;
            } else if (t == P::LOWER && v > alpha) {
                alpha = v;
            } else if (t == P::UPPER && v < beta) {
                beta = v;
            }
            if (alpha >= beta) {
                return info.best_move;
            }
        }
    }

    if (trans_table.count(trans_key) > 0) {
        // do something
    }

    int hi = NEG_INF;
    Move best_move = moves.back();
    int val = NEG_INF;
    for (Move mv : moves) {
        push_move(board, mv);
        val = -negamax(board, depth - 1, -beta, -alpha);
        pop_move(board, mv);

        if (val > hi) {
            hi = val;
            best_move = mv;
        }

        if (hi > alpha) {
            alpha = hi;
        }

        if (hi >= beta) {
            break;
        }
    }
    update_trans_table(board, alpha, beta, depth, hi, best_move);
    return best_move;
}

int negamax(ChessEvaluation &board, int depth, int alpha, int beta) {
    if (in_check(board)) {
        depth++;
    }

    uint64_t trans_key = board.z_hash;

    if (trans_table.count(trans_key) > 0) {
        P info = trans_table.at(trans_key);
        int d = info.depth;
        int t = info.bound_type;
        int v = info.best_val;
        if (d >= depth) {
            if (t == P::EXACT) {
                return v;
            } else if (t == P::LOWER && v > alpha) {
                alpha = v;
            } else if (t == P::UPPER && v < beta) {
                beta = v;
            }
            if (alpha >= beta) {
                return v;
            }
        }
    }

    if (board.GetRepetitionCount() > 1 || board.IsDraw()) {
        return 0;
    }

    if (depth == 0) {
        if (depth_iter < MIN_DEPTH) {
            return qsearch(board, Q_MIN_DEPTH, alpha, beta);
        } else if (depth_iter == MIN_DEPTH) {
            return qsearch(board, Q_MID_DEPTH, alpha, beta);
        }
        return qsearch(board, Q_MAX_DEPTH, alpha, beta);
    }

    vector<Move> moves;
    board.GenLegalMoveListSorted(moves);

    if (moves.empty()) {
        if (in_check(board)) {
            return -MATE - depth;
        }
        return 0;
    }

    int hi = NEG_INF;
    Move best_move = moves.back();
    int val = NEG_INF;

    for (Move mv : moves) {
        push_move(board, mv);
        val = -negamax(board, depth - 1, -beta, -alpha);
        pop_move(board, mv);

        if (val > hi) {
            hi = val;
            best_move = mv;
        }

        if (hi > alpha) {
            alpha = hi;
        }

        if (hi >= beta) {
            break;
        }
    }
    update_trans_table(board, alpha, beta, depth, hi, best_move);
    return hi;
}

int qsearch(ChessEvaluation &board, int depth, int alpha, int beta) {
    int qdepth = depth + depth_iter;
    int trans_key = board.z_hash;

    if (trans_table.count(trans_key) > 0) {
        P info = trans_table.at(trans_key);
        int d = info.depth;
        int t = info.bound_type;
        int v = info.best_val;
        if (d >= qdepth) {
            if (t == P::EXACT) {
                return v;
            } else if (t == P::LOWER && v > alpha) {
                alpha = v;
            } else if (t == P::UPPER && v < beta) {
                beta = v;
            }
            if (alpha >= beta) {
                return v;
            }
        }
    }

    int eval = evaluate(board);

    if (depth == 0) {
        return eval;
    }

    if (eval >= beta) {
        return eval;
    }

    vector<Move> non_quiet_moves;
    gen_non_quiet(board, non_quiet_moves);

    if (non_quiet_moves.empty()) {
        return eval;
    }

    if (eval > alpha) {
        alpha = eval;
    }

    int hi = NEG_INF;
    Move best_move = non_quiet_moves.front();
    for (Move mv : non_quiet_moves) {
        push_move(board, mv);
        eval = -qsearch(board, depth - 1, -beta, -alpha);
        pop_move(board, mv);

        if (eval > hi) {
            hi = eval;
            best_move = mv;
        }

        if (hi > alpha) {
            alpha = hi;
        }

        if (hi >= beta) {
            break;
        }
    }
    update_trans_table(board, alpha, beta, depth, hi, best_move);
    return hi;
}

int evaluate(ChessEvaluation &board) {
    int material;
    int positional;
    board.EvaluateLeaf(material, positional);
    return material * 4 + positional;
}

bool is_non_quiet(Move &mv) {
    if (mv.capture != ' ') {
        return true;
    }
    if (mv.special == SPECIAL_PROMOTION_QUEEN ||
        mv.special == SPECIAL_PROMOTION_ROOK ||
        mv.special == SPECIAL_PROMOTION_KNIGHT ||
        mv.special == SPECIAL_PROMOTION_BISHOP) {
        return true;
    }
    return false;
}

void gen_non_quiet(ChessEvaluation &board, vector<Move> &non_quiet_moves) {
    vector<Move> legal_moves;
    board.GenLegalMoveListSortedReg(legal_moves);
    for (Move mv : legal_moves) {
        if (is_non_quiet(mv)) {
            non_quiet_moves.push_back(mv);
        }
    }
}

void push_move(ChessEvaluation &board, Move &mv) {
    board.PushMove(mv);
    update_hash(board);
}

void pop_move(ChessEvaluation &board, Move &mv) {
    board.PopMove(mv);
    update_hash(board);
}

void update_pos_info(P &pos, int val, Move &mv, int type, int depth) {
    if (depth > pos.depth) {
        pos.best_val = val;
        pos.bound_type = type;
        pos.best_move = mv;
        pos.depth = depth;
    }
}

void update_trans_table(ChessEvaluation &board, int alpha, int beta, int depth,
                        int hi, Move &mv) {
    int bound = P::EXACT;
    if (hi <= alpha) {
        bound = P::LOWER;
    } else if (hi >= beta) {
        bound = P::UPPER;
    }
    if (trans_table.count(board.z_hash) > 0) {
        update_pos_info(trans_table.at(board.z_hash), hi, mv, bound, depth);
    } else {
        P new_pos;
        new_pos.best_val = hi;
        new_pos.best_move = mv;
        new_pos.bound_type = bound;
        new_pos.depth = depth;
        trans_table.insert(make_pair(board.z_hash, new_pos));
    }
}

// taken from thc demo
void display_position(thc::ChessRules &cr, const std::string &description) {
    std::string fen = cr.ForsythPublish();
    std::string s = cr.ToDebugStr();
    printf("%s\n", description.c_str());
    printf("FEN (Forsyth Edwards Notation) = %s\n", fen.c_str());
    printf("Position = %s\n", s.c_str());
}

void play(ChessEvaluation &board, Move &mv, const char *move) {
    mv.NaturalIn((ChessRules *)&board, move);
    board.PlayMove(mv);
    update_hash(board);
}

int main() {
    init_hash();

    ChessEvaluation board;
    Move mv;

    board.Forsyth("8/8/8/8/8/3qk3/8/2K5 b - - 0 1");

    clock_t start, end;
    double cpu_time_used;

    start = clock();

    for (int i = 1; i <= 7; i++) {
        mv = get_best_move(board);
        if (i % 2 != 0) {
            cout << ((i / 2) + 1) << ". ";
        }
        cout << mv.NaturalOut((ChessRules *)&board) << " ";
        board.PlayMove(mv);
        update_hash(board);
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    cout << endl << "ELAPSED TIME: " <<  (cpu_time_used * 1000) << " ms" << endl;

    display_position(board, "\nFinal position");

    return 0;
}