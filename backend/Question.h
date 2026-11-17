#ifndef QUESTION_H
#define QUESTION_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

/**
 * @file Question.h
 * @brief Class declarations for Question hierarchy and Student entity.
 * Demonstrates:
 * 1. Abstract Base Class & Pure Virtual Functions
 * 2. Public Inheritance (MCQQuestion, TrueFalseQuestion)
 * 3. Encapsulation, Constructors (Default, Param, Copy), Virtual Destructors
 * 4. static data members & static member functions
 * 5. this pointer usage & inline member functions
 * 6. Friend functions & Operator Overloading (<<, >>, +)
 */

// Forward declaration for friend function
class Question;
void displayQuestionMeta(const Question& q);

// ============================================================================
// 1. ABSTRACT BASE CLASS: Question
// ============================================================================
class Question {
protected:
    int id;
    std::string text;
    std::string type; // "MCQ" or "TF"
    int marks;

    // Static data member: tracks total question instances created in lifetime
    static int totalQuestionsCreated;

public:
    // 1. Default Constructor
    Question();

    // 2. Parameterized Constructor
    Question(int id, const std::string& text, const std::string& type, int marks);

    // 3. Copy Constructor (Deep Copy demonstration)
    Question(const Question& other);

    // 4. Virtual Destructor (Essential for safe polymorphic deletion via base pointers)
    virtual ~Question();

    // Pure Virtual Functions (Runtime Polymorphism / Interface Definition)
    virtual bool evaluate(const std::string& answer) const = 0;
    virtual std::string toJSON() const = 0;
    virtual std::string serialize() const = 0;
    virtual std::string getCorrectAnswer() const = 0;
    virtual std::unique_ptr<Question> clone() const = 0;

    // Static Member Function
    static int getTotalQuestionsCreated() {
        return totalQuestionsCreated;
    }

    // Inline Member Functions
    inline int getId() const { return this->id; }
    inline std::string getText() const { return this->text; }
    inline std::string getType() const { return this->type; }
    inline int getMarks() const { return this->marks; }

    // Friend Function for file/console stream output
    friend std::ostream& operator<<(std::ostream& os, const Question& q);
    friend void displayQuestionMeta(const Question& q);
};

// ============================================================================
// 2. DERIVED CLASS: MCQQuestion (Multiple Choice Question)
// ============================================================================
class MCQQuestion : public Question {
private:
    std::vector<std::string> options; // Exactly 4 options: A, B, C, D
    std::string correctOption;        // e.g. "A", "B", "C", "D"

public:
    // Default Constructor
    MCQQuestion();

    // Parameterized Constructor
    MCQQuestion(int id, const std::string& text, const std::vector<std::string>& opts,
                const std::string& correctOpt, int marks);

    // Copy Constructor
    MCQQuestion(const MCQQuestion& other);

    // Destructor
    ~MCQQuestion() override;

    // Overridden Pure Virtual Functions
    bool evaluate(const std::string& answer) const override;
    std::string toJSON() const override;
    std::string serialize() const override;
    std::string getCorrectAnswer() const override;
    std::unique_ptr<Question> clone() const override;

    // Specific Getters
    inline const std::vector<std::string>& getOptions() const { return this->options; }
    inline std::string getCorrectOption() const { return this->correctOption; }
};

// ============================================================================
// 3. DERIVED CLASS: TrueFalseQuestion
// ============================================================================
class TrueFalseQuestion : public Question {
private:
    std::string correctAnswer; // "True" or "False"

public:
    // Default Constructor
    TrueFalseQuestion();

    // Parameterized Constructor
    TrueFalseQuestion(int id, const std::string& text,
                      const std::string& correctAns, int marks);

    // Copy Constructor
    TrueFalseQuestion(const TrueFalseQuestion& other);

    // Destructor
    ~TrueFalseQuestion() override;

    // Overridden Pure Virtual Functions
    bool evaluate(const std::string& answer) const override;
    std::string toJSON() const override;
    std::string serialize() const override;
    std::string getCorrectAnswer() const override;
    std::unique_ptr<Question> clone() const override;
};

// ============================================================================
// 4. ENTITY CLASS: Student
// Demonstrates Operator Overloading (+, <<, >>), Static counters, Copy constructor
// ============================================================================
class Student {
private:
    std::string rollNo;
    std::string name;
    int score;
    int totalMarks;
    double percentage;
    std::string timestamp;

    // Static data member
    static int totalQuizzesAttempted;

public:
    // 1. Default Constructor
    Student();

    // 2. Parameterized Constructor
    Student(const std::string& rollNo, const std::string& name,
            int score, int totalMarks, double percentage,
            const std::string& timestamp = "");

    // 3. Copy Constructor
    Student(const Student& other);

    // 4. Destructor
    ~Student();

    // Static Member Functions
    static int getTotalQuizzesAttempted() {
        return totalQuizzesAttempted;
    }
    static void incrementQuizCount() {
        totalQuizzesAttempted++;
    }

    // Inline Accessors
    inline std::string getRollNo() const { return this->rollNo; }
    inline std::string getName() const { return this->name; }
    inline int getScore() const { return this->score; }
    inline int getTotalMarks() const { return this->totalMarks; }
    inline double getPercentage() const { return this->percentage; }
    inline std::string getTimestamp() const { return this->timestamp; }

    // Operator Overloading:
    // '+' operator to add bonus marks to a student's score
    Student operator+(int bonusMarks) const;

    // '+' operator to combine scores of two test modules for the same student
    Student operator+(const Student& other) const;

    // Stream Extraction & Insertion Operator Overloading
    friend std::ostream& operator<<(std::ostream& os, const Student& s);
    friend std::istream& operator>>(std::istream& is, Student& s);

    // Friend Function for summary printing
    friend void printStudentSummary(const Student& s);

    std::string toJSON() const;
};

#endif // QUESTION_H

