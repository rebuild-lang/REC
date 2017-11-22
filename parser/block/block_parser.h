#pragma once
#include "block_token.h"

#include "meta/co_enumerator.h"

namespace parser::block {

using view_t = strings::utf8_view;
using column_t = scanner::column_t;
using filter_token = filter::token;

/**
 * @brief block and line grouping parser is the 2nd parser step
 *
 * scans all the indentations and blocks
 *
 */
struct parser {
    static auto parse(meta::co_enumerator<filter_token> input) -> block_literal {
        if (!input++) return {};
        auto state_ = state_t{};
        auto block_column = column_t{};
        if (input->one_of<filter::new_line_indentation>()) {
            block_column = state_.get_indent_column(*input);
            if (!input++) return {};
        }
        auto block = parse_block(input, block_column, state_);
        if (input) {
            // TODO: report extra input
        }
        return block;
    }

private:
    using optional_char = meta::optional<meta::packed<char>>;
    struct state_t {
        optional_char indent_char{};

        auto get_indent_column(const filter_token &tok) -> column_t {
            const auto &range = tok.range;
            // TODO: extract indent char & verify it!
            // const auto &text = range.text;
            //        if (!text.is_empty()) {
            //            if (!indent_char) {
            //                auto opt_cp = view_t{text}.pull_code_point();
            //                if (!opt_cp || opt_cp.value().v > 255) {
            //                    // error
            //                }
            //                else {
            //                    indent_char = static_cast<char>(opt_cp.value().v);
            //                }
            //            }
            //            // TODO: verify indent char
            //        }
            return range.end_position.column;
        }
    };
    using input_t = meta::co_enumerator<filter_token>;
    using group_token = block::token;
    using token_line = block::line;

    static group_token translate(filter_token &&tok) {
        return std::move(tok.data).visit(
            [](filter::new_line_indentation &&) { return group_token{}; },
            [](filter::block_start_indentation &&) { return group_token{}; },
            [](filter::block_end_indentation &&) { return group_token{}; },
            [](filter::semicolon_separator &&) { return group_token{}; },
            [&](auto &&d) {
                return group_token{std::move(tok.range), std::move(d)};
            });
    }

    static auto extract_line_tokens(token_line &line, input_t &input) {
        using namespace filter;
        // TODO: add semicolon
        while (!input->one_of<
                new_line_indentation,
                block_start_indentation,
                block_end_indentation,
                semicolon_separator>()) {
            line.emplace_back(translate(input.move()));
            if (!input++) return;
        }
    }

    static auto parse_line(input_t &input, column_t parent_block_column, state_t &state) -> token_line {
        using namespace filter;
        auto line = token_line{};
        auto expect_end = false;
        while (true) {
            extract_line_tokens(line, input);
            if (!input) return line;

            while (true) {
                if (input->one_of<semicolon_separator>()) {
                    if (expect_end) {
                        // TODO: report error
                        // handling ignore
                    }
                    input++; // consume semicolon
                    return line; // semicolon terminates current line
                }
                else if (input->one_of<new_line_indentation>()) {
                    auto next_column = state.get_indent_column(*input);
                    if (next_column < parent_block_column) {
                        if (expect_end) {
                            // TODO report missing end
                            // handling terminate line anyways
                        }
                        return line; // end of line in parent
                    }
                    if (next_column == parent_block_column && !expect_end) return line; // regular line break
                    // next_column > parent_block_column => continuation
                    while (true) {
                        if (!input++) return line;
                        extract_line_tokens(line, input);
                        if (!input) return line;

                        if (!input->one_of<new_line_indentation>()) break;
                        // auto continue_column = state_.get_indent_column(*input);
                        // if (continue_column >= next_column) continue;
                        // TODO report continuation error
                        // TODO handling: add lines to a block as well
                    }
                }
                else if (input->one_of<block_end_indentation>()) {
                    auto next_column = state.get_indent_column(*input);
                    if (next_column < parent_block_column) {
                        if (expect_end) {
                            // TODO report missing end
                            // handling terminate line anyways
                        }
                        return line; // end of block in parent
                    }
                    if (next_column == parent_block_column) {
                        if (!expect_end) {
                            // TODO report unexpected end
                            // handling terminate line anyways
                        }
                        input++; // consume the end
                        return line;
                    }
                    // TODO report nested end
                    // handling: ignored
                    if (!input++) return line;
                }
                else if (input->one_of<block_start_indentation>()) {
                    auto next_column = state.get_indent_column(*input);
                    expect_end = true;
                    if (next_column < parent_block_column) {
                        // TODO report missing end
                        // handling: add empty block and finish line
                        line.push_back({input->range, block_literal{}});
                        return line;
                    }
                    if (next_column == parent_block_column) { // empty block
                        line.push_back({input->range, block_literal{}});
                        if (!input++) return line;
                    }
                    else {
                        auto range = input->range;
                        auto block = parse_block(input, next_column, state);
                        line.push_back({std::move(range), std::move(block)});
                    }
                }
                else
                    break;
            }
        }
    }

    static auto parse_block(input_t &input, column_t block_column, state_t &state) -> block_literal {
        using namespace filter;
        auto block = block_literal{};
        while (true) {
            while (true) {
                if (input->one_of<semicolon_separator>()) {
                    if (!input++) return block;
                }
                else if (input->one_of<block_end_indentation>()) {
                    auto indent = state.get_indent_column(*input);
                    if (indent < block_column) return block; // do not consume parent end block
                    // TODO report misplaced end
                    // handling: ignore it
                    if (!input++) return block;
                }
                else if (input->one_of<block_start_indentation, new_line_indentation>()) {
                    auto indent = state.get_indent_column(*input);
                    if (indent < block_column) return block; // line is not part of this block
                    if (indent > block_column) {
                        // TODO: report indentation error
                        // handling: take the line into this block
                    }
                    if (!input++) return block;
                }
                else
                    break;
            }
            auto line = parse_line(input, block_column, state);
            block.lines.emplace_back(std::move(line));
            if (!input) break;
        }
        return block;
    }
};

} // namespace parser::block