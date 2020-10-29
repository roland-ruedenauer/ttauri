// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_radio_button_widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/format10.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T>
class toolbar_tab_button_widget final : public abstract_radio_button_widget<T> {
public:
    using super = abstract_radio_button_widget<T>;
    using value_type = super::value_type;

    observable<std::u8string> label;

    template<typename Value, typename... FormatArgs>
    toolbar_tab_button_widget(
        Window &window,
        std::shared_ptr<Widget> parent,
        value_type true_value,
        Value &&value,
        l10n const &fmt,
        FormatArgs const &... args) noexcept :
        abstract_radio_button_widget<T>(window, parent, std::move(true_value), std::forward<Value>(value)),
        label(format(fmt, args...))
    {
    }

    template<typename Value>
    toolbar_tab_button_widget(Window &window, std::shared_ptr<Widget> parent, value_type true_value, Value &&value) noexcept :
        toolbar_tab_button_widget(window, parent, std::move(true_value), std::forward<Value>(value), l10n{})
    {
    }

    toolbar_tab_button_widget(Window &window, std::shared_ptr<Widget> parent, value_type true_value) noexcept :
        toolbar_tab_button_widget(window, parent, std::move(true_value), observable<int>{}, l10n{})
    {
    }

    ~toolbar_tab_button_widget() {}

    void initialize() noexcept override
    {
        _label_callback = label.subscribe([this](auto...) {
            this->request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints()) {
            _label_cell = std::make_unique<TextCell>(*label, theme->labelStyle);

            ttlet minimum_height = _label_cell->preferredExtent().height();
            ttlet minimum_width = _label_cell->preferredExtent().width() + 2.0f * Theme::margin;

            this->p_preferred_size = {vec{minimum_width, minimum_height}, vec{minimum_width, std::numeric_limits<float>::infinity()}};
            this->p_preferred_base_line = relative_base_line{VerticalAlignment::Middle, -Theme::margin};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->request_relayout, false);
        if (need_layout) {
            ttlet offset = Theme::margin + Theme::borderWidth;
            _button_rectangle = aarect{
                this->rectangle().x(),
                this->rectangle().y() - offset,
                this->rectangle().width(),
                this->rectangle().height() + offset};
        }

        return super::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        draw_button(context);
        draw_label(context);
        draw_focus_line(context);
        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    aarect _button_rectangle;
    std::unique_ptr<TextCell> _label_cell;

    void draw_focus_line(DrawContext const &context) noexcept
    {
        if (this->focus && this->window.active && *this->value == this->true_value) {
            auto parent_ = this->parent.lock();
            tt_assume(std::dynamic_pointer_cast<class ToolbarWidget>(parent_) != nullptr);

            // Draw the focus line over the full width of the window at the bottom
            // of the toolbar.
            auto parent_context = parent_->make_draw_context(context);

            // Draw the line above every other direct child of the toolbar, and between
            // the selected-tab (0.6) and unselected-tabs (0.8).
            parent_context.transform = mat::T(0.0f, 0.0f, 1.7f) * parent_context.transform;

            parent_context.fillColor = theme->accentColor;
            parent_context.drawFilledQuad(
                aarect{parent_->rectangle().x(), parent_->rectangle().y(), parent_->rectangle().width(), 1.0f});
        }
    }

    void draw_button(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        if (this->focus && this->window.active) {
            // The focus line will be placed at 0.7.
            context.transform = mat::T(0.0f, 0.0f, 0.8f) * context.transform;
        } else {
            context.transform = mat::T(0.0f, 0.0f, 0.6f) * context.transform;
        }

        // Override the clipping rectangle to match the toolbar.
        context.clippingRectangle = this->parent.lock()->window_rectangle();

        if (this->hover || *this->value == this->true_value) {
            context.fillColor = theme->fillColor(this->p_semantic_layer - 2);
            context.color = context.fillColor;
        } else {
            context.fillColor = theme->fillColor(this->p_semantic_layer - 1);
            context.color = context.fillColor;
        }

        if (this->focus && this->window.active) {
            context.color = theme->accentColor;
        }

        context.cornerShapes = vec{0.0f, 0.0f, Theme::roundingRadius, Theme::roundingRadius};
        context.drawBoxIncludeBorder(_button_rectangle);
    }

    void draw_label(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.transform = mat::T(0.0f, 0.0f, 0.9f) * context.transform;

        if (*this->enabled) {
            context.color = theme->labelStyle.color;
        }

        _label_cell->draw(context, this->rectangle(), Alignment::MiddleCenter, this->base_line(), true);
    }
};

} // namespace tt