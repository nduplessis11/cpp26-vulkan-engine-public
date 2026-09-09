#pragma once

#include <cstdint>
#include <format>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

[[nodiscard]]
auto inline format_record(LogRecord const& record) -> std::string {
    std::string_view const path{record.location.file_name()};
    auto const slash = path.find_last_of('/');
    auto const file = slash == std::string_view::npos ? path : path.substr(slash + 1);

    return std::format("[{}] {}: {} ({}:{})", severity_string(record.severity),
                       record.category.name, record.message, file, record.location.line());
}

struct Sink {
    virtual ~Sink() = default;
    virtual auto write(LogRecord const&) -> void = 0;
};

class CaptureSink : public Sink {
  public:
    auto write(LogRecord const& record) -> void override {
        lines_.push_back(format_record(record));
    }

    [[nodiscard]]
    auto lines() const -> std::span<std::string const> {
        return lines_;
    }

  private:
    std::vector<std::string> lines_;
};

class Logger {
  public:
    explicit Logger(Sink& sink, Severity threshold) : sink_{sink}, threshold_{threshold} {}

    auto log(Severity severity, Category category, std::string_view message,
             std::source_location location = std::source_location::current()) -> void {
        if (severity >= threshold_) {
            sink_.write(LogRecord{severity, category, message, location});
        }
    }

  private:
    Sink& sink_;
    Severity threshold_;
};

} // namespace engine::core
