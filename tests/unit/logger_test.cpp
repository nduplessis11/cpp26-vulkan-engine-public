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
#include <type_traits>

using engine::core::CaptureSink;
using engine::core::Category;
using engine::core::Logger;
using engine::core::LogRecord;
using engine::core::Severity;

constexpr Category kSandbox{"sandbox"};

static_assert(!std::is_default_constructible_v<Logger>,
              "A default-constructible Logger invites a global instance. "
              "Construct each one with the sink it writes to.");

// --- severity_string --------------------------------------------------------

// One row per enumerator. format_record only ever renders one severity, so
// without this a typo in any other branch ships green.
TEST(SeverityString, SpellsEveryEnumeratorInLowercase) {
    EXPECT_EQ(engine::core::severity_string(Severity::Trace), "trace");
    EXPECT_EQ(engine::core::severity_string(Severity::Debug), "debug");
    EXPECT_EQ(engine::core::severity_string(Severity::Info), "info");
    EXPECT_EQ(engine::core::severity_string(Severity::Warn), "warn");
    EXPECT_EQ(engine::core::severity_string(Severity::Error), "error");
}

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

// Two loggers with different thresholds, both alive at once. This is the
// no-singleton proof. A shared sink shows up as a record in the wrong buffer,
// and a shared threshold shows up as a record filtered by the other logger's
// setting rather than its own.
TEST(Logger, TwoLoggersDoNotObserveEachOther) {
    CaptureSink info_sink;
    Logger info_logger{info_sink, Severity::Info};
    CaptureSink warn_sink;
    Logger warn_logger{warn_sink, Severity::Warn};

    // Info clears info_logger's threshold but not warn_logger's. A single
    // shared threshold set to Warn would drop this line.
    info_logger.log(Severity::Info, kSandbox, "info only");
    // Below warn_logger's own threshold. A shared threshold set to Info would
    // let this line through into warn_sink.
    warn_logger.log(Severity::Info, kSandbox, "below warn threshold");
    warn_logger.log(Severity::Error, kSandbox, "warn only");

    ASSERT_EQ(info_sink.lines().size(), 1U);
    ASSERT_EQ(warn_sink.lines().size(), 1U);

    EXPECT_TRUE(info_sink.lines()[0].starts_with("[info] sandbox: info only (logger_test.cpp:"));
    EXPECT_TRUE(warn_sink.lines()[0].starts_with("[error] sandbox: warn only (logger_test.cpp:"));
}
