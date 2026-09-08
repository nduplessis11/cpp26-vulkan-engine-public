// Tests for the logging channel.
//
// Each test builds its own sink and logger in the test body. There is no
// fixture-level shared state, because "two loggers never observe each other"
// is one of the things under test.
#include <engine/core/logger.hpp>
#include <format>
#include <gtest/gtest.h>
#include <source_location>
#include <string>

using engine::core::CaptureSink;
using engine::core::Category;
using engine::core::LogRecord;
using engine::core::Severity;

constexpr Category kSandbox{"sandbox"};

// --- format_record ----------------------------------------------------------

// format_record is a pure function of its record, so this case needs no
// logger and no sink.
TEST(FormatRecord, RendersSeverityCategoryMessageAndLocation) {
    auto const loc{std::source_location::current()};
    LogRecord record{Severity::Info, kSandbox, "window created", loc};

    auto const output{engine::core::format_record(record)};

    auto const expected{
        std::format("[info] sandbox: window created (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(output, expected);
}

// --- CaptureSink ------------------------------------------------------------

TEST(CaptureSink, StoresTheFormattedLine) {
    CaptureSink sink;
    auto const loc{std::source_location::current()};

    sink.write(LogRecord{Severity::Info, kSandbox, "window created", loc});

    ASSERT_EQ(sink.lines().size(), 1U);

    auto const expected{
        std::format("[info] sandbox: window created (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(sink.lines()[0], expected);
}

TEST(CaptureSink, CopiesTheMessageOutOfATemporary) {
    CaptureSink sink;
    auto const loc{std::source_location::current()};

    sink.write(LogRecord{Severity::Info, kSandbox, std::format("frame {}", 7), loc});

    ASSERT_EQ(sink.lines().size(), 1U);

    auto const expected{std::format("[info] sandbox: frame 7 (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(sink.lines()[0], expected);
}
