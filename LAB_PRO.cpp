// Quiz Game Project (Beginner Student Style)
// All features required by the project (difficulty, lifelines, logs, resume, files)
// Written in simple easy-to-understand code with simple variable names

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <random>

using namespace std;

// category names
string allCategories[5] = { "science", "computer", "sports", "history", "iq" };

// penalties
int penEasy = 2;
int penMed = 3;
int penHard = 5;

// time limits
int timeEasy = 20;
int timeMed = 25;
int timeHard = 35;

// filenames
string highScoreFile = "high_scores.txt";
string logsFile = "quiz_logs.txt";
string saveFile = "save_progress.txt";

// question structure
struct Question {
    char diff;
    string text;
    string A, B, C, D;
    char correct;
};

// lifeline struct
struct LifeLines {
    bool used5050 = false;
    bool usedSkip = false;
    bool usedReplace = false;
    bool usedExtra = false;
};

// saved game state
struct SaveData {
    string playerName;
    string categoryName;
    char diff;
    unsigned long seedValue;
    int index;
    int score;
    int correctCount;
    int wrongCount;
    LifeLines life;
};

// simple trim
string trim(string s) {
    while (s.size() > 0 && isspace(s.front())) s.erase(s.begin());
    while (s.size() > 0 && isspace(s.back())) s.pop_back();
    return s;
}

// safe current time string
string getTimeString() {
    time_t now = time(nullptr);
    tm timeInfo;
    localtime_s(&timeInfo, &now);

    char buff[50];
    strftime(buff, 50, "%Y-%m-%d %H:%M:%S", &timeInfo);

    return string(buff);
}

// check file existence
bool fileExists(string name) {
    ifstream f(name);
    return f.good();
}

// create sample files if missing
void makeSampleFiles() {
    for (int i = 0; i < 5; i++) {
        string f = allCategories[i] + ".txt";
        if (!fileExists(f)) {
            ofstream out(f);
            out << "Q: Sample easy question in " << allCategories[i] << "?" << endl;
            out << "A) Option A" << endl;
            out << "B) Option B" << endl;
            out << "C) Option C" << endl;
            out << "D) Option D" << endl;
            out << "ANSWER: A" << endl;
            out << "DIFF: E" << endl;
            out << "---" << endl;

            out << "Q: Sample medium question in " << allCategories[i] << "?" << endl;
            out << "A) Opt A" << endl;
            out << "B) Opt B" << endl;
            out << "C) Opt C" << endl;
            out << "D) Opt D" << endl;
            out << "ANSWER: B" << endl;
            out << "DIFF: M" << endl;
            out << "---" << endl;
        }
    }
}

// load questions
vector<Question> loadQuestions(string category) {
    vector<Question> list;
    string fname = category + ".txt";

    if (!fileExists(fname)) {
        makeSampleFiles();
    }

    ifstream in(fname);
    string line;
    Question q;
    bool reading = false;

    while (getline(in, line)) {
        string t = trim(line);

        if (t.rfind("Q:", 0) == 0) {
            reading = true;
            q = Question();
            q.text = trim(t.substr(3));
        }
        else if (reading && t.rfind("A)", 0) == 0) q.A = trim(t.substr(2));
        else if (reading && t.rfind("B)", 0) == 0) q.B = trim(t.substr(2));
        else if (reading && t.rfind("C)", 0) == 0) q.C = trim(t.substr(2));
        else if (reading && t.rfind("D)", 0) == 0) q.D = trim(t.substr(2));
        else if (reading && t.rfind("ANSWER:", 0) == 0) q.correct = toupper(trim(t.substr(7))[0]);
        else if (reading && t.rfind("DIFF:", 0) == 0) q.diff = toupper(trim(t.substr(5))[0]);
        else if (t == "---") {
            list.push_back(q);
            reading = false;
        }
    }

    return list;
}

// get penalty
int getPenalty(char d) {
    if (d == 'E') return penEasy;
    if (d == 'M') return penMed;
    return penHard;
}

// get time
int getTimeLimit(char d) {
    if (d == 'E') return timeEasy;
    if (d == 'M') return timeMed;
    return timeHard;
}

// save high score
void saveHigh(string name, int sc) {
    ofstream out(highScoreFile, ios::app);
    out << name << "|" << sc << "|" << getTimeString() << endl;
}

// log quiz
void logQuiz(string name, string cat, char d, int score, int c, int w) {
    ofstream out(logsFile, ios::app);
    out << getTimeString() << " | " << name << " | " << cat << " | "
        << d << " | " << score << " | correct:" << c << " wrong:" << w << endl;
}

// show top scores
void showScores() {
    if (!fileExists(highScoreFile)) {
        cout << "No scores yet." << endl;
        return;
    }

    vector<pair<int, string>> arr;
    ifstream in(highScoreFile);
    string line;

    while (getline(in, line)) {
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        int sc = stoi(line.substr(p1 + 1, p2 - p1 - 1));
        string txt = line.substr(0, p1) + " (" + line.substr(p2 + 1) + ")";
        arr.push_back({ sc, txt });
    }

    sort(arr.begin(), arr.end(), [](auto& a, auto& b) { return a.first > b.first; });

    for (int i = 0; i < arr.size() && i < 5; i++) {
        cout << i + 1 << ") " << arr[i].second << " - " << arr[i].first << endl;
    }
}

// save progress
void saveGame(SaveData s) {
    ofstream out(saveFile);
    out << "PLAYER:" << s.playerName << endl;
    out << "CAT:" << s.categoryName << endl;
    out << "DIFF:" << s.diff << endl;
    out << "SEED:" << s.seedValue << endl;
    out << "INDEX:" << s.index << endl;
    out << "SCORE:" << s.score << endl;
    out << "CORRECT:" << s.correctCount << endl;
    out << "WRONG:" << s.wrongCount << endl;
    out << "5050:" << s.life.used5050 << endl;
    out << "SKIP:" << s.life.usedSkip << endl;
    out << "REP:" << s.life.usedReplace << endl;
    out << "EXTRA:" << s.life.usedExtra << endl;
}

// load progress
bool loadGame(SaveData& s) {
    if (!fileExists(saveFile)) return false;

    ifstream in(saveFile);
    string line;

    while (getline(in, line)) {
        if (line.rfind("PLAYER:", 0) == 0) s.playerName = line.substr(7);
        else if (line.rfind("CAT:", 0) == 0) s.categoryName = line.substr(4);
        else if (line.rfind("DIFF:", 0) == 0) s.diff = line[5];
        else if (line.rfind("SEED:", 0) == 0) s.seedValue = stoul(line.substr(5));
        else if (line.rfind("INDEX:", 0) == 0) s.index = stoi(line.substr(6));
        else if (line.rfind("SCORE:", 0) == 0) s.score = stoi(line.substr(6));
        else if (line.rfind("CORRECT:", 0) == 0) s.correctCount = stoi(line.substr(8));
        else if (line.rfind("WRONG:", 0) == 0) s.wrongCount = stoi(line.substr(6));
        else if (line.rfind("5050:", 0) == 0) s.life.used5050 = (line.back() == '1');
        else if (line.rfind("SKIP:", 0) == 0) s.life.usedSkip = (line.back() == '1');
        else if (line.rfind("REP:", 0) == 0) s.life.usedReplace = (line.back() == '1');
        else if (line.rfind("EXTRA:", 0) == 0) s.life.usedExtra = (line.back() == '1');
    }
    return true;
}

// delete saved game
void clearSave() {
    if (fileExists(saveFile)) remove(saveFile.c_str());
}

// ask one question
int askQuestion(Question q, LifeLines& life, int& streak) {
    cout << "Q: " << q.text << endl;
    cout << "A) " << q.A << endl;
    cout << "B) " << q.B << endl;
    cout << "C) " << q.C << endl;
    cout << "D) " << q.D << endl;

    cout << "Lifelines: ";
    if (!life.used5050) cout << "[1]50/50 ";
    if (!life.usedSkip) cout << "[2]Skip ";
    if (!life.usedReplace) cout << "[3]Replace ";
    if (!life.usedExtra) cout << "[4]Extra Time ";
    cout << endl;

    int limit = getTimeLimit(q.diff);
    time_t start = time(nullptr);

    cout << "Enter answer or lifeline number: ";
    string inp;
    getline(cin, inp);

    time_t endt = time(nullptr);
    int used = difftime(endt, start);

    // lifeline input
    if (!inp.empty() && isdigit(inp[0])) {
        char c = inp[0];
        if (c == '1' && !life.used5050) {
            life.used5050 = true;
            cout << "50/50 used. Correct + one wrong shown:" << endl;
            cout << q.correct << ") Correct Option" << endl;

            for (char x = 'A'; x <= 'D'; x++) {
                if (x != q.correct) {
                    cout << x << ") Wrong option" << endl;
                    break;
                }
            }

            cout << "Enter answer: ";
            getline(cin, inp);
            used = 0;
        }
        else if (c == '2' && !life.usedSkip) {
            life.usedSkip = true;
            cout << "Skipped." << endl;
            return 2;
        }
        else if (c == '3' && !life.usedReplace) {
            life.usedReplace = true;
            cout << "Replaced." << endl;
            return 3;
        }
        else if (c == '4' && !life.usedExtra) {
            life.usedExtra = true;
            limit += 10;
            cout << "Extra time given. Enter answer: ";
            getline(cin, inp);
            used = 0;
        }
    }

    if (used > limit) {
        cout << "Time up!" << endl;
        return -1;
    }

    if (inp.empty()) return 0;

    char ans = toupper(inp[0]);
    if (ans == q.correct) {
        cout << "Correct!" << endl;
        streak++;
        return 1;
    }

    cout << "Wrong. Correct was " << q.correct << endl;
    streak = 0;
    return 0;
}

// run quiz
void startQuiz(string player, string cat, char diff) {
    vector<Question> all = loadQuestions(cat);

    vector<Question> picked;
    for (auto& q : all) if (q.diff == diff) picked.push_back(q);

    if (picked.empty()) {
        cout << "No questions found." << endl;
        return;
    }

    unsigned long seedVal = time(nullptr);
    mt19937 rng(seedVal);
    shuffle(picked.begin(), picked.end(), rng);

    int totalQ = min((int)picked.size(), 10);

    LifeLines life;
    int score = 0, correct = 0, wrong = 0, streak = 0;

    SaveData s;
    s.playerName = player;
    s.categoryName = cat;
    s.diff = diff;
    s.seedValue = seedVal;
    s.index = 0;
    s.score = score;
    s.correctCount = correct;
    s.wrongCount = wrong;
    s.life = life;

    saveGame(s);

    for (int i = 0; i < totalQ; i++) {
        cout << endl << "Question " << (i + 1) << " of " << totalQ << endl;

        int result = askQuestion(picked[i], life, streak);

        if (result == 1) {
            score++;
            correct++;
            if (streak == 3) { score += 5; cout << "Streak +5!" << endl; }
            if (streak == 5) { score += 15; cout << "Streak +15!" << endl; }
        }
        else if (result == 0 || result == -1) {
            score -= getPenalty(diff);
            wrong++;
        }
        else if (result == 2) { /* skip */ }
        else if (result == 3) picked.push_back(picked[i]);

        s.index = i + 1;
        s.score = score;
        s.correctCount = correct;
        s.wrongCount = wrong;
        s.life = life;

        saveGame(s);
    }

    cout << endl << "Final Score: " << score << endl;
    cout << "Correct: " << correct << " Wrong: " << wrong << endl;

    saveHigh(player, score);
    logQuiz(player, cat, diff, score, correct, wrong);
    clearSave();

    cout << "Quiz completed." << endl;
}

// resume quiz
void resumeQuiz() {
    SaveData s;
    if (!loadGame(s)) {
        cout << "No saved quiz." << endl;
        return;
    }

    cout << "Resuming quiz for " << s.playerName << endl;

    vector<Question> all = loadQuestions(s.categoryName);
    vector<Question> pool;
    for (auto& q : all) if (q.diff == s.diff) pool.push_back(q);

    mt19937 rng(s.seedValue);
    shuffle(pool.begin(), pool.end(), rng);

    int totalQ = min((int)pool.size(), 10);
    int score = s.score, correct = s.correctCount, wrong = s.wrongCount, streak = 0;
    LifeLines life = s.life;

    for (int i = s.index; i < totalQ; i++) {
        cout << endl << "Question " << (i + 1) << " of " << totalQ << endl;

        int r = askQuestion(pool[i], life, streak);

        if (r == 1) {
            score++; correct++;
            if (streak == 3) score += 5;
            if (streak == 5) score += 15;
        }
        else if (r == 0 || r == -1) {
            score -= getPenalty(s.diff);
            wrong++;
            streak = 0;
        }

        s.index = i + 1;
        s.score = score;
        s.correctCount = correct;
        s.wrongCount = wrong;
        s.life = life;

        saveGame(s);
    }

    cout << "Resumed Quiz Finished. Score: " << score << endl;

    saveHigh(s.playerName, score);
    logQuiz(s.playerName, s.categoryName, s.diff, score, correct, wrong);
    clearSave();
}

// pick category
int pickCategory() {
    cout << "Categories:" << endl;
    for (int i = 0; i < 5; i++) cout << i + 1 << ") " << allCategories[i] << endl;

    cout << "Enter choice: ";
    int x; cin >> x; cin.ignore();
    if (x >= 1 && x <= 5) return x - 1;
    return -1;
}

// pick difficulty
char pickDiff() {
    cout << "Difficulty:" << endl;
    cout << "1) Easy" << endl;
    cout << "2) Medium" << endl;
    cout << "3) Hard" << endl;
    cout << "Enter: ";
    int d; cin >> d; cin.ignore();
    if (d == 1) return 'E';
    if (d == 2) return 'M';
    return 'H';
}

// add new question
void addQuestion() {
    int c = pickCategory();
    if (c < 0) return;

    cout << "Difficulty (E/M/H): ";
    char d; cin >> d; cin.ignore();
    d = toupper(d);

    cout << "Enter question text:" << endl;
    string q; getline(cin, q);

    string A, B, C, D;
    cout << "Option A: "; getline(cin, A);
    cout << "Option B: "; getline(cin, B);
    cout << "Option C: "; getline(cin, C);
    cout << "Option D: "; getline(cin, D);

    cout << "Correct (A-D): ";
    char ans; cin >> ans; cin.ignore();
    ans = toupper(ans);

    ofstream out(allCategories[c] + ".txt", ios::app);
    out << "Q: " << q << endl;
    out << "A) " << A << endl;
    out << "B) " << B << endl;
    out << "C) " << C << endl;
    out << "D) " << D << endl;
    out << "ANSWER: " << ans << endl;
    out << "DIFF: " << d << endl;
    out << "---" << endl;

    cout << "Question added." << endl;
}

// main menu
int main() {
    makeSampleFiles();

    while (true) {
        cout << endl;
        cout << "==== QUIZ GAME MENU ====" << endl;
        cout << "1) Start Quiz" << endl;
        cout << "2) View High Scores" << endl;
        cout << "3) Resume Saved Quiz" << endl;
        cout << "4) Add Question" << endl;
        cout << "5) Exit" << endl;

        cout << "Enter choice: ";
        int ch; cin >> ch; cin.ignore();

        if (ch == 1) {
            cout << "Enter your name: ";
            string name; getline(cin, name);

            int cat = pickCategory();
            if (cat < 0) continue;

            char d = pickDiff();

            startQuiz(name, allCategories[cat], d);
        }
        else if (ch == 2) {
            showScores();
        }
        else if (ch == 3) {
            resumeQuiz();
        }
        else if (ch == 4) {
            addQuestion();
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
