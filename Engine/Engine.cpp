#include "Engine.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <regex>


Engine::Engine() = default;

int Engine::Run() {
    // spawn AI thread
    std::thread ai_thread(&Engine::ComputeThread, this);

    while (!shutdown) {
        // keep parsing commands
        std::string line;
        std::getline(std::cin, line);
        ParseCommand(line);
    }

    // join ai thread before quitting
    ai_thread.join();
    return 0;
}

void Engine::ComputeThread() {
    while (true) {
        while (!queue.wait_for(std::chrono::milliseconds(10))) {
            // keep waiting for commands
        }
        Command cmd = queue.pop_value();
        switch (cmd.type) {
            case CommandType::UCI: {
#ifndef NDEBUG
                debug << "uci\n";
#endif
                HandleUCI();
                break;
            }
            case CommandType::Debug: {
#ifndef NDEBUG
                debug << "debug\n";
#endif
                HandleDebug(std::get<int>(cmd.arg));
                break;
            }
            case CommandType::SetOption: {
                auto args = std::get<std::pair<std::string, std::string>>(cmd.arg);
#ifndef NDEBUG
                debug << "setoption " << args.first << ":" << args.second << "\n";
#endif
                HandleSetOption(args.first, args.second);
                break;
            }
            case CommandType::NewGame: {
#ifndef NDEBUG
                debug << "newgame\n";
#endif
                HandleNewGame();
                break;
            }
            case CommandType::IsReady: {
#ifndef NDEBUG
                debug << "isready\n";
#endif
                HandleIsReady();
                break;
            }
            case CommandType::PosFen: {
#ifndef NDEBUG
                debug << "posfen: ";
                for (auto move : std::get<std::vector<std::string>>(cmd.arg)) {
                    debug << move << "\n";
                }
#endif
                HandlePosFen(std::get<std::vector<std::string>>(cmd.arg));
                break;
            }
            case CommandType::PosStart: {
#ifndef NDEBUG
                debug << "posstart: ";
                for (auto move : std::get<std::vector<std::string>>(cmd.arg)) {
                    debug << move << "\n";
                }
#endif
                HandlePosStart(std::get<std::vector<std::string>>(cmd.arg));
                break;
            }
            case CommandType::Go:{
#ifndef NDEBUG
                debug << "go\n";
#endif
                HandleGo(std::get<Search>(cmd.arg));
                break;
            }
            case CommandType::PonderHit:{
#ifndef NDEBUG
                debug << "ponderhit\n";
#endif
                HandlePonderHit();
                break;
            }
            case CommandType::Stop: {
#ifndef NDEBUG
                debug << "stop\n";
#endif
                HandleStop();
                break;
            }
            case CommandType::Quit: {
#ifndef NDEBUG
                debug << "quit\n";
#endif
                HandleQuit();
                return;
            }
        }
#ifndef NDEBUG
        debug.flush();
#endif
    }
}

void Engine::SendMove(const std::string& move) {
    std::cout << "bestmove " << move << '\n' << std::flush;
}

void Engine::SendMove(char c0, char r0, char c1, char r1) {
    std::cout << "bestmove " << c0 << r0 << c1 << r1 << '\n' << std::flush;
}

void Engine::HandleUCI() {
    std::cout << "id name " << Name() << '\n';
    std::cout << "id author " << Author() << '\n';
    std::cout << "uciok\n" << std::flush;
}

void Engine::HandleIsReady() {
    std::cout << "readyok\n" << std::flush;
}

void Engine::ParseCommand(const std::string& line) {
    // switch first character, most commands can be determined from this
    switch (line[0]) {
        case 'u': {
            if (line.starts_with("ucinewgame")) {
                queue.push({CommandType::NewGame});
                return;
            }
            else if (line.starts_with("uci")) {
                queue.push({CommandType::UCI});
                return;
            }
            break;
        }
        case 'd': {
            if (line.starts_with("debug")) {
                queue.push({CommandType::Debug, line.ends_with("on")});
                return;
            }
            break;
        }
        case 'i': {
            if (line.starts_with("isready")) {
                queue.push({CommandType::IsReady});
                return;
            }
            break;
        }
        case 's': {
            if (line.starts_with("stop")) {
                queue.push({CommandType::Stop});
                return;
            }
            else if (line.starts_with("setoption")) {
                const std::regex setoption("setoption name (.*) value (.*)");
                std::smatch match;
                if (std::regex_match(line, match, setoption)) {
                    queue.push({CommandType::SetOption, std::make_pair(match[0], match[1])});
                    return;
                }
            }
            break;
        }
        case 'p': {
            if (line.starts_with("position")) {
                std::vector<std::string> moves{};
                const std::regex word_regex("(\\w+)");
                for (auto word = std::sregex_iterator(line.begin() + 8, line.end(), word_regex); word != std::sregex_iterator(); word++) {
                    moves.push_back(word->str());
                }
                if (line.find("fen", 9, 3)) {
                    queue.push({CommandType::PosFen, std::move(moves)});
                    return;
                }
                else if (line.find("startpos", 9, 8)) {
                    queue.push({CommandType::PosStart, std::move(moves)});
                    return;
                }
            }
            else if (line.starts_with("ponderhit")) {
                queue.push({CommandType::PonderHit});
                return;
            }
            break;
        }
        case 'g': {
            if (line.starts_with("go")) {
                Search search{-1, false, {}};
                const std::regex word_regex("(\\w+)");

                for (auto word = std::sregex_iterator(line.begin() + 3, line.end(), word_regex); word != std::sregex_iterator(); word++) {
                    const std::string _word = word->str();
                    switch (_word[0]) {
                        case 's': {
                            if (_word == "searchmoves") {
                                word++;
                                for (; word != std::sregex_iterator(); word++) {
                                    search.moves.push_back(word->str());
                                }
                                queue.push({CommandType::Go, search});
                            }
                            break;
                        }
                        case 'p': {
                            if (_word == "ponder") {
                                search.ponder = true;
                                return;
                            }
                            break;
                        }
                        case 'w': {
                            if (_word == "wtime") {
                                word++;
                                search.wtime = std::stoi(word->str());
                            }
                            else if (line.find("winc", 3, 4)) {
                                word++;
                                search.winc = std::stoi(word->str());
                            }
                            break;
                        }
                        case 'b': {
                            if (_word == "btime") {
                                word++;
                                search.btime = std::stoi(word->str());
                            }
                            else if (line.find("binc", 3, 4)) {
                                word++;
                                search.binc = std::stoi(word->str());
                            }
                            break;
                        }
                        case 'm': {
                            if (_word == "movestogo") {
                                word++;
                                search.movestogo = std::stoi(word->str());
                            }
                            else if (_word == "mate") {
                                word++;
                                search.mate = std::stoi(word->str());
                            }
                            else if (_word == "movetime") {
                                word++;
                                search.movetime = std::stoi(word->str());
                            }
                            break;
                        }
                        case 'd': {
                            if (_word == "depth") {
                                word++;
                                search.depth = std::stoi(word->str());
                            }
                            break;
                        }
                        case 'n': {
                            if (_word == "nodes") {
                                word++;
                                search.nodes = std::stoi(word->str());
                            }
                            break;
                        }
                        case 'i': {
                            if (line.find("infinite", 3, 8)) {
                                search.duration = -1;
                            }
                            break;
                        }
                        default: {
#ifndef NDEBUG
                            debug << "Bad param!\n";
#endif
                        }
                    }
                }
                queue.push({CommandType::Go, search});
                return;
            }
            break;
        }
        case 'q': {
            if (line.starts_with("quit")) {
                queue.push({CommandType::Quit});
                shutdown = true;
                return;
            }
            break;
        }
    }
    // Bad command!
#ifndef NDEBUG
    debug << "Bad command: " << line << "\n";
#endif
}