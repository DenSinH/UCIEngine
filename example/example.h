#pragma once


struct Example final : Engine {
    /*
     * This is an example "AI" that simply moves the pawns forward 3 times and then quits.
     * This was mostly meant for testing, but provides a nice example of intended use.
     *
     * A main function would look like
     *
     *      int main() {
     *          Example engine{};
     *          return engine.Run();
     *      }
     * */

    int counter = 0;

    void HandleGo(Search& search) override {
        if (counter < 8) {
            SendMove('a' + counter, '7', 'a' + counter, '6');
        }
        else if (counter < 16) {
            SendMove('a' + counter - 8, '6', 'a' + counter - 8, '5');
        }
        else if (counter < 24) {
            SendMove('a' + counter - 16, '5', 'a' + counter, '4');
        }
        else {
            SendMove("0000");
        }
        counter++;
    }
};