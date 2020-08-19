// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_binary_operator_node.hpp"

namespace tt {

struct formula_logical_or_node final : formula_binary_operator_node {
    formula_logical_or_node(parse_location location, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(std::move(location), std::move(lhs), std::move(rhs)) {}

    datum evaluate(formula_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        if (lhs_) {
            return lhs_;
        } else {
            return rhs->evaluate(context);
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} || {})", *lhs, *rhs);
    }
};

}