#include "QuizManager.h"
#include "Exceptions.h"
#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <limits>

/**
 * @file main_cli.cpp
 * @brief Terminal-based Entry Point for the Online Quiz System.
 * Uses the exact same C++ OOP backend classes (Question, MCQQuestion, TrueFalseQuestion,
 * Student, QuizManager, Exceptions, Templates) without needing any network ports.
 */

// Utility: Clear input buffer
static void clearBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Utility: Print section banner
static void printBanner(const std::string& title) {
    std::cout << "\n========================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================================\n";
}

// 1. Take Quiz
void runQuiz(QuizManager& manager) {
    printBanner("START ONLINE QUIZ (CLI MODE)");
    std::string rollNo, name;

    std::cout << "Enter Student Roll Number: ";
    std::getline(std::cin, rollNo);
    std::cout << "Enter Student Full Name:   ";
    std::getline(std::cin, name);

    if (rollNo.empty() || name.empty()) {
        std::cout << "[-] Roll Number and Name cannot be empty!\n";
        return;
    }

    try {
        // Retrieve randomized questions using polymorphic base class pointers
        auto questions = manager.getShuffledQuestions();
        std::map<int, std::string> answers;

        std::cout << "\n[!] Quiz Started! Total Questions: " << questions.size() << "\n";
        std::cout << "[!] For MCQ questions, enter A, B, C, or D.\n";
        std::cout << "[!] For True/False questions, enter True or False.\n\n";

        for (size_t i = 0; i < questions.size(); ++i) {
            const auto& q = questions[i];
            std::cout << "--------------------------------------------------------\n";
            std::cout << "Question " << (i + 1) << " of " << questions.size()
                      << " [" << q->getType() << " - " << q->getMarks() << " Marks]:\n";
            std::cout << q->getText() << "\n\n";

            if (q->getType() == "MCQ") {
                const MCQQuestion* mcq = dynamic_cast<const MCQQuestion*>(q.get());
                if (mcq) {
                    const auto& opts = mcq->getOptions();
                    char optChar = 'A';
                    for (const auto& opt : opts) {
                        std::cout << "  " << optChar << ") " << opt << "\n";
                        optChar++;
                    }
                }
            } else if (q->getType() == "TF") {
                std::cout << "  -> Enter: True or False\n";
            }

            std::string userAns;
            std::cout << "\nYour Answer: ";
            std::getline(std::cin, userAns);
            answers[q->getId()] = userAns;
        }

        // Evaluate using QuizManager
        EvaluationResult result = manager.evaluate(rollNo, name, answers);

        printBanner("QUIZ RESULT & SCORECARD");
        std::cout << "Student Roll No : " << result.student.getRollNo() << "\n";
        std::cout << "Student Name    : " << result.student.getName() << "\n";
        std::cout << "Total Questions : " << result.totalQuestions << "\n";
        std::cout << "Correct Answers : " << result.correctCount << "\n";
        std::cout << "Score Obtained  : " << result.student.getScore() << " / " << result.student.getTotalMarks() << "\n";
        std::cout << "Percentage      : " << std::fixed << std::setprecision(2) << result.student.getPercentage() << "%\n";
        std::cout << "Result Status   : " << (result.passed ? "[PASSED] Congratulations!" : "[FAILED] Better luck next time!") << "\n";

        std::cout << "\n--- QUESTION BREAKDOWN ---\n";
        for (const auto& rev : result.reviews) {
            std::cout << "\nQ" << rev.id << ": " << rev.text << "\n";
            std::cout << "  Your Answer    : " << rev.userAnswer << "\n";
            std::cout << "  Correct Answer : " << rev.correctAnswer << "\n";
            std::cout << "  Result         : " << (rev.isCorrect ? "[CORRECT] (+ " + std::to_string(rev.marksAwarded) + " Marks)" : "[INCORRECT] (0 Marks)") << "\n";
        }

        std::cout << "\n[+] Your result has been saved to data/results.txt successfully!\n";

    } catch (const EmptyQuizException& e) {
        std::cerr << "\n[-] Error: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[-] Unexpected Error: " << e.what() << "\n";
    }
}

// 2. View Leaderboard
void displayLeaderboard(const QuizManager& manager) {
    printBanner("STUDENT LEADERBOARD (RANKED HIGH TO LOW)");
    auto leaderboard = manager.getLeaderboard();

    if (leaderboard.empty()) {
        std::cout << "No quiz attempts recorded yet.\n";
        return;
    }

    std::cout << std::left
              << std::setw(6)  << "Rank"
              << std::setw(15) << "Roll No"
              << std::setw(22) << "Student Name"
              << std::setw(12) << "Score"
              << std::setw(14) << "Percentage"
              << "Timestamp\n";
    std::cout << std::string(80, '-') << "\n";

    int rank = 1;
    for (const auto& s : leaderboard) {
        std::string medal = (rank == 1) ? " [1st]" : (rank == 2) ? " [2nd]" : (rank == 3) ? " [3rd]" : "";
        std::string rankStr = std::to_string(rank) + medal;
        std::cout << std::left
                  << std::setw(6)  << rankStr
                  << std::setw(15) << s.getRollNo()
                  << std::setw(22) << s.getName()
                  << std::setw(12) << (std::to_string(s.getScore()) + "/" + std::to_string(s.getTotalMarks()))
                  << std::setw(14) << (std::to_string(static_cast<int>(s.getPercentage())) + "%")
                  << s.getTimestamp() << "\n";
        rank++;
    }
    std::cout << std::string(80, '-') << "\n";
}

// 3. Admin: Add New Question
void addNewQuestionAdmin(QuizManager& manager) {
    printBanner("ADMIN PANEL - ADD NEW QUESTION");
    std::cout << "Select Question Type:\n";
    std::cout << "1. Multiple Choice Question (MCQ)\n";
    std::cout << "2. True / False Question (TF)\n";
    std::cout << "Enter choice (1-2): ";

    int typeChoice;
    if (!(std::cin >> typeChoice)) {
        clearBuffer();
        std::cout << "[-] Invalid choice!\n";
        return;
    }
    clearBuffer();

    int nextId = static_cast<int>(manager.getQuestionCount()) + 1;
    std::string text;
    int marks;

    std::cout << "Enter Question Text: ";
    std::getline(std::cin, text);

    std::cout << "Enter Marks for this question: ";
    if (!(std::cin >> marks)) {
        clearBuffer();
        marks = 5;
    }
    clearBuffer();

    if (typeChoice == 1) {
        std::vector<std::string> opts(4);
        std::cout << "Enter Option A: "; std::getline(std::cin, opts[0]);
        std::cout << "Enter Option B: "; std::getline(std::cin, opts[1]);
        std::cout << "Enter Option C: "; std::getline(std::cin, opts[2]);
        std::cout << "Enter Option D: "; std::getline(std::cin, opts[3]);

        std::string correctOpt;
        std::cout << "Enter Correct Option Letter (A/B/C/D): ";
        std::getline(std::cin, correctOpt);

        manager.addNewQuestion(std::make_unique<MCQQuestion>(nextId, text, opts, correctOpt, marks));
        std::cout << "\n[+] MCQ Question #" << nextId << " saved and added to data/questions.txt!\n";

    } else if (typeChoice == 2) {
        std::string correctAns;
        std::cout << "Enter Correct Answer (True / False): ";
        std::getline(std::cin, correctAns);

        manager.addNewQuestion(std::make_unique<TrueFalseQuestion>(nextId, text, correctAns, marks));
        std::cout << "\n[+] True/False Question #" << nextId << " saved and added to data/questions.txt!\n";
    } else {
        std::cout << "[-] Invalid choice.\n";
    }
}

// 4. Demonstrate OOP Concepts & Operator Overloading in Viva
void demonstrateOOPConcepts(const QuizManager& manager) {
    printBanner("VIVA OOP CONCEPT DEMONSTRATION & VERIFICATION");

    // 1. Static Members demonstration
    std::cout << "1. STATIC MEMBERS DEMONSTRATION:\n";
    std::cout << "   - Question::totalQuestionsCreated = " << Question::getTotalQuestionsCreated() << "\n";
    std::cout << "   - Student::totalQuizzesAttempted  = " << Student::getTotalQuizzesAttempted() << "\n";

    // 2. Operator + Overloading demonstration
    std::cout << "\n2. OPERATOR OVERLOADING (+ OPERATOR):\n";
    Student s1("21BCSE001", "Demo Student", 35, 50, 70.0);
    std::cout << "   Initial Student: ";
    printStudentSummary(s1);

    // Bonus marks using operator+
    Student sWithBonus = s1 + 5;
    std::cout << "   After + 5 Bonus Marks (s1 + 5): ";
    printStudentSummary(sWithBonus);

    // Merge two quiz scores using operator+
    Student s2("21BCSE001", "Demo Student", 40, 50, 80.0);
    Student sMerged = s1 + s2;
    std::cout << "   After Merging 2 Quiz Modules (s1 + s2): ";
    printStudentSummary(sMerged);

    // 3. Template Function demonstration
    std::cout << "\n3. TEMPLATE FUNCTION (calculatePercentage):\n";
    double pctDouble = calculatePercentage<double>(42.0, 50.0);
    int pctInt = calculatePercentage<int>(42, 50);
    std::cout << "   - calculatePercentage<double>(42.0, 50.0) = " << pctDouble << "%\n";
    std::cout << "   - calculatePercentage<int>(42, 50)       = " << pctInt << "%\n";

    // 4. Generic Template Class (Repository<T>)
    std::cout << "\n4. GENERIC TEMPLATE CLASS (Repository<T>):\n";
    Repository<std::string> topicRepo;
    topicRepo.add("Polymorphism");
    topicRepo.add("Inheritance");
    topicRepo.add("Encapsulation");
    topicRepo.add("Templates");
    std::cout << "   Repository holds " << topicRepo.count() << " OOP topics.\n";

    // 5. Polymorphic Base Class pointers
    std::cout << "\n5. RUNTIME POLYMORPHISM & BASE POINTERS:\n";
    std::unique_ptr<Question> polyQ = std::make_unique<MCQQuestion>(
        999, "What is runtime polymorphism?",
        std::vector<std::string>{"Overriding", "Overloading", "Macros", "Inline"}, "A", 5);
    std::cout << "   Base Pointer (Question*) pointing to MCQQuestion object.\n";
    displayQuestionMeta(*polyQ); // Friend function call
    std::cout << "   Evaluating 'A': " << (polyQ->evaluate("A") ? "Correct" : "Incorrect") << "\n";
}

int main() {
    std::unique_ptr<QuizManager> manager;
    try {
        manager = std::make_unique<QuizManager>("data/questions.txt", "data/results.txt");
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << "\n";
        return 1;
    }

    int choice = 0;
    while (true) {
        printBanner("ONLINE QUIZ SYSTEM (C++ OOP ENGINE)");
        std::cout << "1. Attempt Quiz\n";
        std::cout << "2. View Leaderboard\n";
        std::cout << "3. Admin: Add New Question\n";
        std::cout << "4. Viva OOP Concept Verification & Demo\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice (1-5): ";

        if (!(std::cin >> choice)) {
            clearBuffer();
            std::cout << "[-] Please enter a valid number.\n";
            continue;
        }
        clearBuffer();

        switch (choice) {
            case 1:
                runQuiz(*manager);
                break;
            case 2:
                displayLeaderboard(*manager);
                break;
            case 3:
                addNewQuestionAdmin(*manager);
                break;
            case 4:
                demonstrateOOPConcepts(*manager);
                break;
            case 5:
                std::cout << "\nThank you for using the Online Quiz System. Exiting...\n";
                return 0;
            default:
                std::cout << "[-] Invalid choice! Please select 1 to 5.\n";
                break;
        }
    }

    return 0;
}

