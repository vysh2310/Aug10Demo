//media integration
#include <iostream>
using namespace std;

#define main media_core_standalone_main
#include "car_media_polyspace_demo.cpp"
#undef main

class DashboardController {
public:
    void run(MediaSystem& system) {
        cout << "=== Dashboard App Starting ===\n";
        system.togglePower();
        system.play();

        cout << "\n[Dashboard] User presses NEXT repeatedly.\n";
        cout << "[Dashboard] UI assumes playlist wraps around safely after the last item.\n\n";

        // Intentional integration issue:
        // The dashboard assumes wrap-around behaviour exists in the core.
        // The core does not enforce that contract.
        for (int pressCount = 1; pressCount <= 5; ++pressCount) {
            cout << "[Dashboard] NEXT press #" << pressCount << "\n";
            system.nextSong();
        }
    }
};

int main() { /* polyspace CERT-CPP:ERR50-CPP [Justified:Low] "Integration demo exits after a fixed validation scenario" polyspace CERT-CPP:ERR51-CPP [Justified:Low] "Integration demo relies on top-level runtime exception reporting" */
    MediaSystem system;
    DashboardController dashboard;
    dashboard.run(system);
    return 0;
}
