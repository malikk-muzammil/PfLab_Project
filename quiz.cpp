// ============================================================
// quiz.cpp
// Quiz Game Project
// This program implements a console-based quiz game.
// It uses arrays, structs, loops, randomization, time tracking
// and file handling for saving/loading progress, questions,
// logs, and high scores.
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cctype>

using namespace std;

// ---------- CONFIG CONSTANTS ----------

// Maximum number of quiz categories supported
const int MAX_CATEGORIES = 5;

// Maximum questions that can be loaded from a file
const int MAX_QUESTIONS = 300;

// Maximum questions that can be played in a session
const int MAX_PLAY_QUESTIONS = 100;

// Maximum number of high scores that can be read
const int MAX_HIGHS = 200;

// File names used by the program
const string highScoreFile = "high_scores.txt";
const string logsFile = "quiz_logs.txt";
const string saveFile = "save_progress.txt";

// Available categories
string categories[MAX_CATEGORIES] = { "science", "computer", "sports", "history", "iq" };

// Penalties for wrong answers by difficulty
int penEasy = 2;
int penMed  = 3;
int penHard = 5;

// Time limits for each difficulty
int timeEasy = 20;
int timeMed  = 25;
int timeHard = 35;

// ---------- STRUCT DEFINITIONS ----------

// Structure to store one quiz question
struct Question {
    char diff;        // Difficulty level: E, M, H
    string text;      // Question text
    string A;         // Option A
    string B;         // Option B
    string C;         // Option C
    string D;         // Option D
    char correct;     // Correct option: A, B, C, or D
};

// Structure to store lifeline usage state
struct LifeLines {
    bool used5050;    // 50/50 lifeline used or not
    bool usedSkip;    // Skip lifeline used or not
    bool usedReplace; // Replace lifeline used or not
    bool usedExtra;   // Extra time lifeline used or not
};

// Structure for saving and loading game progress
struct SaveData {
    string playerName;    // Player name
    string categoryName;  // Selected category
    char diff;            // Difficulty
    unsigned long seedValue; // Random seed for shuffle
    int index;            // Current question index
    int score;            // Current score
    int correctCount;     // Number of correct answers
    int wrongCount;       // Number of wrong answers
    LifeLines life;       // Lifeline usage state
};

// ---------- UTILITY FUNCTIONS ----------

// Returns current time in format "YYYY-MM-DD HH:MM:SS"
string getTimeStringSimple() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[40];

    int y = t->tm_year + 1900;
    int mo = t->tm_mon + 1;
    int d = t->tm_mday;
    int hh = t->tm_hour;
    int mm = t->tm_min;
    int ss = t->tm_sec;

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", y, mo, d, hh, mm, ss);
    return string(buf);
}

// Checks whether a file exists or not
bool fileExistsSimple(const string &name) {
    ifstream f(name.c_str());
    bool ok = f.good();
    f.close();
    return ok;
}

// Trims whitespace from start and end of a string
string simpleTrim(const string &s) {
    int i = 0;
    int j = (int)s.length() - 1;

    while (i <= j && isspace((unsigned char)s[i])) i++;
    while (j >= i && isspace((unsigned char)s[j])) j--;

    if (j < i) return "";
    return s.substr(i, j - i + 1);
}

// Converts a character to uppercase
char upchar(char c) {
    return (char)toupper((unsigned char)c);
}

// ---------- SAMPLE FILE CREATION ----------

// Creates sample category files if they are missing
void makeSampleFilesIfMissing() {
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        string fname = categories[i] + ".txt";

        if (!fileExistsSimple(fname)) {
            ofstream out(fname.c_str());
            if (!out) continue;

            // Sample easy question
            out << "Q: Sample easy question in " << categories[i] << "?" << endl;
            out << "A) Option A" << endl;
            out << "B) Option B" << endl;
            out << "C) Option C" << endl;
            out << "D) Option D" << endl;
            out << "ANSWER: A" << endl;
            out << "DIFF: E" << endl;
            out << "---" << endl;

            // Sample medium question
            out << "Q: Sample medium question in " << categories[i] << "?" << endl;
            out << "A) Opt A" << endl;
            out << "B) Opt B" << endl;
            out << "C) Opt C" << endl;
            out << "D) Opt D" << endl;
            out << "ANSWER: B" << endl;
            out << "DIFF: M" << endl;
            out << "---" << endl;

            out.close();
        }
    }
}

// ---------- QUESTION LOADING ----------

// Loads questions from category file into array
int loadQuestionsFromFile(const string &categoryName, Question qarr[], int maxQ) {
    string fname = categoryName + ".txt";

    // Create sample files if file is missing
    if (!fileExistsSimple(fname)) {
        makeSampleFilesIfMissing();
        if (!fileExistsSimple(fname)) return 0;
    }

    ifstream in(fname.c_str());
    if (!in.is_open()) return 0;

    string line;
    Question q;
    bool reading = false;
    int count = 0;

    // Read file line by line
    while (getline(in, line) && count < maxQ) {
        string t = simpleTrim(line);
        if (t.length() == 0) continue;

        if (t.size() >= 2 && t[0] == 'Q' && t[1] == ':') {
            reading = true;
            q.diff = 'E';
            q.text = simpleTrim(t.substr(2));
            q.A = q.B = q.C = q.D = "";
            q.correct = 'A';
        }
        else if (reading && t.size() >= 2 && t[0] == 'A' && t[1] == ')') {
            q.A = simpleTrim(t.substr(2));
        }
        else if (reading && t.size() >= 2 && t[0] == 'B' && t[1] == ')') {
            q.B = simpleTrim(t.substr(2));
        }
        else if (reading && t.size() >= 2 && t[0] == 'C' && t[1] == ')') {
            q.C = simpleTrim(t.substr(2));
        }
        else if (reading && t.size() >= 2 && t[0] == 'D' && t[1] == ')') {
            q.D = simpleTrim(t.substr(2));
        }
        else if (reading && t.size() >= 7 && t.substr(0,7) == "ANSWER:") {
            string v = simpleTrim(t.substr(7));
            if (v.length() > 0) q.correct = upchar(v[0]);
        }
        else if (reading && t.size() >= 5 && t.substr(0,5) == "DIFF:") {
            string v = simpleTrim(t.substr(5));
            if (v.length() > 0) q.diff = upchar(v[0]);
        }
        else if (t == "---") {
            if (q.text != "" && q.A != "" && q.B != "" && q.C != "" && q.D != "") {
                qarr[count] = q;
                count++;
            }
            reading = false;
        }
    }

    in.close();
    return count;
}

// ---------- SHUFFLING ----------

// Shuffles the questions using Fisher-Yates algorithm
void simpleShuffle(Question arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Question temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// ---------- HIGH SCORE FUNCTIONS ----------

// Saves a high score to file
void saveHighScore(const string &name, int sc) {
    ofstream out(highScoreFile.c_str(), ios::app);
    if (!out) return;

    out << name << "|" << sc << "|" << getTimeStringSimple() << endl;
    out.close();
}

// Displays top 5 high scores
void showHighScoresSimple() {
    if (!fileExistsSimple(highScoreFile)) {
        cout << "No scores yet." << endl;
        return;
    }

    ifstream in(highScoreFile.c_str());
    if (!in.is_open()) {
        cout << "Cannot open high scores." << endl;
        return;
    }

    string line;
    string names[MAX_HIGHS];
    int scores[MAX_HIGHS];
    string times[MAX_HIGHS];
    int count = 0;

    while (getline(in, line) && count < MAX_HIGHS) {
        int p1 = -1, p2 = -1;

        for (int i = 0; i < (int)line.length(); i++) {
            if (line[i] == '|' && p1 == -1) { p1 = i; continue; }
            if (line[i] == '|' && p1 != -1) { p2 = i; break; }
        }

        if (p1 == -1 || p2 == -1) continue;

        names[count] = line.substr(0, p1);
        string scs = line.substr(p1 + 1, p2 - p1 - 1);
        times[count] = line.substr(p2 + 1);

        int s = atoi(scs.c_str());
        scores[count] = s;
        count++;
    }
    in.close();

    // Sort by score (descending)
    for (int i = 0; i < count - 1; i++) {
        int best = i;
        for (int j = i + 1; j < count; j++) {
            if (scores[j] > scores[best]) best = j;
        }
        if (best != i) {
            string tn = names[i]; names[i] = names[best]; names[best] = tn;
            int ts = scores[i]; scores[i] = scores[best]; scores[best] = ts;
            scores[best] = ts;
            string tt = times[i]; times[i] = times[best]; times[best] = tt;
        }
    }

    // print top 5
    int top = 5;
    if (count < top) top = count;
    for (int i = 0; i < top; i++) {
        cout << (i + 1) << ") " << names[i] << " - " << scores[i] << " (" << times[i] << ")" << endl;
    }
    if (count == 0) cout << "No scores yet." << endl;
}

// ---------- LOGGING ----------

// Appends a simple log entry for each quiz run
void logQuizRun(const string &name, const string &cat, char d, int score, int c, int w) {
    ofstream out(logsFile.c_str(), ios::app);
    if (!out) return;
    out << getTimeStringSimple() << " | " << name << " | " << cat << " | " << d
        << " | " << score << " | correct:" << c << " wrong:" << w << endl;
    out.close();
}

// ---------- SAVE / LOAD GAME ----------

// Writes a minimal save file containing current progress and lifelines
void saveGameSimple(const SaveData &s) {
    ofstream out(saveFile.c_str());
    if (!out) return;
    out << "PLAYER:" << s.playerName << endl;
    out << "CAT:" << s.categoryName << endl;
    out << "DIFF:" << s.diff << endl;
    out << "SEED:" << s.seedValue << endl;
    out << "INDEX:" << s.index << endl;
    out << "SCORE:" << s.score << endl;
    out << "CORRECT:" << s.correctCount << endl;
    out << "WRONG:" << s.wrongCount << endl;
    out << "5050:" << (s.life.used5050 ? "1" : "0") << endl;
    out << "SKIP:" << (s.life.usedSkip ? "1" : "0") << endl;
    out << "REP:" << (s.life.usedReplace ? "1" : "0") << endl;
    out << "EXTRA:" << (s.life.usedExtra ? "1" : "0") << endl;
    out.close();
}

// Loads save file into SaveData structure; returns false if no save
bool loadGameSimple(SaveData &s) {
    if (!fileExistsSimple(saveFile)) return false;
    ifstream in(saveFile.c_str());
    if (!in.is_open()) return false;
    string line;
    while (getline(in, line)) {
        if (line.size() >= 7 && line.substr(0,7) == "PLAYER:") {
            s.playerName = line.substr(7);
        } else if (line.size() >= 4 && line.substr(0,4) == "CAT:") {
            s.categoryName = line.substr(4);
        } else if (line.size() >= 5 && line.substr(0,5) == "DIFF:") {
            string v = line.substr(5);
            if (v.length() > 0) s.diff = upchar(v[0]);
        } else if (line.size() >= 5 && line.substr(0,5) == "SEED:") {
            string v = line.substr(5);
            s.seedValue = (unsigned long)atol(v.c_str());
        } else if (line.size() >= 6 && line.substr(0,6) == "INDEX:") {
            string v = line.substr(6);
            s.index = atoi(v.c_str());
        } else if (line.size() >= 6 && line.substr(0,6) == "SCORE:") {
            string v = line.substr(6);
            s.score = atoi(v.c_str());
        } else if (line.size() >= 8 && line.substr(0,8) == "CORRECT:") {
            string v = line.substr(8);
            s.correctCount = atoi(v.c_str());
        } else if (line.size() >= 6 && line.substr(0,6) == "WRONG:") {
            string v = line.substr(6);
            s.wrongCount = atoi(v.c_str());
        } else if (line.size() >= 5 && line.substr(0,5) == "5050:") {
            string v = line.substr(5);
            s.life.used5050 = (v == "1");
        } else if (line.size() >= 5 && line.substr(0,5) == "SKIP:") {
            string v = line.substr(5);
            s.life.usedSkip = (v == "1");
        } else if (line.size() >= 4 && line.substr(0,4) == "REP:") {
            string v = line.substr(4);
            s.life.usedReplace = (v == "1");
        } else if (line.size() >= 6 && line.substr(0,6) == "EXTRA:") {
            string v = line.substr(6);
            s.life.usedExtra = (v == "1");
        }
    }
    in.close();
    return true;
}

// Removes the save file (called when quiz completes)
void clearSaveSimple() {
    if (fileExistsSimple(saveFile)) remove(saveFile.c_str());
}

// ---------- PENALTY and TIME HELPERS ----------

// Returns penalty based on difficulty character
int getPenaltySimple(char d) {
    if (d == 'E') return penEasy;
    if (d == 'M') return penMed;
    return penHard;
}

// Returns time limit (seconds) based on difficulty
int getTimeLimitSimple(char d) {
    if (d == 'E') return timeEasy;
    if (d == 'M') return timeMed;
    return timeHard;
}

// ---------- ASK SINGLE QUESTION ----------

// Presents one question, handles lifelines and timing, returns a code:
// 1 => correct, 0 => wrong, -1 => timeout, 2 => skip, 3 => replace, 4 => invalid/no answer
int askQuestionSimple(Question q, LifeLines &life, int &streak) {
    cout << "Q: " << q.text << endl;
    cout << "A) " << q.A << endl;
    cout << "B) " << q.B << endl;
    cout << "C) " << q.C << endl;
    cout << "D) " << q.D << endl;

    // show available lifelines
    cout << "Lifelines: ";
    if (!life.used5050) cout << "[1]50/50 ";
    if (!life.usedSkip) cout << "[2]Skip ";
    if (!life.usedReplace) cout << "[3]Replace ";
    if (!life.usedExtra) cout << "[4]ExtraTime ";
    cout << endl;

    int limit = getTimeLimitSimple(q.diff);
    time_t start = time(NULL);

    cout << "Enter answer letter (A-D) or lifeline number: ";
    string inp;
    if (!getline(cin, inp)) inp = "";

    time_t now = time(NULL);
    int used = (int)difftime(now, start);

    // If lifeline chosen (numeric input)
    if (inp.length() > 0 && isdigit((unsigned char)inp[0])) {
        char c = inp[0];
        if (c == '1' && !life.used5050) {
            life.used5050 = true;
            // basic 50/50 display: show correct and first wrong
            cout << "50/50 used. Showing correct option and one wrong option:" << endl;
            if (q.correct == 'A') cout << "A) " << q.A << endl;
            else cout << "A) " << q.A << endl;
            for (char x = 'A'; x <= 'D'; x++) {
                if (x != q.correct) {
                    cout << x << ") ";
                    if (x == 'A') cout << q.A << endl;
                    if (x == 'B') cout << q.B << endl;
                    if (x == 'C') cout << q.C << endl;
                    if (x == 'D') cout << q.D << endl;
                    break;
                }
            }
            cout << "Enter answer (A-D): ";
            if (!getline(cin, inp)) inp = "";
            used = 0;
        }
        else if (c == '2' && !life.usedSkip) {
            life.usedSkip = true;
            cout << "Skipped." << endl;
            return 2;
        }
        else if (c == '3' && !life.usedReplace) {
            life.usedReplace = true;
            cout << "Replace used. This question will appear later." << endl;
            return 3;
        }
        else if (c == '4' && !life.usedExtra) {
            life.usedExtra = true;
            limit += 10; // grant +10 seconds
            cout << "Extra time granted. Enter answer: ";
            if (!getline(cin, inp)) inp = "";
            used = 0;
        }
    }

    // Check timeout after lifeline handling
    if (used > limit) {
        cout << "Time up!" << endl;
        return -1;
    }

    if (inp.length() == 0) {
        cout << "No answer entered." << endl;
        return 4;
    }

    char ans = upchar(inp[0]);
    if (ans == q.correct) {
        cout << "Correct!" << endl;
        streak++;
        return 1;
    } else if (ans >= 'A' && ans <= 'D') {
        cout << "Wrong. Correct was " << q.correct << endl;
        streak = 0;
        return 0;
    } else {
        cout << "Invalid input." << endl;
        return 4;
    }
}

// ---------- START A NEW QUIZ SESSION ----------

// Orchestrates a full quiz play session (loads questions, shuffles,
// applies lifelines, tracks score, saves progress, and finishes)
void startQuizSimple(const string &player, const string &cat, char diff) {
    // load all questions for the category
    Question pool[MAX_QUESTIONS];
    int total = loadQuestionsFromFile(cat, pool, MAX_QUESTIONS);
    if (total == 0) {
        cout << "No questions found for this category." << endl;
        return;
    }

    // filter questions by difficulty
    Question pick[MAX_PLAY_QUESTIONS];
    int pickCount = 0;
    for (int i = 0; i < total; i++) {
        if (pool[i].diff == diff) {
            if (pickCount < MAX_PLAY_QUESTIONS) {
                pick[pickCount] = pool[i];
                pickCount++;
            }
        }
    }
    if (pickCount == 0) {
        cout << "No questions with selected difficulty." << endl;
        return;
    }

    // shuffle the chosen questions and record seed for resume
    unsigned long seedVal = (unsigned long)time(NULL);
    srand((unsigned int)seedVal);
    simpleShuffle(pick, pickCount);

    int totalQ = pickCount;
    if (totalQ > 10) totalQ = 10; // limit to 10 questions per play

    // initialize lifelines and counters
    LifeLines life;
    life.used5050 = false;
    life.usedSkip = false;
    life.usedReplace = false;
    life.usedExtra = false;

    int score = 0;
    int correct = 0;
    int wrong = 0;
    int streak = 0;

    // prepare and save initial save data (so resume works)
    SaveData sd;
    sd.playerName = player;
    sd.categoryName = cat;
    sd.diff = diff;
    sd.seedValue = seedVal;
    sd.index = 0;
    sd.score = score;
    sd.correctCount = correct;
    sd.wrongCount = wrong;
    sd.life = life;
    saveGameSimple(sd);

    // main question loop
    int i = 0;
    while (i < totalQ) {
        cout << endl << "Question " << (i + 1) << " of " << totalQ << endl;
        int res = askQuestionSimple(pick[i], life, streak);
        if (res == 1) {
            score++;
            correct++;
            if (streak == 3) { score += 5; cout << "Streak +5!" << endl; }
            if (streak == 5) { score += 15; cout << "Streak +15!" << endl; }
            i++;
        }
        else if (res == 0 || res == -1) {
            score -= getPenaltySimple(diff);
            wrong++;
            i++;
        }
        else if (res == 2) {
            // skip: just advance
            i++;
        }
        else if (res == 3) {
            // replace: send this question to end of play list
            if (totalQ < MAX_PLAY_QUESTIONS) {
                pick[totalQ] = pick[i];
                totalQ++;
            }
            // shift left to remove current
            for (int s = i; s < totalQ - 1; s++) {
                pick[s] = pick[s + 1];
            }
            // do not increment i (new question now at i)
        }
        else {
            // invalid / no answer: move on without penalty
            i++;
        }

        // save progress after each question
        sd.index = i;
        sd.score = score;
        sd.correctCount = correct;
        sd.wrongCount = wrong;
        sd.life = life;
        saveGameSimple(sd);
    }

    // finalization: report results, save high score, log, and clear save
    cout << endl << "Final Score: " << score << endl;
    cout << "Correct: " << correct << "  Wrong: " << wrong << endl;

    saveHighScore(player, score);
    logQuizRun(player, cat, diff, score, correct, wrong);
    clearSaveSimple();

    cout << "Quiz completed." << endl;
}

// ---------- RESUME SAVED QUIZ ----------

// Loads save file, reconstructs shuffle using stored seed, and continues
void resumeQuizSimple() {
    SaveData sd;
    if (!loadGameSimple(sd)) {
        cout << "No saved quiz." << endl;
        return;
    }
    cout << "Resuming quiz for " << sd.playerName << " in " << sd.categoryName << " difficulty " << sd.diff << endl;

    // load category questions
    Question pool[MAX_QUESTIONS];
    int total = loadQuestionsFromFile(sd.categoryName, pool, MAX_QUESTIONS);

    // filter by difficulty into pick[]
    Question pick[MAX_PLAY_QUESTIONS];
    int pickCount = 0;
    for (int i = 0; i < total; i++) {
        if (pool[i].diff == sd.diff) {
            if (pickCount < MAX_PLAY_QUESTIONS) {
                pick[pickCount] = pool[i];
                pickCount++;
            }
        }
    }
    if (pickCount == 0) {
        cout << "No questions for this save." << endl;
        return;
    }

    // reconstruct shuffle using seed stored in save
    srand((unsigned int)sd.seedValue);
    simpleShuffle(pick, pickCount);

    int totalQ = pickCount;
    if (totalQ > 10) totalQ = 10;

    int score = sd.score;
    int correct = sd.correctCount;
    int wrong = sd.wrongCount;
    int streak = 0;
    LifeLines life = sd.life;

    int i = sd.index; // resume index
    while (i < totalQ) {
        cout << endl << "Question " << (i + 1) << " of " << totalQ << endl;
        int res = askQuestionSimple(pick[i], life, streak);
        if (res == 1) {
            score++; correct++;
            if (streak == 3) { score += 5; cout << "Streak +5!" << endl; }
            if (streak == 5) { score += 15; cout << "Streak +15!" << endl; }
            i++;
        } else if (res == 0 || res == -1) {
            score -= getPenaltySimple(sd.diff);
            wrong++;
            i++;
        } else if (res == 2) {
            i++;
        } else if (res == 3) {
            if (totalQ < MAX_PLAY_QUESTIONS) {
                pick[totalQ] = pick[i];
                totalQ++;
            }
            for (int s = i; s < totalQ - 1; s++) pick[s] = pick[s + 1];
            // do not increment i
        } else {
            i++;
        }

        // save progress after each question
        sd.index = i;
        sd.score = score;
        sd.correctCount = correct;
        sd.wrongCount = wrong;
        sd.life = life;
        saveGameSimple(sd);
    }

    cout << "Resumed Quiz Finished. Score: " << score << endl;
    saveHighScore(sd.playerName, score);
    logQuizRun(sd.playerName, sd.categoryName, sd.diff, score, correct, wrong);
    clearSaveSimple();
}

// ---------- ADD QUESTION INTERFACE ----------

// Allows user to append a new question to a category file
void addQuestionSimple() {
    cout << "Select category to add question:" << endl;
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        cout << (i + 1) << ") " << categories[i] << endl;
    }
    cout << "Enter choice: ";
    int ch = 0;
    if (!(cin >> ch)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid." << endl; return; }
    cin.ignore(1000, '\n');
    if (ch < 1 || ch > MAX_CATEGORIES) { cout << "Invalid." << endl; return; }
    int idx = ch - 1;

    cout << "Enter difficulty (E/M/H): ";
    char d;
    cin >> d; cin.ignore(1000, '\n');
    d = upchar(d);
    if (d != 'E' && d != 'M' && d != 'H') d = 'E';

    cout << "Enter question text:" << endl;
    string q; getline(cin, q);
    cout << "Option A: "; string A; getline(cin, A);
    cout << "Option B: "; string B; getline(cin, B);
    cout << "Option C: "; string C; getline(cin, C);
    cout << "Option D: "; string D; getline(cin, D);
    cout << "Correct option (A-D): ";
    char ans; cin >> ans; cin.ignore(1000, '\n');
    ans = upchar(ans);
    if (ans < 'A' || ans > 'D') ans = 'A';

    ofstream out((categories[idx] + ".txt").c_str(), ios::app);
    if (!out) { cout << "Cannot open file to add question." << endl; return; }
    out << "Q: " << q << endl;
    out << "A) " << A << endl;
    out << "B) " << B << endl;
    out << "C) " << C << endl;
    out << "D) " << D << endl;
    out << "ANSWER: " << ans << endl;
    out << "DIFF: " << d << endl;
    out << "---" << endl;
    out.close();

    cout << "Question added to " << categories[idx] << ".txt" << endl;
}

// ---------- PICK CATEGORY / DIFFICULTY HELPERS ----------

int pickCategorySimple() {
    cout << "Categories:" << endl;
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        cout << (i + 1) << ") " << categories[i] << endl;
    }
    cout << "Enter choice: ";
    int x;
    if (!(cin >> x)) { cin.clear(); cin.ignore(1000, '\n'); return -1; }
    cin.ignore(1000, '\n');
    if (x >= 1 && x <= MAX_CATEGORIES) return x - 1;
    return -1;
}

char pickDiffSimple() {
    cout << "Difficulty:" << endl;
    cout << "1) Easy" << endl;
    cout << "2) Medium" << endl;
    cout << "3) Hard" << endl;
    cout << "Enter: ";
    int d;
    if (!(cin >> d)) { cin.clear(); cin.ignore(1000, '\n'); return 'E'; }
    cin.ignore(1000, '\n');
    if (d == 1) return 'E';
    if (d == 2) return 'M';
    return 'H';
}

// ---------- MAIN MENU ----------

int main() {
    // seed random once at program start (not used for per-quiz deterministic seed)
    srand((unsigned int)time(NULL));

    // ensure sample files exist so menu options work immediately
    makeSampleFilesIfMissing();

    while (true) {
        cout << endl;
        cout << "==== QUIZ GAME MENU ====" << endl;
        cout << "1) Start Quiz" << endl;
        cout << "2) View High Scores" << endl;
        cout << "3) Resume Saved Quiz" << endl;
        cout << "4) Add Question" << endl;
        cout << "5) Exit" << endl;
        cout << "Enter choice: ";
        int ch = 0;
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input." << endl;
            continue;
        }
        cin.ignore(1000, '\n');

        if (ch == 1) {
            cout << "Enter your name: ";
            string name; getline(cin, name);
            int cat = pickCategorySimple();
            if (cat < 0) continue;
            char d = pickDiffSimple();
            startQuizSimple(name, categories[cat], d);
        }
        else if (ch == 2) {
            showHighScoresSimple();
        }
        else if (ch == 3) {
            resumeQuizSimple();
        }
        else if (ch == 4) {
            addQuestionSimple();
        }
        else if (ch == 5) {
            cout << "Goodbye!" << endl;
            break;
        }
        else {
            cout << "Invalid option." << endl;
        }
    }

    return 0;
}



