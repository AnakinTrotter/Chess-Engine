#include "hasher.h"

// +1 because that's what they did in thc.h
uint64_t z_pieces[2][6][64 + 1];
uint64_t z_enpassant[64 + 1];
uint64_t z_castle[4];
uint64_t z_black_move;

constexpr int SEED = 69;

void hasher::init_hash() {
    std::default_random_engine rand_gen;
    rand_gen.seed(SEED);
    std::uniform_int_distribution<uint64_t> dist_rand(0, ULLONG_MAX);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 64; k++) {
                z_pieces[i][j][k] = dist_rand(rand_gen);
            }
        }
    }
    for (int i = 0; i < 64; i++) {
        z_enpassant[i] = dist_rand(rand_gen);
    }
    for (int i = 0; i < 4; i++) {
        z_castle[i] = dist_rand(rand_gen);
    }
    z_black_move = dist_rand(rand_gen);
}

void hasher::update_hash(thc::ChessEvaluation &board) {
    board.z_hash = 0;
    for (int i = 0; i < 64; i++) {
        char c = board.squares[i];
        if (c == '.') {
            continue;
        } else if (c == 'p') {
            board.z_hash ^= z_pieces[1][0][i];
        } else if (c == 'P') {
            board.z_hash ^= z_pieces[0][0][i];
        } else if (c == 'n') {
            board.z_hash ^= z_pieces[1][1][i];
        } else if (c == 'N') {
            board.z_hash ^= z_pieces[0][1][i];
        } else if (c == 'b') {
            board.z_hash ^= z_pieces[1][2][i];
        } else if (c == 'B') {
            board.z_hash ^= z_pieces[0][2][i];
        } else if (c == 'r') {
            board.z_hash ^= z_pieces[1][3][i];
        } else if (c == 'R') {
            board.z_hash ^= z_pieces[0][3][i];
        } else if (c == 'q') {
            board.z_hash ^= z_pieces[1][4][i];
        } else if (c == 'Q') {
            board.z_hash ^= z_pieces[0][4][i];
        } else if (c == 'k') {
            board.z_hash ^= z_pieces[1][5][i];
        } else if (c == 'K') {
            board.z_hash ^= z_pieces[0][5][i];
        }
    }
    if (board.wking_allowed()) {
        board.z_hash ^= z_castle[0];
    }
    if (board.wqueen_allowed()) {
        board.z_hash ^= z_castle[1];
    }
    if (board.bking_allowed()) {
        board.z_hash ^= z_castle[2];
    }
    if (board.bqueen_allowed()) {
        board.z_hash ^= z_castle[3];
    }
    if (!board.WhiteToPlay()) {
        board.z_hash ^= z_black_move;
    }
    board.z_hash ^= z_enpassant[board.groomed_enpassant_target()];
}