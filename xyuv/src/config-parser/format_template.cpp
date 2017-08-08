/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Stian Valentin Svedenborg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "xyuv/format_template.h"
#include "minicalc/minicalc.h"
#include "../config_parser.h"
#include "parsing_helpers.h"

#include <map>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "../config_parser.h"
#include "xyuv/structures/constants.h"
#include "../to_string.h"
#include "parsing_helpers.h"
#include "minicalc/ast.h"
#include "../assert.h"
#include "../utility.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <unordered_map>
#include <iostream>
#include <limits>
#include <set>
#include <xyuv/structures/format.h>
#include <sstream>

#define DECORATE(name, stmts) \
do {\
    try {\
        stmts;\
    } catch (parse_error & e) {\
        throw parse_error(std::string(e.what()) + " while parsing '" + name + "'");\
    }\
} while (0)

namespace xyuv {


    // Statically declare the supported constants.
    static std::set<std::string> global_constants = {
            // Image origin
            "upper_left", "lower_left",

            // INTERLEAVE_MODES
            "NO_INTERLEAVING", "INTERLEAVE_1_3_5__0_2_4", "INTERLEAVE_0_2_4__1_3_5",

            // BLOCK_ORDER_SWIZZLE
            "NOT_USED",
    };

    // Statically declare the special variables.
    static std::set<std::string> special_variable = {
            // Image size
            "image_w", "image_h",

            // Sample generator values
            "block_x", "block_y"
    };

    static const std::map<std::string, xyuv::interleave_pattern> str2interleave_pattern = {
            { "NO_INTERLEAVING", xyuv::interleave_pattern::NO_INTERLEAVING},
            { "INTERLEAVE_0_2_4__1_3_5", xyuv::interleave_pattern::INTERLEAVE_0_2_4__1_3_5 },
            { "INTERLEAVE_1_3_5__0_2_4", xyuv::interleave_pattern::INTERLEAVE_1_3_5__0_2_4 }
    };

    static const std::map<std::string, xyuv::image_origin> str2origin = {
            { "lower_left", xyuv::image_origin::LOWER_LEFT},
            { "upper_left", xyuv::image_origin::UPPER_LEFT}
    };

    template <typename T>
    static T str2enum(const std::string & str, const std::map<std::string, T> & _map) {
        auto it = _map.find(str);
        if (it == _map.end()) {
            std::ostringstream msg;
            msg << "Unexpected enum: '" << str << "' expected one of {";
            uint32_t i = 0;
            for (auto & elem: _map) {
                msg << elem.first;
                if (i < _map.size()-1) {
                    msg << ", ";
                }
                i++;
            }
            msg << "}.";

            throw parse_error(msg.str());
        }
        return it->second;
    }

    static const std::string sample_fields[] = { "plane", "int_bits", "frac_bits", "offset", "has_continuation"};

    class value_range_error : public parse_error {
    public:
        value_range_error(AST::value::ival_t low_inclusive, AST::value::ival_t high_inclusive, AST::value::ival_t value) : parse_error("Value out of range'" \
                        "' expected value in [" + to_string(low_inclusive) + ", " + to_string(high_inclusive) + "] was " \
                        + to_string(value)) {}
    };

    class format_template_impl {
    private:
        std::map<std::string, std::shared_ptr<MiniCalc>> fields;
        std::unordered_map<std::string, AST::value> constants;
        std::vector<std::string> resolve_order;
        std::set<std::string> auto_generate_samples;
        uint32_t n_planes = 0;
        bool rgb_mode = false;

        void set_constant(const std::string & parent, const std::string & name, const AST::value & value) {
            std::string full_name = parent + (parent != "" ? "." : "") + name;
            this->constants.emplace(full_name, value);
        }

        const AST::value get_constant(const std::string & parent, const std::string & name) {
            std::string full_name = parent + (parent != "" ? "." : "") + name;
            auto it = this->constants.find(full_name);
            XYUV_ASSERT(it != this->constants.end() && "Programming error");

            return it->second;
        }

        void validate_int_constant_range(const std::string & parent, const std::string & name, AST::value::ival_t low_inclusive, AST::value::ival_t high_inclusive) {
            std::string full_name = parent + '.' + name;
            auto it = this->constants.find(full_name);
            if ( it != this->constants.end()) {
                XYUV_ASSERT(it->second.type == AST::value_type::INT && "Programming error, only supported for integers.");
                auto value = it->second.ival;
                if ((value) < (low_inclusive) || (high_inclusive) < (value)) {
                    throw value_range_error(low_inclusive, high_inclusive, value);
                }
            }
        }

        void parse_minicalc(const std::string & parent, const std::string & name, AST::value_type expected_type, rapidjson::Value *value) {


            if (value->IsBool()) {
                if (expected_type == AST::value_type::BOOL) {
                    set_constant(parent, name, AST::value::boolean(value->GetBool()));
                } else {
                    throw MiniCalcParseError("Unexpected constant, expected " + to_string(expected_type) + " got bool.");
                }
            } else if (value->IsInt64()) {
                if (expected_type == AST::value_type::INT) {
                    this->set_constant(parent, name, AST::value::integer(value->GetInt64()));
                } else {
                    throw MiniCalcParseError("Unexpected constant, expected " + to_string(expected_type) + " got int.");
                }
            } else {
                // If we get here, the type of the expression must be a string.
                if (! value->IsString()) {
                    throw MiniCalcParseError("Unexpected constant, expected " + to_string(expected_type) + " or minicalc expression (as a string).");
                }

                std::shared_ptr<MiniCalc> expr(new MiniCalc(std::string(value->GetString(), value->GetStringLength()), &global_constants));

                if (expr->is_const()) {
                    this->set_constant(parent, name, expr->evaluate(nullptr));
                } else {
                    std::string full_name = parent + (parent != "" ? "." : "") + name;
                    for (auto & dependency : expr->get_dependencies()) {
                        // Analyse whether this is a leaf variable or a full reference.
                        // This is luckily easy: If it contains a '.' it's a full reference, otherwise it is a leaf.
                        if (dependency.first.find('.') == std::string::npos
                            && special_variable.find(dependency.first) == special_variable.end()) {
                            // If it is a leaf-reference and not referring to one of the special variables, rename all references to full references.
                            for (auto * variable_node : dependency.second) {
                                variable_node->rename(parent + "."  + dependency.first);
                            }
                        }
                    }
                    expr->recalculate_dependencies();

                    this->fields.emplace(full_name, expr);
                }
            }
        }

        void validate_dependencies(const std::map<std::string, std::set<std::string>> & producer_to_consumer, const std::map<std::string, std::set<std::string>> & consumer_to_producer) {
            // Nothing can depend on generated samples.
            for (const auto & generated_sample : this->auto_generate_samples) {
                auto it = producer_to_consumer.find(generated_sample);
                if (it != producer_to_consumer.end() && !it->second.empty()) {
                    std::ostringstream sout;
                    sout << "Fields cannot depend on fields from generated samples. '"
                         << *it->second.begin() << "' depends on '"
                         << generated_sample << "' which is generated.";
                    throw parse_error(sout.str());
                }
            }

            // If a plane's block order is not 1, 1 require the same plane's line-stride and plane-stride to be dependent on the macro_block_w.
            for (uint32_t plane = 0; plane < this->n_planes; plane++ ) {
                std::string plane_name = "planes[" + to_string(plane) + ']';
                std::string line_stride = plane_name + ".line_stride";
                std::string plane_size = plane_name + ".plane_size";
                std::string macro_block_w = plane_name + ".block_order.macro_block_w";
                std::string macro_block_h = plane_name + ".block_order.macro_block_h";

                auto it = constants.find(macro_block_w);
                if (it == constants.end() || it->second.check_ival() != 1) {
                    // In this case, line_stride must depend on macro_block_w.
                    auto producer = producer_to_consumer.find(macro_block_w);
                    if (producer == producer_to_consumer.end() || producer->second.find(line_stride) == producer->second.end()) {
                        std::ostringstream sout;
                        sout << "Dependency error while parsing planes["
                             << plane << "]: This plane's macro_block_w != 1, but line_stride does not depend on it. "
                                "When using block_order, the image must allocate enough space to fill the equivalent of "
                                "next_multiple(image_w, planes[" << plane << "].block_order.macro_block_w).";
                        throw parse_error(sout.str());
                    }
                }

                it = constants.find(macro_block_h);
                if (it == constants.end() || it->second.check_ival() != 1) {
                    // In this case, line_stride must depend on macro_block_w.
                    auto producer = producer_to_consumer.find(macro_block_h);
                    if (producer == producer_to_consumer.end() || producer->second.find(plane_size) == producer->second.end()) {
                        std::ostringstream sout;
                        sout << "Dependency error while parsing planes["
                             << plane << "]: This plane's macro_block_h != 1, but plane_size does not depend on it. "
                                     "When using block_order, the image must allocate enough space to fill the equivalent of "
                                     "next_multiple(image_h, planes[" << plane << "].block_order.macro_block_h).";
                        throw parse_error(sout.str());
                    }
                }
            }
        }

    public:
        format_template_impl();
        void parse(const std::string &json);
        void parse_document(rapidjson::Document &root);

        // Top-level:
        void parse_fourcc(rapidjson::Value *licence_root);
        void parse_subsampling(rapidjson::Value *licence_root);

        // Parsing of planes.
        void parse_block_order(const std::string& parent, rapidjson::Value *plane_root);
        uint32_t parse_block_order_swizzle(const std::string & parent, const std::string & name, rapidjson::Value *array);
        void parse_plane(const std::string& parent, rapidjson::Value *plane_root);

        // Parsing of Channels
        void parse_block(const std::string& parent, rapidjson::Value *channel_root);
        void parse_sample(const std::string & root_name, rapidjson::Value *sample_root);

        // Inflation
        xyuv::format inflate(uint32_t image_w, uint32_t image_h, const xyuv::chroma_siting & chroma_siting, const xyuv::conversion_matrix & matrix );

        // Inflate plane
        void inflate_plane(xyuv::format & fmt, uint32_t i);
        void inflate_block_order(xyuv::block_order & bo, const std::string & parent);

        // Inflate channel_block
        void inflate_channel_block(const xyuv::format & fmt, xyuv::channel_block &target_channel_block, const std::string &name);
        void inflate_samples(const xyuv::format & fmt, xyuv::channel_block &target_channel_block, const std::string &name, bool generate);

        AST::value evaluate_generated_sample_field(const std::string &sample_base, const std::string & field_name);

    };

    xyuv::format_template_impl::format_template_impl() { }

    void format_template_impl::parse(const std::string &json) {
        rapidjson::Document d;
        d.Parse(json.c_str());

        if (d.HasParseError()) {
            rapidjson::ParseResult res(d.GetParseError(), d.GetErrorOffset());
            throw parse_error(std::string("on line ") + to_string(line_number_from_offset(json, res.Offset())) +
                              " JSON syntax error: " + rapidjson::GetParseError_En(res.Code()));
        }

        parse_document(d);
    }

    std::string find_cycle(const std::map<std::string, std::set<std::string>> & producer_to_consumer, const std::string & starting_point) {
        std::map<std::string, std::string> come_from;
        std::vector<std::string> queue;

        for (const auto & consumer: producer_to_consumer.find(starting_point)->second) {
            come_from[consumer] = starting_point;
            queue.push_back(consumer);
        }
        while (!queue.empty()) {
            std::string current = queue.back();
            queue.pop_back();

            auto it = producer_to_consumer.find(current);
            if (it != producer_to_consumer.end()) {
                for (const auto & consumer: it->second) {
                    if (come_from.find(consumer) == come_from.end()) {
                        come_from[consumer] = current;
                        queue.push_back(consumer);

                        if (consumer == starting_point) {
                            // Jackpot, we have closed the cycle, now back-track and return the result.
                            std::ostringstream sout;
                            sout << starting_point << " -> ";

                            // Reuse current
                            current = come_from[starting_point];
                            while (current != starting_point) {
                                sout << current << " -> ";
                                current = come_from[current];
                            }
                            sout << starting_point;
                            auto result = sout.str();
                            return result;
                        }
                    }
                }
            }
        }
        XYUV_ASSERT(false && "Should never get here.");
    }

    void format_template_impl::parse_document(rapidjson::Document &root) {

        DECLARE_OPTIONAL(root, fourcc, String);
        DECLARE_REQUIRED_MINICALC(root, origin);
        DECLARE_OPTIONAL(root, subsampling_mode, Object);

        DECLARE_OPTIONAL(root, y_block, Object);
        DECLARE_OPTIONAL(root, u_block, Object);
        DECLARE_OPTIONAL(root, v_block, Object);
        DECLARE_OPTIONAL(root, a_block, Object);

        // We accept either r or y
        DECLARE_OPTIONAL(root, r_block, Object);
        DECLARE_OPTIONAL(root, g_block, Object);
        DECLARE_OPTIONAL(root, b_block, Object);

        DECLARE_REQUIRED(root, planes, Array);

        if (r_block == nullptr
            && g_block == nullptr
            && b_block == nullptr
            && y_block == nullptr
            && u_block == nullptr
            && v_block == nullptr
            && a_block == nullptr
        ) {
            throw parse_error("All formats require at least one channel block. That is, one or more of: r_block, g_block, b_block, y_block, u_block, v_block or a_block");
        }


        if (r_block != nullptr || g_block != nullptr || b_block != nullptr) {
            rgb_mode = true;
        }

        if (rgb_mode && (y_block != nullptr || u_block != nullptr || v_block != nullptr)) {
            throw parse_error("RGB and YUV channel blocks combined in the same configuration template, this is not supported. Use either r_block, g_block, b_block OR y_block, u_block, v_block");
        }

        // Parse toplevel fields
        parse_minicalc("", "origin", AST::value_type::STRING, origin);

        if (fourcc != nullptr) {
            DECORATE("fourcc", parse_fourcc(fourcc));
        }

        parse_subsampling(subsampling_mode);

        int i = 0;
        for (auto it = planes->Begin(); it != planes->End(); ++it) {
            parse_plane("planes[" + to_string(i) + "]", &(*it));
            i++;
        }

        if (rgb_mode) {
            parse_block("r_block", r_block);
            parse_block("g_block", g_block);
            parse_block("b_block", b_block);
        } else {
            parse_block("y_block", y_block);
            parse_block("u_block", u_block);
            parse_block("v_block", v_block);
        }
        parse_block("a_block", a_block);

        // Now once everything has been loaded and parsed, we need to topologically sort it.
        std::map<std::string, std::set<std::string>> producer_to_consumer, consumer_to_producer;
        std::vector<std::string> no_remaining_dependencies;

        // Fill the prepare the datastructures.
        for (const auto & elem: this->fields) {
            auto consumer = elem.first;
            uint32_t dependencies = 0;
            for (const auto & dependency_elem: elem.second->get_dependencies()) {
                auto producer = dependency_elem.first;

                // If dependent on something that is not a special variables or resolved constant, add the dependency.
                if (special_variable.find(producer) == special_variable.end() && constants.find(producer) == constants.end()) {
                    producer_to_consumer[producer].insert(consumer);
                    consumer_to_producer[consumer].insert(producer);
                    dependencies++;
                }

            }
            if (dependencies == 0) {
                no_remaining_dependencies.push_back(consumer);
            }
        }

        // Do extra validation.
        this->validate_dependencies(producer_to_consumer, consumer_to_producer);

        // Khan's Algorithm main loop.
        while (!no_remaining_dependencies.empty()) {
            std::string producer = no_remaining_dependencies.back();
            no_remaining_dependencies.pop_back();
            if (auto_generate_samples.find(producer) == auto_generate_samples.end()) {
                resolve_order.push_back(producer);
            }

            for (auto & consumer: producer_to_consumer[producer]) {
                consumer_to_producer[consumer].erase(producer);
                if (consumer_to_producer[consumer].empty()) {
                    no_remaining_dependencies.push_back(consumer);
                }
            }
        }

        // Check for cycles.
        for (auto & elem: consumer_to_producer) {
            if (!elem.second.empty()) {
                // The current consumer is also a producer and is part of a cycle.
                throw parse_error("The format template cannot contain a cycle, break the cycle: "
                                  + find_cycle(producer_to_consumer, elem.first));
            }
        }
    }

    void format_template_impl::parse_fourcc(rapidjson::Value *fourcc) {
        if (fourcc->GetStringLength() > 4) {
            throw parse_error("Length of field 'fourcc' must be <= 4 characters.");
        }

        this->set_constant("", "fourcc", AST::value::string(std::string(fourcc->GetString(), fourcc->GetStringLength())));
    }

    void format_template_impl::parse_subsampling(rapidjson::Value *subsampling_root) {
        DECLARE_REQUIRED_MINICALC(*subsampling_root, macro_px_w);
        DECLARE_REQUIRED_MINICALC(*subsampling_root, macro_px_h);
        std::string parent = "subsampling_mode";

        this->parse_minicalc(parent, "macro_px_w", AST::value_type::INT, macro_px_w);
        this->parse_minicalc(parent, "macro_px_h", AST::value_type::INT, macro_px_h);

        this->validate_int_constant_range(parent, "macro_px_w", 0, 255);
        this->validate_int_constant_range(parent, "macro_px_h", 0, 255);
    }

    void format_template_impl::parse_plane(const std::string& root_name, rapidjson::Value *plane_root) {
        DECLARE_REQUIRED_MINICALC(*plane_root, base_offset);
        DECLARE_REQUIRED_MINICALC(*plane_root, line_stride);
        DECLARE_REQUIRED_MINICALC(*plane_root, plane_size);
        DECLARE_REQUIRED_MINICALC(*plane_root, block_stride);

        DECLARE_OPTIONAL_MINICALC(*plane_root, interleave_pattern);
        DECLARE_OPTIONAL(*plane_root, block_order, Object);

        parse_minicalc(root_name, "base_offset", AST::value_type::INT, base_offset);
        parse_minicalc(root_name, "line_stride", AST::value_type::INT, line_stride);
        parse_minicalc(root_name, "plane_size", AST::value_type::INT, plane_size);
        parse_minicalc(root_name, "block_stride", AST::value_type::INT, block_stride);

        if (interleave_pattern) {
            parse_minicalc(root_name, "interleave_pattern", AST::value_type::STRING, interleave_pattern);
        }
        else {
            this->set_constant(root_name, "interleave_pattern", AST::value::string("NO_INTERLEAVING"));
        }

        if (block_order != nullptr) {
            parse_block_order(root_name, block_order);
        }
        {
            set_constant(root_name + ".block_order", "mega_block_width", AST::value::integer(1));
            set_constant(root_name + ".block_order", "mega_block_height", AST::value::integer(1));
        }

        n_planes++;
    }

    uint32_t format_template_impl::parse_block_order_swizzle(const std::string & parent, const std::string & name, rapidjson::Value *array) {

        uint32_t i = 0;
        for (auto it = array->Begin(); it != array->End(); ++it, ++i) {
            std::string index_name = name + '[' + to_string(i) + ']';

            if (it->IsString() && std::string{ it->GetString(), it->GetStringLength() } == "-") {
                this->set_constant(parent, index_name, AST::value::string("NOT_USED"));
            }
            else {
                parse_minicalc(parent, index_name, AST::value_type::INT, &(*it));
                if (!it->IsUint()) {
                    throw parse_error("Block order bit swizzle not recognized. Expected integer or '-'.");
                }
            }
        }

        if (i >= 32) {
            throw parse_error("Block order bit swizzles are only supported up to 32 bit swizzle patterns. The given pattern is too big.");
        }

        return i;
    }

    void format_template_impl::parse_block_order(const std::string& parent, rapidjson::Value *block_order_root) {
        std::string full_name = parent + ".block_order";

        DECLARE_REQUIRED(*block_order_root, mega_block_width, Uint);
        DECLARE_REQUIRED(*block_order_root, mega_block_height, Uint);
        DECLARE_REQUIRED(*block_order_root, x_mask, Array);
        DECLARE_REQUIRED(*block_order_root, y_mask, Array);

        parse_minicalc(full_name, "mega_block_width", AST::value_type::INT, mega_block_width);
        parse_minicalc(full_name, "mega_block_height", AST::value_type::INT, mega_block_height);

        uint32_t xmask_count = parse_block_order_swizzle(full_name, "x_mask", x_mask);
        uint32_t ymask_count = parse_block_order_swizzle(full_name, "y_mask", x_mask);

        if (xmask_count != ymask_count) {
            throw parse_error("The length of " + parent + ".x_mask must be to equal that of " + parent + ".ymask." );
        }
    }

    void format_template_impl::parse_sample(const std::string & root_name, rapidjson::Value *sample_root) {
        DECLARE_REQUIRED_MINICALC(*sample_root, plane);
        DECLARE_REQUIRED_MINICALC(*sample_root, int_bits);
        DECLARE_REQUIRED_MINICALC(*sample_root, frac_bits);
        DECLARE_REQUIRED_MINICALC(*sample_root, offset);
        DECLARE_OPTIONAL_MINICALC(*sample_root, has_continuation);

        parse_minicalc(root_name, "plane", AST::value_type::INT, plane);
        parse_minicalc(root_name, "int_bits", AST::value_type::INT, int_bits);
        parse_minicalc(root_name, "frac_bits", AST::value_type::INT, frac_bits);
        parse_minicalc(root_name, "offset", AST::value_type::INT, offset);

        if (has_continuation != nullptr) {
            parse_minicalc(root_name, "has_continuation", AST::value_type::INT, has_continuation);
        } else {
            set_constant(root_name, "has_continuation", AST::value::boolean(false));
        }

        validate_int_constant_range(root_name, "plane", 0, 255);
        validate_int_constant_range(root_name, "int_bits", 0, 255);
        validate_int_constant_range(root_name, "frac_bits", 0, 255);
        validate_int_constant_range(root_name, "offset", 0, std::numeric_limits<uint16_t>::max());

    }

    void format_template_impl::parse_block(const std::string &root_name, rapidjson::Value *channel_root) {
        if (channel_root == nullptr) {
            this->set_constant(root_name, "block_w", AST::value::integer(0));
            this->set_constant(root_name, "block_h", AST::value::integer(0));
            return;
        };

        DECLARE_REQUIRED_MINICALC(*channel_root, block_w);
        DECLARE_REQUIRED_MINICALC(*channel_root, block_h);
        DECLARE_OPTIONAL(*channel_root, generate, Bool);
        DECLARE_REQUIRED(*channel_root, samples, Array);

        parse_minicalc(root_name, "block_w", AST::value_type::INT, block_w);
        parse_minicalc(root_name, "block_h", AST::value_type::INT, block_h);

        this->validate_int_constant_range(root_name, "block_w", 0, std::numeric_limits<uint16_t>::max());
        this->validate_int_constant_range(root_name, "block_h", 0, std::numeric_limits<uint16_t>::max());

        bool should_generate = (generate != nullptr && generate->GetBool());
        this->set_constant(root_name, "generate", AST::value::boolean(should_generate));

        auto it = samples->Begin();
        auto end = samples->End();
        int i = 0;
        for (; it != end; ++it, i++) {
            std::string sample_name = root_name + ".samples[" + to_string(i) + "]";
            parse_sample(sample_name, &(*it));

            if (should_generate) {
                for (auto &field : sample_fields) {
                    this->auto_generate_samples.insert(sample_name + '.' + field);
                }
            }
        }
        set_constant(root_name, "samples._size", AST::value::integer(i));
    }

    xyuv::format format_template_impl::inflate(uint32_t image_w, uint32_t image_h, const xyuv::chroma_siting & chroma_siting, const xyuv::conversion_matrix & matrix ) {

        // First, inject image dimensions




        this->constants["image_w"] = AST::value::integer(image_w);
        this->constants["image_h"] = AST::value::integer(image_h);

        // Next resolve all fields (except any generated samples).
        for (auto & field: this->resolve_order) {
            auto it = this->fields.find(field);
            this->constants[field] = it->second->evaluate(&this->constants);
        }

        // Do the evaluation.
        xyuv::format fmt;
        fmt.image_w = image_w;
        fmt.image_h = image_h;
        fmt.size = 0; // This is updated from inflate plane.

        fmt.chroma_siting = chroma_siting;
        fmt.conversion_matrix = matrix;


        fmt.origin = str2enum(get_constant("","origin").check_sval(), str2origin);
        fmt.fourcc = get_constant("", "fourcc").check_sval();

        for (uint32_t i = 0; i < n_planes; i++) {
            inflate_plane(fmt, i);
        }

        if (this->rgb_mode) {
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::Y], "r_block");
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::U], "g_block");
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::V], "b_block");
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::A], "a_block");
        }
        else {
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::Y], "y_block");
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::U], "u_block");
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::V], "v_block");
            inflate_channel_block(fmt, fmt.channel_blocks[xyuv::channel::A], "a_block");
        }

        // Determine the size of the image:


        // Clean up.
        this->constants.erase("image_w");
        this->constants.erase("image_h");

        return fmt;
    }

    uint32_t check_uint_range(AST::value::ival_t min, AST::value::ival_t max, const AST::value &val ) {
        AST::value::ival_t ival = val.check_ival();
        XYUV_ASSERT(min >= 0 && "Programming error.");
        if (ival < min || ival > max) {
            throw value_range_error(min, max, ival);
        }
        return static_cast<uint32_t>(ival);
    }

    uint8_t check_swizzle_value(AST::value & value) {
        if (value.type == AST::value_type::STRING) {
            if (value.sval == "NOT_USED") {
                return block_order::NOT_USED;
            }
            else {
                throw parse_error("Unrecognized block order swizzle '" + value.sval
                                  + "'. Expected integers in [0,31], '-' or 'NOT_USED'");
            }
        } else {
            return static_cast<uint8_t>(check_uint_range(0, 31, value));
        }
    }

    void format_template_impl::inflate_block_order(xyuv::block_order & bo, const std::string & parent) {
        std::string base = parent + ".block_order";

        bo.mega_block_width = check_uint_range(0, std::numeric_limits<AST::value::ival_t>::max(), get_constant(parent, "mega_block_width"));
        bo.mega_block_height = check_uint_range(0, std::numeric_limits<AST::value::ival_t>::max(), get_constant(parent, "mega_block_height"));

        std::vector<uint8_t> ymask, xmask;


        uint32_t i = 0;
        do {
            std::string xmask_name = ".x_mask[" + to_string(i) + ']';
            std::string ymask_name = ".y_mask[" + to_string(i) + ']';
            auto xmask_it = constants.find(parent + xmask_name);
            auto ymask_it = constants.find(parent + ymask_name);
            bool xmask_found = xmask_it != constants.end();
            bool ymask_found = ymask_it != constants.end();

            XYUV_ASSERT((xmask_found && ymask_found) || (!xmask_found && !ymask_found));

            if (xmask_found && ymask_found) {
                xmask.push_back(check_swizzle_value(xmask_it->second));
                ymask.push_back(check_swizzle_value(ymask_it->second));
            }
            else break;

            i++;
        } while (true);

        XYUV_ASSERT(ymask.size() == xmask.size() && ymask.size() <= 32);

        i = 0;
        for (auto it = xmask.rbegin(); it != xmask.rend(); ++it, ++i) {
            bo.x_mask[i] = *it;
        }
        for (; i < 32; i++) {
            bo.x_mask[i] = block_order::NOT_USED;
        }
        i = 0;
        for (auto it = ymask.rbegin(); it != ymask.rend(); ++it, ++i) {
            bo.y_mask[i] = *it;
        }
        for (; i < 32; i++) {
            bo.y_mask[i] = block_order::NOT_USED;
        }
    }

    void format_template_impl::inflate_channel_block(const xyuv::format & fmt, xyuv::channel_block &target_channel_block, const std::string &name) {
        target_channel_block.w = static_cast<uint16_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), get_constant(name, "block_w")));
        target_channel_block.h = static_cast<uint16_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), get_constant(name, "block_h")));

        bool generate = get_constant(name, "generate").check_bval();
        inflate_samples(fmt, target_channel_block, name, generate);
    }

    AST::value format_template_impl::evaluate_generated_sample_field(const std::string &sample_base, const std::string & field_name) {
        std::string full_name = sample_base + "." + field_name;
        auto it = constants.find(full_name);
        if (it != constants.end()) {
            return it->second;
        }
        else {
            auto expr = fields.find(full_name);
            XYUV_ASSERT(expr != fields.end() && "Programmer error");
            return expr->second->evaluate(&this->constants);
        }
    }

    void format_template_impl::inflate_samples(const xyuv::format & fmt, xyuv::channel_block &target_channel_block, const std::string &name, bool generate) {
        std::string base_name = name + ".samples";
        uint32_t n_samples = static_cast<uint32_t>(get_constant(base_name, "_size").ival);

        if (generate) {
            for (uint16_t block_y = 0; block_y < target_channel_block.h; block_y++) {
                set_constant("", "block_y", AST::value::integer(block_y));
                for (uint16_t block_x = 0; block_x < target_channel_block.w; block_x++) {
                    set_constant("", "block_x", AST::value::integer(block_x));

                    for (uint32_t i = 0; i < n_samples; i++) {
                        xyuv::sample sample;
                        std::string index = '[' + to_string(i) + ']';
                        sample.plane = static_cast<uint8_t>(check_uint_range(0, static_cast<AST::value::ival_t>(fmt.planes.size()), evaluate_generated_sample_field(base_name + index, "plane")));
                        sample.offset = static_cast<uint16_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), evaluate_generated_sample_field(base_name + index, "offset")));
                        sample.integer_bits = static_cast<uint8_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), evaluate_generated_sample_field(base_name + index, "integer_bits")));
                        sample.fractional_bits = static_cast<uint8_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), evaluate_generated_sample_field(base_name + index, "fractional_bits")));
                        sample.has_continuation = evaluate_generated_sample_field(base_name + index, "has_continuation").check_bval();

                        if (sample.integer_bits > 0 || sample.fractional_bits > 0) {
                            target_channel_block.samples.push_back(sample);
                        }
                    }
                }
            }
        }
        else {
            for (uint32_t i = 0; i < n_samples; i++) {
                xyuv::sample sample;
                std::string index = '[' + to_string(i) + ']';
                sample.plane = static_cast<uint8_t>(check_uint_range(0, static_cast<AST::value::ival_t>(fmt.planes.size()), get_constant(base_name + index, "plane")));
                sample.offset = static_cast<uint16_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), get_constant(base_name + index, "offset")));
                sample.integer_bits = static_cast<uint8_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), get_constant(base_name + index, "int_bits")));
                sample.fractional_bits = static_cast<uint8_t>(check_uint_range(0, std::numeric_limits<uint16_t>::max(), get_constant(base_name + index, "frac_bits")));
                sample.has_continuation = get_constant(base_name + index, "has_continuation").check_bval();

                if (sample.integer_bits > 0 || sample.fractional_bits > 0) {
                    target_channel_block.samples.push_back(sample);
                }
            }
        }
    }

    void format_template_impl::inflate_plane(xyuv::format & fmt, uint32_t i) {
        xyuv::plane plane;

        std::string parent = "planes[" + to_string(i) + "]";

        plane.base_offset = check_uint_range(0, std::numeric_limits<AST::value::ival_t>::max(), get_constant(parent, "base_offset"));
        plane.line_stride = check_uint_range(0, std::numeric_limits<AST::value::ival_t>::max(), get_constant(parent, "line_stride"));
        plane.size = check_uint_range(0, std::numeric_limits<AST::value::ival_t>::max(), get_constant(parent, "plane_size"));
        plane.block_stride = check_uint_range(0, std::numeric_limits<AST::value::ival_t>::max(), get_constant(parent, "block_stride"));

        plane.interleave_mode = str2enum(get_constant(parent, "interleave_pattern").check_sval(), str2interleave_pattern);

        inflate_block_order(plane.block_order, parent + ".block_order");

        if (plane.base_offset + plane.size > fmt.size) {
            fmt.size = plane.base_offset + plane.size;
        }

        fmt.planes.push_back(plane);
    }

    // Do a pImpl pattern to avoid exposing MiniCalc and rapid-json interfaces.
    format_template::format_template() : impl(new format_template_impl()) {}
    format_template::~format_template() = default;

    void format_template::load_file(const std::string &filename) {
        auto src = read_json(filename);
        this->impl->parse(src);
    }

    xyuv::format format_template::inflate(uint32_t image_w, uint32_t image_h, const xyuv::chroma_siting & chroma_siting, const xyuv::conversion_matrix & matrix ) {
        return this->impl->inflate(image_w, image_h, chroma_siting, matrix);
    }

    } // namespace xyuv

