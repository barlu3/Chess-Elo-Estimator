#include "Game.h"
#include "api/moveHistory.h"
#include "search/MoveExecutor.h"
#include "search/header/alphabeta.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <vector>

#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); std::exit(1); } } while (0)

struct Position { std::string id, category, name; Game game; };

static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r");
    if (b == std::string::npos) return "";
    return s.substr(b, s.find_last_not_of(" \t\r") - b + 1);
}

static std::vector<Position> loadPositions(const std::string& path) {
    std::ifstream in(path);
    CHECK(in.is_open());
    std::vector<Position> positions;
    std::string line;
    while (std::getline(in, line)) {
        if (trim(line).empty() || trim(line)[0] == '#') continue;

        std::vector<std::string> fields;
        size_t start = 0, semi;
        while ((semi = line.find(';', start)) != std::string::npos) {
            fields.push_back(trim(line.substr(start, semi - start)));
            start = semi + 1;
        }
        fields.push_back(trim(line.substr(start)));
        CHECK(fields.size() == 4);

        Position p;
        p.id = fields[1];
        p.category = fields[2];
        p.name = fields[3];

        bool loaded = p.game.loadFEN(fields[0]);
        if (!loaded) std::fprintf(stderr, "bad FEN for id %s\n", p.id.c_str());
        CHECK(loaded);

        std::vector<Move> moves = MoveExecutor::generateMoves(p.game);
        if (moves.empty()) std::fprintf(stderr, "no legal moves for id %s\n", p.id.c_str());
        CHECK(!moves.empty());

        positions.push_back(p);
    }
    return positions;
}

// Counts the nodes a full-width search visits, with no evaluation.
static long long treeCount(Game& game, MoveHistory& history, int depth) {
    if (depth == 0) return 1;
    std::vector<Move> moves = MoveExecutor::generateMoves(game);
    if (moves.empty()) return 1;
    long long count = 1;
    for (const Move& move : moves) {
        if (!MoveExecutor::make(move, game, history)) continue;
        count += treeCount(game, history, depth - 1);
        MoveExecutor::undo(game, history);
    }
    return count;
}

// Number of alphabeta() calls a pruning-off AlphaBeta::search(root, depth) makes.
static long long negamaxNodes(const Game& root, int depth) {
    std::vector<Move> rootMoves = MoveExecutor::generateMoves(root);

    unsigned int hw = std::thread::hardware_concurrency();
    size_t workers = hw == 0 ? 1 : hw;
    if (workers > rootMoves.size()) workers = rootMoves.size();

    std::atomic<size_t> next{0};
    std::atomic<long long> total{0};
    std::vector<std::thread> pool;
    for (size_t w = 0; w < workers; w++) {
        pool.emplace_back([&]() {
            Game localGame = root;
            MoveHistory history;
            long long local = 0;
            for (size_t i = next++; i < rootMoves.size(); i = next++) {
                if (!MoveExecutor::make(rootMoves[i], localGame, history)) continue;
                local += treeCount(localGame, history, depth - 1);
                MoveExecutor::undo(localGame, history);
            }
            total += local;
        });
    }
    for (std::thread& t : pool) t.join();
    return total.load();
}

static SearchResult timedSearch(const Game& game, int depth, bool pruning, bool ordering, double& ms) {
    Game g = game;
    MoveHistory history;
    AlphaBeta::pruning = pruning;
    AlphaBeta::ordering = ordering;

    auto start = std::chrono::steady_clock::now();
    SearchResult result = AlphaBeta::search(g, history, depth, 1);
    ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

    AlphaBeta::pruning = true;
    AlphaBeta::ordering = true;
    return result;
}

static void printBuildInfo() {
    std::printf("compiler: %s\n", __VERSION__);
    std::printf("flags: %s\n", BENCH_FLAGS);
    std::printf("build type: %s\n", BENCH_BUILD_TYPE_NAME);
    std::printf("hardware_concurrency: %u\n", std::thread::hardware_concurrency());
#ifndef NDEBUG
    std::printf("WARNING: NDEBUG not defined, this is not a Release build; timings are meaningless\n");
#endif
    std::fflush(stdout);
}

static void printSearch(const char* label, const Game& game, int depth, bool ordering) {
    double ms = 0;
    SearchResult r = timedSearch(game, depth, true, ordering, ms);
    std::printf("%s d%d main=%lld q=%lld score=%lld s=%.2f\n", label, depth,
                r.mainNodes, r.quiescenceNodes, r.score, ms / 1000.0);
    std::fflush(stdout);
}

static int probe(const std::string& path, const std::string& id) {
    printBuildInfo();
    std::vector<Position> positions = loadPositions(path);

    const Position* found = nullptr;
    for (const Position& p : positions) {
        if (p.id == id) { found = &p; break; }
    }
    CHECK(found != nullptr);
    const Game& game = found->game;

    auto start = std::chrono::steady_clock::now();
    long long nodes = negamaxNodes(game, 5);
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::printf("negamax d5 nodes=%lld s=%.2f\n", nodes, s);
    std::fflush(stdout);

    printSearch("mvv", game, 5, true);
    printSearch("ab", game, 5, false);
    printSearch("mvv", game, 6, true);
    printSearch("ab", game, 6, false);
    return 0;
}

struct Row {
    std::string id, category;
    long long neg5, ab5Main, ab5Q, mvv5Main, mvv5Q, score5Ab, score5Mvv;
    double ab5Ms, mvv5Ms;
    long long ab6Main, ab6Q, mvv6Main, mvv6Q;
    double ab6Ms, mvv6Ms;
};

// shortcut: depth-6 timing is a single run (counts are deterministic), raise if C3 needs depth-6 medians
static const int kRuns6 = 1;

// Runs the search `runs` times; node counts and score must match across runs.
static SearchResult measure(const Game& game, int depth, bool ordering, int runs, double& medianMs) {
    std::vector<double> times;
    SearchResult first{};
    for (int i = 0; i < runs; i++) {
        double ms = 0;
        SearchResult r = timedSearch(game, depth, true, ordering, ms);
        if (i == 0) first = r;
        CHECK(r.mainNodes == first.mainNodes && r.quiescenceNodes == first.quiescenceNodes && r.score == first.score);
        times.push_back(ms);
    }
    std::sort(times.begin(), times.end());
    medianMs = times[times.size() / 2];
    return first;
}

// Pruning-off search must visit the same tree as the node-only count; pruning and ordering must not change the score.
static void identityCheck(const Position& p, int depth, int& mismatches) {
    double ms = 0;
    SearchResult off = timedSearch(p.game, depth, false, false, ms);
    CHECK(off.mainNodes == negamaxNodes(p.game, depth));
    SearchResult ab = timedSearch(p.game, depth, true, false, ms);
    SearchResult mvv = timedSearch(p.game, depth, true, true, ms);
    if (ab.score != off.score || mvv.score != off.score) {
        std::fprintf(stderr, "SCORE MISMATCH id=%s depth=%d off=%lld ab=%lld mvv=%lld\n", p.id.c_str(), depth,
                     off.score, ab.score, mvv.score);
        mismatches++;
    }
}

static double savedPct(long long nodes, long long negamax) {
    return 100.0 * (1.0 - double(nodes) / double(negamax));
}

static void printAggregate(const char* mode, const std::vector<Row>& rows, bool mvv) {
    std::vector<double> pct;
    long long sumMain = 0, sumNeg = 0;
    for (const Row& r : rows) {
        long long main = mvv ? r.mvv5Main : r.ab5Main;
        pct.push_back(savedPct(main, r.neg5));
        sumMain += main;
        sumNeg += r.neg5;
    }
    std::sort(pct.begin(), pct.end());
    size_t n = pct.size();
    double median = n % 2 ? pct[n / 2] : (pct[n / 2 - 1] + pct[n / 2]) / 2.0;
    std::printf("%s: median per-position %% saved = %.2f, pooled %% saved (1 - sum nodes / sum negamax) = %.2f, "
                "min = %.2f, max = %.2f\n", mode, median, savedPct(sumMain, sumNeg), pct.front(), pct.back());
}

static int run(const std::string& epdPath, const std::string& csvPath) {
    printBuildInfo();
    std::vector<Position> positions = loadPositions(epdPath);
    CHECK(!positions.empty());

    int mismatches = 0, depth3Count = 0, depth4Count = 0;
    for (const Position& p : positions) {
        identityCheck(p, 3, mismatches);
        depth3Count++;
        bool deep = p.id == "op1" || p.id == "mg6" || p.id == "eg2" || p.id == "eg3" || p.id == "eg4";
        if (deep) {
            identityCheck(p, 4, mismatches);
            depth4Count++;
        }
        std::printf("identity id=%s\n", p.id.c_str());
        std::fflush(stdout);
    }
    CHECK(depth4Count >= 3);
    std::printf("identity check: main-node counts equal the node-only tree count on %d positions at depth 3 and %d at depth 4\n",
                depth3Count, depth4Count);

    std::ofstream csv(csvPath);
    CHECK(csv.is_open());
    csv << std::fixed << std::setprecision(3);
    csv << "id,category,negamax5,ab5_main,ab5_q,ab5_ms,mvv5_main,mvv5_q,mvv5_ms,score5_ab,score5_mvv,"
           "ab6_main,ab6_q,ab6_ms,mvv6_main,mvv6_q,mvv6_ms\n";
    csv.flush();

    std::vector<Row> rows;
    for (const Position& p : positions) {
        Row r;
        r.id = p.id;
        r.category = p.category;
        r.neg5 = negamaxNodes(p.game, 5);

        SearchResult ab5 = measure(p.game, 5, false, 3, r.ab5Ms);
        SearchResult mvv5 = measure(p.game, 5, true, 3, r.mvv5Ms);
        if (ab5.score != mvv5.score) {
            std::fprintf(stderr, "ROOT SCORE MISMATCH id=%s ab=%lld mvv=%lld\n", p.id.c_str(), ab5.score, mvv5.score);
            mismatches++;
        }
        SearchResult ab6 = measure(p.game, 6, false, kRuns6, r.ab6Ms);
        SearchResult mvv6 = measure(p.game, 6, true, kRuns6, r.mvv6Ms);

        r.ab5Main = ab5.mainNodes;
        r.ab5Q = ab5.quiescenceNodes;
        r.score5Ab = ab5.score;
        r.mvv5Main = mvv5.mainNodes;
        r.mvv5Q = mvv5.quiescenceNodes;
        r.score5Mvv = mvv5.score;
        r.ab6Main = ab6.mainNodes;
        r.ab6Q = ab6.quiescenceNodes;
        r.mvv6Main = mvv6.mainNodes;
        r.mvv6Q = mvv6.quiescenceNodes;

        std::printf("%s neg5=%lld ab5_main=%lld mvv5_main=%lld\n", r.id.c_str(), r.neg5, r.ab5Main, r.mvv5Main);
        std::fflush(stdout);

        csv << r.id << ',' << r.category << ',' << r.neg5 << ',' << r.ab5Main << ',' << r.ab5Q << ',' << r.ab5Ms << ','
            << r.mvv5Main << ',' << r.mvv5Q << ',' << r.mvv5Ms << ',' << r.score5Ab << ',' << r.score5Mvv << ','
            << r.ab6Main << ',' << r.ab6Q << ',' << r.ab6Ms << ',' << r.mvv6Main << ',' << r.mvv6Q << ',' << r.mvv6Ms
            << '\n';
        csv.flush();
        rows.push_back(r);
    }

    std::printf("\n| id | category | negamax | ab | ab+mvvlva | %% saved ab | %% saved ab+mvvlva | q nodes ab | "
                "q nodes ab+mvvlva | d6 ab main | d6 ab q | d6 ab+mvvlva main | d6 ab+mvvlva q |\n");
    std::printf("|---|---|---|---|---|---|---|---|---|---|---|---|---|\n");
    for (const Row& r : rows) {
        std::printf("| %s | %s | %lld | %lld | %lld | %.2f | %.2f | %lld | %lld | %lld | %lld | %lld | %lld |\n",
                    r.id.c_str(), r.category.c_str(), r.neg5, r.ab5Main, r.mvv5Main,
                    savedPct(r.ab5Main, r.neg5), savedPct(r.mvv5Main, r.neg5), r.ab5Q, r.mvv5Q,
                    r.ab6Main, r.ab6Q, r.mvv6Main, r.mvv6Q);
    }
    std::printf("\n");
    printAggregate("ab", rows, false);
    printAggregate("ab+mvvlva", rows, true);
    std::printf("mismatches: %d\n", mismatches);
    return mismatches == 0 ? 0 : 1;
}

int main(int argc, char** argv) {
    std::string cmd = argc > 1 ? argv[1] : "";
    if (cmd == "probe" && argc == 4) return probe(argv[2], argv[3]);
    if (cmd == "run" && argc == 4) return run(argv[2], argv[3]);
    std::fprintf(stderr, "usage: bench probe <epd> <id>\n       bench run <epd> <csv>\n");
    return 1;
}
