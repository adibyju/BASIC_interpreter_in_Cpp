#include "string_with_arrows.h"
#include <algorithm>

std::string string_with_arrows(const std::string& text, const Position& pos_start, const Position& pos_end) {
    std::string result = "";

    // Calculate indices
    int idx_start = std::max(static_cast<int>(text.rfind('\n', pos_start.idx - 1)), 0);
    int idx_end = static_cast<int>(text.find('\n', idx_start + 1));
    if (idx_end < 0) idx_end = text.length();

    // Generate each line
    int line_count = pos_end.ln - pos_start.ln + 1;
    for (int i = 0; i < line_count; i++) {
        // Calculate line columns
        std::string line = text.substr(idx_start, idx_end - idx_start);
        int col_start = (i == 0) ? pos_start.col : 0;
        int col_end = (i == line_count - 1) ? pos_end.col : line.length() - 1;

        // Append to result
        result += line + '\n';
        result += std::string(col_start, ' ') + std::string(col_end - col_start, '^') + '\n';

        // Re-calculate indices
        idx_start = idx_end;
        idx_end = static_cast<int>(text.find('\n', idx_start + 1));
        if (idx_end < 0) idx_end = text.length();
    }

    // Replace tabs with spaces
    result.erase(remove(result.begin(), result.end(), '\t'), result.end());

    return result;
}
