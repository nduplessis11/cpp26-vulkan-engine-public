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
using engine::core::Logger;
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

// --- Logger ----------------------------------------------------------------
TEST(Logger, WritesAFormattedRecordToItsSink) {
    CaptureSink sink;
    Logger logger{sink, Severity::Info};
    auto const loc{std::source_location::current()};

    logger.log(Severity::Info, kSandbox, "window created", loc);

    ASSERT_EQ(sink.lines().size(), 1U);

    auto const expected{
        std::format("[info] sandbox: window created (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(sink.lines()[0], expected);
}

// The location argument is omitted on purpose: the defaulted
// std::source_location::current() is the thing under test. Passing loc here
// would make the case pass even if log() captured the location in its own
// body, which would report logger.hpp for every message in the engine.
TEST(Logger, DefaultsLocationToTheCallSite) {
    CaptureSink sink;
    Logger logger{sink, Severity::Info};

    logger.log(Severity::Info, kSandbox, "window created");

    ASSERT_EQ(sink.lines().size(), 1U);
    EXPECT_TRUE(sink.lines()[0].starts_with("[info] sandbox: window created (logger_test.cpp:"));
}

TEST(Logger, DropsMessagesBelowItsThreshold) {
    CaptureSink sink;
    Logger logger{sink, Severity::Warn};

    logger.log(Severity::Info, kSandbox, "window created");

    EXPECT_TRUE(sink.lines().empty());
}

TEST(Logger, PassesMessagesAboveItsThreshold) {
    CaptureSink sink;
    Logger logger{sink, Severity::Warn};

    logger.log(Severity::Error, kSandbox, "device lost");

    EXPECT_EQ(sink.lines().size(), 1U);
}

// Remaining cases for the Logger:
//
// - Two independent loggers. Two CaptureSinks and two Loggers with different
//   thresholds in one test body. A record sent through A lands in A's buffer
//   and nowhere else, and B's threshold does not change what A emits. Any
//   design with a hidden shared instance fails this, which is what makes it
//   the no-singleton proof.
// - static_assert(!std::is_default_constructible_v<Logger>) at namespace
//   scope, so every call site must name a logger it was handed. Needs
//   <type_traits>.
