/*
 * PSTUnit tests for car_media_polyspace_demo.cpp.
 * Covers power control, playback flow, track navigation, and rename behavior.
 */

#include "pstunit.h"

#include <ios>
#include <sstream>
#include <string>

#define main car_media_polyspace_demo_program_main
#include "car_media_polyspace_demo.cpp"
#undef main

struct MediaSystemConsoleFixture : public pst_suite_fixture {
    std::streambuf* originalCin = nullptr;
    std::streambuf* originalCout = nullptr;
    std::istringstream input;
    std::ostringstream output;

    void test_setup() {
        originalCin = std::cin.rdbuf();
        originalCout = std::cout.rdbuf();
        std::cin.clear();
        std::cout.clear();
        input.clear();
        input.str("");
        output.clear();
        output.str("");
        std::cin.rdbuf(input.rdbuf());
        std::cout.rdbuf(output.rdbuf());
    }

    void test_teardown() {
        std::cin.rdbuf(originalCin);
        std::cout.rdbuf(originalCout);
        std::cin.clear();
        std::cout.clear();
    }

    void setInput(const std::string& value) {
        input.clear();
        input.str(value);
    }

    std::string capturedOutput() const {
        return output.str();
    }
};

PST_SUITE_WITH_FIXTURE(media_system_suite, MediaSystemConsoleFixture);

static MediaSystemConsoleFixture* fixture() {
    return static_cast<MediaSystemConsoleFixture*>(PST_SUITE_TEST_FIXTURE_PTR());
}

/* Test goal: power toggles between ON and OFF states. */
PST_TEST(media_system_suite, toggle_power_reports_state_changes) {
    MediaSystem system;

    // Exercise the power toggle twice.
    system.togglePower();
    system.togglePower();

    // Verify that both state transitions were reported.
    PST_VERIFY_EQ(std::string("System ON\nSystem OFF\n"), fixture()->capturedOutput());
}

/* Test goal: play rejects requests while the system is off. */
PST_TEST(media_system_suite, play_reports_system_off_when_unpowered) {
    MediaSystem system;

    // Attempt playback without enabling the system.
    system.play();

    // Verify the off-state message.
    PST_VERIFY_EQ(std::string("System OFF\n"), fixture()->capturedOutput());
}

/* Test goal: play starts with the first configured song after power on. */
PST_TEST(media_system_suite, play_reports_initial_song_after_power_on) {
    MediaSystem system;

    // Enable the system and clear the toggle output.
    system.togglePower();
    fixture()->output.str("");
    fixture()->output.clear();

    // Start playback.
    system.play();

    // Verify the initial song selection.
    PST_VERIFY_EQ(std::string("Playing: SongA\n"), fixture()->capturedOutput());
}

/* Test goal: nextSong advances playback to the next track. */
PST_TEST(media_system_suite, next_song_advances_to_second_track) {
    MediaSystem system;

    // Enable the system and clear the toggle output.
    system.togglePower();
    fixture()->output.str("");
    fixture()->output.clear();

    // Advance one track.
    system.nextSong();

    // Verify playback moved to SongB.
    PST_VERIFY_EQ(std::string("Playing: SongB\n"), fixture()->capturedOutput());
}

/* Test goal: renameSong exits early while powered off and leaves input available. */
PST_TEST(media_system_suite, rename_song_requires_power_before_consuming_input) {
    MediaSystem system;

    // Queue a title before calling rename while the system is off.
    fixture()->setInput("RoadTrip\n");
    system.renameSong();
    PST_VERIFY_EQ(std::string("System OFF\n"), fixture()->capturedOutput());

    // Power on and rename again using the still-buffered title.
    fixture()->output.str("");
    fixture()->output.clear();
    system.togglePower();
    fixture()->output.str("");
    fixture()->output.clear();
    system.renameSong();

    // Verify the pending title was applied on the powered-on call.
    PST_VERIFY_EQ(std::string("Enter new title: Renamed current song to RoadTrip\n"),
                  fixture()->capturedOutput());
}

/* Test goal: renameSong updates the current song when valid input is provided. */
PST_TEST(media_system_suite, rename_song_updates_current_track_title) {
    MediaSystem system;

    // Enable the system and provide the replacement title.
    system.togglePower();
    fixture()->output.str("");
    fixture()->output.clear();
    fixture()->setInput("CommuteMix\n");

    // Rename the current song and then play it back.
    system.renameSong();
    system.play();

    // Verify both the rename confirmation and subsequent playback.
    PST_VERIFY_EQ(std::string("Enter new title: Renamed current song to CommuteMix\n"
                              "Playing: CommuteMix\n"),
                  fixture()->capturedOutput());
}

/* Test goal: renameSong reports Invalid when the input stream is already failed. */
PST_TEST(media_system_suite, rename_song_handles_stream_failure) {
    MediaSystem system;

    // Enable the system and force a stream failure before reading input.
    system.togglePower();
    fixture()->output.str("");
    fixture()->output.clear();
    std::cin.setstate(std::ios::failbit);

    // Attempt the rename with a failed stream.
    system.renameSong();

    // Verify the error path is reported.
    PST_VERIFY_EQ(std::string("Enter new title: Invalid\n"), fixture()->capturedOutput());
}

int main(int argc, char* argv[]) {
    return PST_MAIN(argc, argv);
}
