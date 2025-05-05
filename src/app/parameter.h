#pragma once

#include <lib/base/parameter.h>

#include "constants.h"

class TargetPositionParameter final : public Parameter<float> {
public:
    TargetPositionParameter(float *value): Parameter(value) {} // NOLINT(*-explicit-constructor)

    bool parse(const String &data) override {
        if constexpr (MQTT_INVERT_POSITION) {
            if (data.length() == 0) return false;

            Type value = data.toFloat();
            value = 100.f - std::max(0.f, std::min(100.f, value));

            return set_value(&value, sizeof(value));
        }

        return Parameter::parse(data);
    }

    [[nodiscard]] String to_string() const override {
        if constexpr (MQTT_INVERT_POSITION) {
            Type value;
            memcpy(&value, Parameter::get_value(), sizeof(value));

            return String(100 - value);
        }

        return Parameter::to_string();
    }
};
