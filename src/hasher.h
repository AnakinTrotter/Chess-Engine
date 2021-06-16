#include <random>

#include "thc.h"

namespace hasher {
void init_hash();
void update_hash(thc::ChessEvaluation &board);
}  // namespace hasher