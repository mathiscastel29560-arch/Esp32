#include "ir_learning.h"
namespace IRLearning {
LearnResult learn(uint16_t timeoutMs) { return {false, "", 0, "IR learning stub - awaiting hardware"}; }
ReplayResult replay(const String &irCode, uint8_t repeats) { return {false, 0, 0}; }
String listLearned() { return "No learned codes"; }
} // namespace IRLearning
