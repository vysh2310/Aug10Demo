#include <iostream>
#include <cstring>
#include <limits>
using namespace std;
// TODO: Improve input validation for safety and security

class MediaSystem {
private:
    bool powerOn;
    int currentSongIndex;
    char songs[4][10];

public:
    MediaSystem() {
        powerOn = false;
        currentSongIndex = 0;


        strcpy(songs[0], "SongA");
        strcpy(songs[1], "SongB");
        strcpy(songs[2], "SongC");
        strcpy(songs[3], "SongD");
    }

    void togglePower() {
        powerOn = !powerOn;
        cout << (powerOn ? "System ON\n" : "System OFF\n");
    }

    void play() {
        if (!powerOn) {
            cout << "System OFF\n";
            return;
        }
        cout << "Playing: " << songs[currentSongIndex] << endl;
    }

    void nextSong() {
        if (!powerOn) return;

        currentSongIndex = (currentSongIndex + 1) % 4;
        play();
    }

    void renameSong() {
        if (!powerOn) {
            cout << "System OFF\n";
            return;
        }

        char newTitle[10];
        cout << "Enter new title: ";
        cin >> newTitle; 
       
        strcpy(songs[currentSongIndex], newTitle);
        cout << "Renamed current song to " << songs[currentSongIndex] << endl;
    }
};

int main() {
    MediaSystem system;
    int choice = 0;

    while (true) {
        cout << "\n1.Power 2.Play 3.Next 4.Rename 5.Exit\n";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid\n";
            continue;
        }

        switch (choice) {
            case 1: system.togglePower(); break;
            case 2: system.play(); break;
            case 3: system.nextSong(); break;
            case 4: system.renameSong(); break;
            case 5: return 0;
            default: cout << "Invalid\n";
        }
    }
}

