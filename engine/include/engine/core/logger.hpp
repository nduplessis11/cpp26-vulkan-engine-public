#pragma once

#include <cstdint>
#include <cstdio> // IWYU pragma: keep
#include <format>
#include <print>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine::core {

enum class Severity : std::uint8_t { Trace, Debug, Info, Warn, Error };
auto inline severity_string(Severity const severity) -> std::string {
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
    auto const file =
        slash == std::string_view::npos ? path : path.substr(slash + 1);

    return std::format("[{}] {}: {} ({}:{})", severity_string(record.severity),
                       record.category.name, record.message, file,
                       record.location.line());
}

struct ISink {
    virtual ~ISink() = default;
    virtual auto write(LogRecord const&) -> void = 0;

  protected:
    ISink() = default;
    ISink(ISink const&) = default;
    ISink(ISink&&) = default;
    auto operator=(ISink const&) -> ISink& = default;
    auto operator=(ISink&&) -> ISink& = default;
};

class StderrSink : public ISink {
  public:
    auto write(LogRecord const& record) -> void override {
        auto const line = format_record(record);
        try {
            std::println(stderr, "{}", line);
        } catch (std::system_error const&) { // NOLINT(bugprone-empty-catch)
            // Ignore write failures; reporting them would use the same broken
            // output.
        }
    }
};

class CaptureSink : public ISink {
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

template <class... Args> struct FmtWithLoc {
    std::format_string<Args...> fmt;
    std::source_location loc;

    // The implicit conversion is the point: it turns a literal at the call site
    // into the wrapper, which is how the location gets captured.
    // NOLINTNEXTLINE(google-explicit-constructor)
    consteval FmtWithLoc(char const* text, std::source_location const where =
                                               std::source_location::current())
        : fmt{text}, loc{where} {}
};

class Logger {
  public:
    explicit Logger(ISink& sink, Severity const threshold)
        : sink_{sink}, threshold_{threshold} {}

    auto log(Severity const severity, Category const category,
             std::string_view const message,
             std::source_location const location =
                 std::source_location::current()) const -> void {
        if (severity >= threshold_) {
            sink_.write(LogRecord{.severity = severity,
                                  .category = category,
                                  .message = message,
                                  .location = location});
        }
    }

    template <class... Args>
    auto log(Severity const severity, Category const category,
             FmtWithLoc<std::type_identity_t<Args>...> fmt,
             Args&&... args) const -> void {
        vlog(severity, category, fmt.fmt.get(), std::make_format_args(args...),
             fmt.loc);
    }

    auto vlog(Severity const severity, Category const category,
              std::string_view const format, std::format_args const args,
              std::source_location const loc =
                  std::source_location::current()) const -> void {
        if (severity >= threshold_) {
            auto const formatted = std::vformat(format, args);
            sink_.write(LogRecord{.severity = severity,
                                  .category = category,
                                  .message = formatted,
                                  .location = loc});
        }
    }

  private:
    ISink& sink_;
    Severity threshold_;
};

} // namespace engine::core
