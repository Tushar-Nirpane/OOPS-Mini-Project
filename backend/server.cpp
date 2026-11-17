#include "httplib.h"
#include "QuizManager.h"
#include "Exceptions.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

/**
 * @file server.cpp
 * @brief HTTP REST API Server & Static Web Server for the Online Quiz System.
 * Uses cpp-httplib (single-header C++17 server).
 */

// Simple lightweight JSON extraction helpers
static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::regex reg("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, reg) && match.size() > 1) {
        return match.str(1);
    }
    return "";
}

static int extractJsonInt(const std::string& json, const std::string& key, int defaultVal = 0) {
    std::regex reg("\"" + key + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (std::regex_search(json, match, reg) && match.size() > 1) {
        try {
            return std::stoi(match.str(1));
        } catch (...) {
            return defaultVal;
        }
    }
    return defaultVal;
}

static std::map<int, std::string> extractAnswers(const std::string& json) {
    std::map<int, std::string> answers;
    size_t answersPos = json.find("\"answers\"");
    if (answersPos == std::string::npos) return answers;

    size_t objStart = json.find('{', answersPos);
    if (objStart == std::string::npos) return answers;

    size_t objEnd = json.find('}', objStart);
    if (objEnd == std::string::npos) return answers;

    std::string answersBlock = json.substr(objStart, objEnd - objStart + 1);

    // Matches "123": "Answer" or "123":"Answer"
    std::regex pairRegex("\"([0-9]+)\"\\s*:\\s*\"([^\"]*)\"");
    auto words_begin = std::sregex_iterator(answersBlock.begin(), answersBlock.end(), pairRegex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        int qId = std::stoi(match.str(1));
        std::string ans = match.str(2);
        answers[qId] = ans;
    }
    return answers;
}

static std::vector<std::string> extractOptionsArray(const std::string& json) {
    std::vector<std::string> options;
    size_t optPos = json.find("\"options\"");
    if (optPos == std::string::npos) return options;

    size_t arrStart = json.find('[', optPos);
    if (arrStart == std::string::npos) return options;

    size_t arrEnd = json.find(']', arrStart);
    if (arrEnd == std::string::npos) return options;

    std::string arrBlock = json.substr(arrStart, arrEnd - arrStart + 1);
    std::regex itemRegex("\"([^\"]*)\"");
    auto begin = std::sregex_iterator(arrBlock.begin(), arrBlock.end(), itemRegex);
    auto end = std::sregex_iterator();

    for (std::sregex_iterator i = begin; i != end; ++i) {
        options.push_back((*i).str(1));
    }
    return options;
}

int main(int argc, char* argv[]) {
    int port = 8080;

    // Check PORT environment variable (standard on Render, Railway, Cloud Run, Heroku)
    const char* envPort = std::getenv("PORT");
    if (envPort != nullptr && envPort[0] != '\0') {
        try {
            port = std::stoi(envPort);
        } catch (...) {
            port = 8080;
        }
    } else if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    httplib::Server svr;
    std::mutex quizMutex;

    // Initialize QuizManager
    // Checks both "./data" and "../data"
    std::unique_ptr<QuizManager> manager;
    try {
        manager = std::make_unique<QuizManager>("data/questions.txt", "data/results.txt");
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << "\n";
        return 1;
    }

    // CORS & Options Pre-flight handler
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        if (req.method == "OPTIONS") {
            res.status = 200;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // 1. GET /api/questions - Returns all questions (answers hidden)
    svr.Get("/api/questions", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(quizMutex);
        try {
            std::string json = manager->getQuestionsJSON();
            res.set_content(json, "application/json");
        } catch (const QuizException& e) {
            res.status = 500;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        }
    });

    // 2. POST /api/submit - Evaluates answers and records score
    svr.Post("/api/submit", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(quizMutex);
        try {
            std::string rollNo = extractJsonString(req.body, "rollNo");
            std::string name = extractJsonString(req.body, "name");

            if (rollNo.empty() || name.empty()) {
                res.status = 400;
                res.set_content("{\"status\":\"error\",\"message\":\"Roll Number and Name are required.\"}", "application/json");
                return;
            }

            auto answers = extractAnswers(req.body);
            EvaluationResult eval = manager->evaluate(rollNo, name, answers);
            res.set_content(eval.toJSON(), "application/json");

            std::cout << "[Server] Submission recorded: " << rollNo << " - " << name
                      << " | Score: " << eval.student.getScore() << "/" << eval.student.getTotalMarks()
                      << " (" << eval.student.getPercentage() << "%)\n";

        } catch (const EmptyQuizException& e) {
            res.status = 400;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        }
    });

    // 3. GET /api/leaderboard - Returns sorted student scores
    svr.Get("/api/leaderboard", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(quizMutex);
        try {
            std::string json = manager->getLeaderboardJSON();
            res.set_content(json, "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        }
    });

    // 4. POST /api/questions - Admin endpoint to add a new question
    svr.Post("/api/questions", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(quizMutex);
        try {
            std::string type = extractJsonString(req.body, "type");
            std::string text = extractJsonString(req.body, "text");
            int marks = extractJsonInt(req.body, "marks", 5);
            int nextId = static_cast<int>(manager->getQuestionCount()) + 1;

            if (text.empty() || (type != "MCQ" && type != "TF")) {
                res.status = 400;
                res.set_content("{\"status\":\"error\",\"message\":\"Invalid question type or empty question text.\"}", "application/json");
                return;
            }

            if (type == "MCQ") {
                auto options = extractOptionsArray(req.body);
                std::string correctOpt = extractJsonString(req.body, "correctOption");
                if (options.size() < 4 || correctOpt.empty()) {
                    res.status = 400;
                    res.set_content("{\"status\":\"error\",\"message\":\"MCQ requires 4 options and a correctOption (A/B/C/D).\"}", "application/json");
                    return;
                }
                manager->addNewQuestion(std::make_unique<MCQQuestion>(nextId, text, options, correctOpt, marks));
            } else if (type == "TF") {
                std::string correctAns = extractJsonString(req.body, "correctAnswer");
                if (correctAns != "True" && correctAns != "False") {
                    res.status = 400;
                    res.set_content("{\"status\":\"error\",\"message\":\"TrueFalse requires correctAnswer to be 'True' or 'False'.\"}", "application/json");
                    return;
                }
                manager->addNewQuestion(std::make_unique<TrueFalseQuestion>(nextId, text, correctAns, marks));
            }

            res.set_content("{\"status\":\"success\",\"message\":\"Question added successfully!\",\"id\":" + std::to_string(nextId) + "}", "application/json");
            std::cout << "[Server Admin] New " << type << " Question added: ID #" << nextId << "\n";

        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}", "application/json");
        }
    });

    // 5. GET /api/stats - System stats (static variables verification)
    svr.Get("/api/stats", [&](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(quizMutex);
        std::string json = "{\"totalQuestionsCreated\":" + std::to_string(Question::getTotalQuestionsCreated())
                         + ",\"totalQuizzesAttempted\":" + std::to_string(Student::getTotalQuizzesAttempted())
                         + ",\"currentQuestionsInBank\":" + std::to_string(manager->getQuestionCount()) + "}";
        res.set_content(json, "application/json");
    });

    // Static file helper for rock-solid cloud container serving
    auto serveStatic = [](const std::string& path, const std::string& mime, httplib::Response& res) {
        std::ifstream file(path);
        if (!file.is_open()) file.open("../" + path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), mime);
            return true;
        }
        return false;
    };

    svr.Get("/", [=](const httplib::Request&, httplib::Response& res) {
        if (!serveStatic("frontend/index.html", "text/html", res)) {
            res.status = 404;
            res.set_content("frontend/index.html not found", "text/plain");
        }
    });

    svr.Get("/style.css", [=](const httplib::Request&, httplib::Response& res) {
        if (!serveStatic("frontend/style.css", "text/css", res)) {
            res.status = 404;
            res.set_content("style.css not found", "text/plain");
        }
    });

    svr.Get("/app.js", [=](const httplib::Request&, httplib::Response& res) {
        if (!serveStatic("frontend/app.js", "application/javascript", res)) {
            res.status = 404;
            res.set_content("app.js not found", "text/plain");
        }
    });

    // Static files mount point: frontend/
    svr.set_mount_point("/", "./frontend");
    svr.set_mount_point("/", "../frontend");

    std::cout << "========================================================\n";
    std::cout << "   ONLINE QUIZ SYSTEM - C++ OOP BACKEND SERVER\n";
    std::cout << "========================================================\n";
    std::cout << " Server started successfully at:\n";
    std::cout << " -> http://localhost:" << port << "/\n";
    std::cout << " Loaded " << manager->getQuestionCount() << " questions from file.\n";
    std::cout << " API Endpoints available:\n";
    std::cout << "   GET  /api/questions    - Fetch question list\n";
    std::cout << "   POST /api/submit       - Submit quiz responses\n";
    std::cout << "   GET  /api/leaderboard  - View student leaderboard\n";
    std::cout << "   POST /api/questions    - Admin: Add new question\n";
    std::cout << "   GET  /api/stats        - System OOP static metrics\n";
    std::cout << " Press Ctrl+C in this terminal to stop the server.\n";
    std::cout << "========================================================\n";

    svr.listen("0.0.0.0", port);
    return 0;
}

