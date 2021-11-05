#pragma once

#include "util/ThreadQueue.h"

#include <string>
#include <vector>
#include <array>
#include <variant>

#ifndef NDEBUG
#include <fstream>
#endif


struct Engine {
    Engine();

    int Run();
    static void SendMove(const std::string& move);
    static void SendMove(char c0, char r0, char c1, char r1);

protected:
    struct Search {
        int duration = -1;
        bool ponder = false;
        int wtime = 0;
        int btime = 0;
        int winc = 0;
        int binc = 0;
        int depth = 0;
        int nodes = 0;
        int mate = 0;
        int movestogo = 0;
        int movetime = 0;
        std::vector<std::string> moves{};
    };

    /*
     * Name and author of AI
     * Response must look like:
     * id name DenSinH\n
     * id author DenSinH\n
     * uciok\n
     * */
    virtual std::string Name() { return "DenSinH"; }
    virtual std::string Author() { return "DenSinH"; }

    /*
     * Enable or disable debug mode
     * GUI sends
     * debug [on|off]
     * Handle to your own liking, GUI expects no output
     * */
    virtual void HandleDebug(bool state) { }

    /*
     * Sets options in AI
     * GUI sends
     * setoption name <name (with spaces)> value <value>
     * Handle to your own liking, GUI expects no output
     * */
    virtual void HandleSetOption(const std::string& name, const std::string& value) { }

    /*
     * Start a new game
     * */
    virtual void HandleNewGame() { }

    /*
     * Ask if AI is ready
     * GUI expects
     * readyok\n
     * Change this if you like, but by default this just outputs "readyok\n"
     * */
    virtual void HandleIsReady();

    /*
     * set up the position described in fenstring on the internal board and
     * play the moves on the internal chess board.
     * if the game was played  from the start position the string "startpos" will be sent
     * */
    virtual void HandlePosFen(const std::vector<std::string>& moves) { }
    virtual void HandlePosStart(const std::vector<std::string>& moves) { }

    /*
     * start calculating on the current position set up with the "position" command.
	 * There are a number of commands that can follow this command, all will be sent in the same string.
	 * If one command is not send its value should be interpreted as it would not influence the search.
     *
     * Parameters are in the Search struct.
     * */
    virtual void HandleGo(Search& search) { }

    /*
     * the user has played the expected move. This will be sent if the engine was told to ponder on the same move
	 * the user has played. The engine should continue searching but switch from pondering to normal search.
     * */
    virtual void HandlePonderHit() { }

    /*
     * stop calculating as soon as possible,
	 * don't forget the "bestmove" and possibly the "ponder" token when finishing the search
     * */
    virtual void HandleStop() { }

    /*
     * terminate the program.
     * This command automatically exits the AI thread and the main thread after you do your cleanup in this function
     * */
    virtual void HandleQuit() { }

private:
#ifndef NDEBUG
    std::ofstream debug{"./debug.txt"};
#endif
    bool shutdown = false;

    void HandleUCI();
    void ComputeThread();

    enum class CommandType {
        UCI, Debug, SetOption, NewGame, IsReady, PosFen, PosStart, Go, PonderHit, Stop, Quit
    };

    struct Command {
        CommandType type;
        std::variant<std::vector<std::string>, std::pair<std::string, std::string>, Search, int> arg;
    };

    util::ThreadQueue<Command> queue{};

    void ParseCommand(const std::string& command);
};