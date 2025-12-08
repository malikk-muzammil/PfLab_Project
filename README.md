Quiz Game Project (C++)

This is a simple Console-Based Quiz Game written in C++ for a Programming Fundamentals project. The game lets the user choose a category and difficulty level, then asks multiple-choice questions stored in text files. The program includes lifelines, scoring, negative marking, streak bonuses, and the ability to resume a quiz.

The code is written in a beginner-friendly, easy style using basic C++ features like structs, vectors, file handling, and functions.

PROJECT FEATURES

Categories
The game includes five categories:

Science

Computer

Sports

History

IQ/Logic

Each category has its own text file.

Difficulty Levels
There are three difficulty levels:

Easy (E)

Medium (M)

Hard (H)

Difficulty affects the timer, negative marking, and which questions are selected.

Lifelines
The program includes four lifelines:

50/50: removes two wrong options

Skip: skip question without penalty

Replace: replace current question

Extra Time: gives additional time

Each lifeline can be used once.

Scoring System
Correct answer: +1 point
Wrong answer: negative marking

Easy: -2

Medium: -3

Hard: -5

Streak bonuses:

3 correct answers: +5

5 correct answers: +15

Auto-Generated Files
The program automatically creates the following files if they are missing:
science.txt
computer.txt
sports.txt
history.txt
iq.txt
high_scores.txt
quiz_logs.txt
save_progress.txt

These files store questions, scores, logs, and resume data.

Resume Feature
If the user quits during a quiz, the program saves:

Current question number

Score

Lifelines used

Category

Difficulty

Shuffle seed

The quiz can be resumed later from the exact same point.

Add Question Feature
The user can add new questions from the menu.
Questions must follow this format:

Q: Question text
A) option A
B) option B
C) option C
D) option D
ANSWER: X
DIFF: E/M/H
CONCEPTS USED

This project demonstrates:

use of struct to group related data

use of vectors to store lists of questions

file handling using ifstream and ofstream

reading and parsing formatted text files

randomization using shuffle()

basic timing using time()

modular programming using functions

simple menu-driven program design

HOW TO RUN

Open the project in any C++ IDE (Visual Studio, CodeBlocks, Dev-C++).

Place main.cpp in your project folder.

Build and run the program.

The required text files will be created automatically.

Follow on-screen menu instructions.

NOTES

This project is written in beginner style for clarity and learning.

The text file question format must be followed correctly.

No external libraries are required.
