#pragma once

#include <cstdint>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

namespace engine::core {

enum class Severity : std::uint8_t { Trace, Debug, Info, Warn, Error };
auto inline severity_string(Severity severity) -> std::string {
    switch (severity) {
    case Severity::Trace:
        return "trace";
    case Severity::Debug:
        return "debug";
    case Severity::Info:
        return "info";
    case Severity::Warn:
        return "warn";
    case Severity::Error:
        return "error";
    }
    std::unreachable();
}

struct Category {
    std::string_view name;
};

struct LogRecord {
    Severity severity;
    Category category;
    std::string_view message;
    std::source_location location;
};

auto inline format_record(LogRecord const& record) -> std::string {
    std::string_view const path{record.location.file_name()};
    auto const slash = path.find_last_of('/');
    auto const file = slash == std::string_view::npos ? path : path.substr(slash + 1);

    return std::format("[{}] {}: {} ({}:{})", severity_string(record.severity),
                       record.category.name, record.message, file, record.location.line());
}

} // namespace engine::core
