// Logging tests use separate sinks and loggers to check their independence.
#include <engine/core/logger.hpp>
#include <format>
#include <gtest/gtest.h>
#include <source_location>
#include <string>
#include <type_traits>

using engine::core::CaptureSink;
using engine::core::Category;
using engine::core::ISink;
using engine::core::Logger;
using engine::core::LogRecord;
using engine::core::Severity;

constexpr Category kSandbox{"sandbox"};

static_assert(!std::is_default_constructible_v<Logger>,
              "A default-constructible Logger invites a global instance. "
              "Construct each one with the sink it writes to.");

// `sink_a = sink_b` through two ISink& calls ISink::operator=, which has no
// members to copy, so sink_a keeps its old lines.
static_assert(!std::is_copy_assignable_v<ISink> &&
                  !std::is_move_assignable_v<ISink>,
              "Assigning through an ISink& copies nothing. Keep ISink's "
              "assignment operators protected.");
static_assert(std::is_copy_assignable_v<CaptureSink>,
              "Protected ISink assignment must still let a derived sink "
              "copy itself.");

// Severity names.

// Check every severity; the formatting test below only uses Info.
TEST(SeverityString, SpellsEveryEnumeratorInLowercase) {
    EXPECT_EQ(engine::core::severity_string(Severity::Trace), "trace");
    EXPECT_EQ(engine::core::severity_string(Severity::Debug), "debug");
    EXPECT_EQ(engine::core::severity_string(Severity::Info), "info");
    EXPECT_EQ(engine::core::severity_string(Severity::Warn), "warn");
    EXPECT_EQ(engine::core::severity_string(Severity::Error), "error");
}

// Record formatting.

// Formatting depends only on the record; no logger or sink is needed.
TEST(FormatRecord, RendersSeverityCategoryMessageAndLocation) {
    auto const loc{std::source_location::current()};
    LogRecord record{Severity::Info, kSandbox, "window created", loc};

    auto const output{engine::core::format_record(record)};

    auto const expected{std::format(
        "[info] sandbox: window created (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(output, expected);
}

// CaptureSink.

TEST(CaptureSink, StoresTheFormattedLine) {
    CaptureSink sink;
    auto const loc{std::source_location::current()};

    sink.write(LogRecord{Severity::Info, kSandbox, "window created", loc});

    ASSERT_EQ(sink.lines().size(), 1U);

    auto const expected{std::format(
        "[info] sandbox: window created (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(sink.lines()[0], expected);
}

TEST(CaptureSink, CopiesTheMessageOutOfATemporary) {
    CaptureSink sink;
    auto const loc{std::source_location::current()};

    sink.write(
        LogRecord{Severity::Info, kSandbox, std::format("frame {}", 7), loc});

    ASSERT_EQ(sink.lines().size(), 1U);

    auto const expected{std::format(
        "[info] sandbox: frame 7 (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(sink.lines()[0], expected);
}

// Logger.
TEST(Logger, WritesAFormattedRecordToItsSink) {
    CaptureSink sink;
    Logger logger{sink, Severity::Info};
    auto const loc{std::source_location::current()};

    logger.log(Severity::Info, kSandbox, "window created", loc);

    ASSERT_EQ(sink.lines().size(), 1U);

    auto const expected{std::format(
        "[info] sandbox: window created (logger_test.cpp:{})", loc.line())};
    EXPECT_EQ(sink.lines()[0], expected);
}

// Omit the location to check that the default reports this file's call site.
// Capturing it inside log() would report logger.hpp instead.
TEST(Logger, DefaultsLocationToTheCallSite) {
    CaptureSink sink;
    Logger logger{sink, Severity::Info};

    logger.log(Severity::Info, kSandbox, "window created");

    ASSERT_EQ(sink.lines().size(), 1U);
    EXPECT_TRUE(sink.lines()[0].starts_with(
        "[info] sandbox: window created (logger_test.cpp:"));
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

// Each logger must use its own sink and threshold.
// Shared state would put a message in the wrong buffer or filter it
// incorrectly.
TEST(Logger, TwoLoggersDoNotObserveEachOther) {
    CaptureSink info_sink;
    Logger info_logger{info_sink, Severity::Info};
    CaptureSink warn_sink;
    Logger warn_logger{warn_sink, Severity::Warn};

    // A shared Warn threshold would wrongly drop this message.
    info_logger.log(Severity::Info, kSandbox, "info only");
    // A shared Info threshold would wrongly keep this message.
    warn_logger.log(Severity::Info, kSandbox, "below warn threshold");
    warn_logger.log(Severity::Error, kSandbox, "warn only");

    ASSERT_EQ(info_sink.lines().size(), 1U);
    ASSERT_EQ(warn_sink.lines().size(), 1U);

    EXPECT_TRUE(info_sink.lines()[0].starts_with(
        "[info] sandbox: info only (logger_test.cpp:"));
    EXPECT_TRUE(warn_sink.lines()[0].starts_with(
        "[error] sandbox: warn only (logger_test.cpp:"));
}

TEST(Logger, VlogFormatsArgumentsIntoItsSink) {
    CaptureSink sink;
    Logger logger{sink, Severity::Info};

    int frame = 7;
    logger.vlog(Severity::Info, kSandbox, "frame {}",
                std::make_format_args(frame));

    ASSERT_EQ(sink.lines().size(), 1U);
    EXPECT_TRUE(sink.lines()[0].starts_with(
        "[info] sandbox: frame 7 (logger_test.cpp:"));
}

TEST(Logger, FormatsArgumentsIntoItsSink) {
    CaptureSink sink;
    Logger logger{sink, Severity::Info};

    logger.log(Severity::Info, kSandbox, "frame {} of {}", 7, 60);

    ASSERT_EQ(sink.lines().size(), 1U);
    EXPECT_TRUE(sink.lines()[0].starts_with(
        "[info] sandbox: frame 7 of 60 (logger_test.cpp:"));
}
