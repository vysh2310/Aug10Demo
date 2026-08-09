/*
 * PSTUnit coverage for the Aug10Demo media system.
 * This file exercises the core MediaSystem behavior and mirrors the
 * DashboardController integration sequence using observable console I/O.
 */
#include "pstunit.h"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

#define main media_core_standalone_main
#include "C:/Aug10_CFE/Aug10Demo/car_media_polyspace_demo.cpp"
#undef main

namespace {

class ScopedIoRedirect {
public:
    ScopedIoRedirect()
        : originalCin_(std::cin.rdbuf(input_.rdbuf())),
          originalCout_(std::cout.rdbuf(output_.rdbuf())) {}

    ~ScopedIoRedirect() {
        std::cin.rdbuf(originalCin_);
        std::cout.rdbuf(originalCout_);
    }

    void setInput(const std::string& text) {
        input_.str(text);
        input_.clear();
    }

    std::string output() const {
        return output_.str();
    }

private:
    std::istringstream input_;
    std::ostringstream output_;
    std::streambuf* originalCin_;
    std::streambuf* originalCout_;
};

std::size_t countOccurrences(const std::string& haystack, const std::string& needle) {
    std::size_t count = 0;
    std::size_t position = 0;

    while ((position = haystack.find(needle, position)) != std::string::npos) {
        ++count;
        position += needle.size();
    }

    return count;
}

class DashboardControllerHarness {
public:
    void run(MediaSystem& system) {
        cout << "=== Dashboard App Starting ===\n";
        system.togglePower();
        system.play();

        cout << "\n[Dashboard] User presses NEXT repeatedly.\n";
        cout << "[Dashboard] UI assumes playlist wraps around safely after the last item.\n\n";

        for (int pressCount = 1; pressCount <= 5; ++pressCount) {
            cout << "[Dashboard] NEXT press #" << pressCount << "\n";
            system.nextSong();
        }
    }
};

}  // namespace

PST_SUITE(aug10demo_media_system);

/* Test: toggling power once turns the system on. */
PST_TEST(aug10demo_media_system, toggle_power_turns_on) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Toggle power once.
    system.togglePower();

    // Verify the visible state transition.
    PST_VERIFY_EQ(io.output(), std::string("System ON\n"));
}

/* Test: toggling power twice returns the system to the off state. */
PST_TEST(aug10demo_media_system, toggle_power_turns_off_after_second_press) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Toggle the power on and then off again.
    system.togglePower();
    system.togglePower();

    // Verify both transitions are reported.
    PST_VERIFY_EQ(io.output(), std::string("System ON\nSystem OFF\n"));
}

/* Test: play reports that the system is off when power has not been enabled. */
PST_TEST(aug10demo_media_system, play_reports_off_when_unpowered) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Attempt playback while powered off.
    system.play();

    // Verify the rejection message.
    PST_VERIFY_EQ(io.output(), std::string("System OFF\n"));
}

/* Test: play outputs the first song after power is enabled. */
PST_TEST(aug10demo_media_system, play_outputs_first_song_when_powered) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Power on the unit and play the current song.
    system.togglePower();
    system.play();

    // Verify the startup transition and first-song playback.
    PST_VERIFY_EQ(io.output(), std::string("System ON\nPlaying: SongA\n"));
}

/* Test: nextSong does nothing while the system is powered off. */
PST_TEST(aug10demo_media_system, next_song_is_noop_when_unpowered) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Attempt to advance while powered off.
    system.nextSong();

    // Verify that no output is produced.
    PST_VERIFY_EQ(io.output(), std::string(""));
}

/* Test: nextSong advances from SongA to SongB after power-on. */
PST_TEST(aug10demo_media_system, next_song_advances_when_powered) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Power on the system and advance once.
    system.togglePower();
    system.nextSong();

    // Verify the next song becomes active and is played.
    PST_VERIFY_EQ(io.output(), std::string("System ON\nPlaying: SongB\n"));
}

/* Test: nextSong wraps back to SongA after the fourth advance. */
PST_TEST(aug10demo_media_system, next_song_wraps_after_last_entry) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Power on and cycle through the four available songs.
    system.togglePower();
    system.nextSong();
    system.nextSong();
    system.nextSong();
    system.nextSong();

    // Verify the sequence reaches SongD and then wraps to SongA.
    const std::string captured = io.output();
    PST_VERIFY_TRUE(captured.find("Playing: SongD\n") != std::string::npos);
    PST_VERIFY_TRUE(captured.find("Playing: SongA\n") != std::string::npos);
    PST_VERIFY_EQ(countOccurrences(captured, "Playing: "), static_cast<std::size_t>(4));
}

/* Test: renameSong reports that the system is off when unavailable. */
PST_TEST(aug10demo_media_system, rename_song_reports_off_when_unpowered) {
    // Set up the system and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;

    // Attempt to rename while powered off.
    system.renameSong();

    // Verify the operation is rejected.
    PST_VERIFY_EQ(io.output(), std::string("System OFF\n"));
}

/* Test: renameSong updates the current song when valid input is provided. */
PST_TEST(aug10demo_media_system, rename_song_updates_current_track) {
    // Set up the system, provide input, and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;
    io.setInput("JazzMix\n");

    // Power on, rename the current song, and play it back.
    system.togglePower();
    system.renameSong();
    system.play();

    // Verify the rename is acknowledged and the new title is active.
    PST_VERIFY_TRUE(io.output().find("Renamed current song to JazzMix\n") != std::string::npos);
    PST_VERIFY_TRUE(io.output().find("Playing: JazzMix\n") != std::string::npos);
}

PST_SUITE(aug10demo_dashboard);

/* Test: the dashboard integration powers on the system and issues five NEXT presses. */
PST_TEST(aug10demo_dashboard, dashboard_run_executes_expected_sequence) {
    // Set up the system, dashboard, and capture the console output.
    ScopedIoRedirect io;
    MediaSystem system;
    DashboardControllerHarness dashboard;

    // Run the dashboard flow against the media system.
    dashboard.run(system);

    // Verify the startup messages and the complete playback sequence.
    const std::string captured = io.output();
    PST_VERIFY_TRUE(captured.find("=== Dashboard App Starting ===\n") != std::string::npos);
    PST_VERIFY_TRUE(captured.find("[Dashboard] NEXT press #5\n") != std::string::npos);
    PST_VERIFY_EQ(countOccurrences(captured, "[Dashboard] NEXT press #"), static_cast<std::size_t>(5));
    PST_VERIFY_TRUE(captured.find("Playing: SongA\n") != std::string::npos);
    PST_VERIFY_TRUE(captured.rfind("Playing: SongB\n") != std::string::npos);
}

int main(int argc, char* argv[]) {
    return PST_MAIN(argc, argv);
}
