#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>

using namespace std;
set<string> guessed_words; 
map<string, long double> freq_map;

map<char, array<int, 5>> LetterFrequency(vector<string> candidates);
map<string, long double> ScoreOfWords(vector<string> candidates, map<char, array<int, 5>> freq);

void InitCandidates(vector<string> &candidates) {
  ifstream fin1("../data/all_words.txt");

  string word;
  while (fin1 >> word) {
    candidates.push_back(word);
  }
}

void InitFrequency() {
  ifstream fin3("../data/words_freq.txt");
  string s, word;
  long double freq=0;
  int parity=0;
  while (fin3 >> s) {
    if (parity % 2 == 0) {
      word = s;
    } else {
      freq = stold(s);
      freq_map.insert({word, freq});
    }
    parity++;
  }
}

bool isPresent(const char &c, const string &s) {
  bool flag = false;
  for (const char &x : s) {
    if (c == x) flag = true;
  }
  return flag;
}

bool isPresent(const char &c, const vector<char> &v) {
  bool flag = false;
  for (const char &x : v) {
    if (c == x) flag = true;
  }
  return flag;
}

bool isValidWord(string &s) {
  if (s.size() != 5) return false;
  for (char &c : s) {
    c = tolower(c);
    if (c < 'a' || c > 'z') return false;
  }

  return true;
}

bool isValidResponse(string &s) {
  if (s.size() != 5) return false;
  for (char &c : s) {
    c = tolower(c);
    if (c != 'g' && c != 'y' && c != 'b') return false;
  }

  return true;
}

vector<pair<char, int>> GoodLetter(string response, string guess) {
  vector<pair<char, int>> good_letters;
  for (int i = 0; i < 5; ++i) {
    if (response[i] == 'g') {
      good_letters.push_back({guess[i], i});
    }
  }
  return good_letters;
}

vector<char> BadLetters(string response, string guess) {
  vector<char> bad_letters;
  for (int i = 0; i < 5; ++i) {
    if (response[i] == 'b') {
      bad_letters.push_back(guess[i]);
    }
  }
  return bad_letters;
}

vector<pair<char, int>> MisplacedLetters(string response, string guess) {
  vector<pair<char, int>> misplaced_letters;
  for (int i = 0; i < 5; ++i) {
    if (response[i] == 'y') {
      misplaced_letters.push_back({guess[i], i});
    }
  }
  return misplaced_letters;
}

vector<string> FilterWords(string response, string guess, vector<string> candidates) {
  auto good_letters = GoodLetter(response, guess);
  auto bad_letter = BadLetters(response, guess);
  auto misplaced_letters = MisplacedLetters(response, guess);

  vector<char> present_letters;
  for (auto &w : good_letters) present_letters.push_back(w.first);
  for (auto &w : misplaced_letters) present_letters.push_back(w.first);

  vector<string> possibleCandidates;

  for (string &word : candidates) {
    bool flag = true;

    // Case 1-> discarding all the black letters except those which are present
    for (char &bad : bad_letter) {
      if (isPresent(bad, word) && !isPresent(bad, present_letters)) {
        flag = false;
        break;
      }
    }

    // Case 2 -> Discarding all the words where
    //           correct letters are not present
    //           at respective index of the guessed word
    for (auto &itr : good_letters) {
      char good = itr.first;
      int index = itr.second;

      if (good != word[index]) {
        flag = false;
        break;
      }
    }

    // Case 3 -> Discarding all the words where
    //           misplaced letters are present at respective index
    //           of the guessed word
    for (auto &itr : misplaced_letters) {
      char misplaced = itr.first;
      int index = itr.second;

      if (misplaced == word[index]) {
        flag = false;
        break;
      }
    }

    // Case 4-> Discarding all the words where the
    //          letters that are present in present_letters
    //          are not present in word
    for (char &present : present_letters) {
      if (!isPresent(present, word)) {
        flag = false;
        break;
      }
    }

    // Case 5-> discarding all the cases where
    //          where frequency of words
    //          that are not present and are present is not the same in the word.
    for (char &present : present_letters) {
      if (isPresent(present, bad_letter)) {
        if (count(present_letters.begin(), present_letters.end(), present) !=
            count(word.begin(), word.end(), present)) {
          flag = false;
          break;
        }
      }
    }
    if (flag) possibleCandidates.push_back(word);
  }

  auto freq = LetterFrequency(possibleCandidates);
  auto word_scores =  ScoreOfWords(possibleCandidates, freq);

  // Sorting possible candidates
  sort(possibleCandidates.begin(), possibleCandidates.end(), [&](string a, string b){
    return word_scores[a] < word_scores[b];
  });

  // Printing all possible candidates
  cout << "\n Possible Words:\n";
  for (int i=0; i< min(5, (int)possibleCandidates.size()); i++) {
    cout << (i+1) << ". " << possibleCandidates[i] << "\n";
  };

  return possibleCandidates;
}

map<char, array<int, 5>> LetterFrequency(vector<string> candidates) {
  map<char, array<int, 5>> freq;

  for (char alph = 'a'; alph <= 'z'; ++alph) {
    array<int, 5> table;
    for (string &word : candidates) {
      for (int i = 0; i < 5; ++i) {
        if (word[i] == alph) table[i] += 1;
      }
    }
    freq.insert({alph, table});
  }

  return freq;
}

map<string, long double> ScoreOfWords(vector<string> candidates, map<char, array<int, 5>> freq) {
  map<string, long double> word_score;
  array<int, 5> max_freq;
  for (auto itr : freq) {
    for (int i = 0; i < 5; ++i) {
      max_freq[i] = max(max_freq[i], itr.second[i]);
    }
  }

  for (string &word : candidates) {
    long double cur_score = 1.0;
    for (int i = 0; i < 5; ++i) {
      char letter = word[i];
      // Less score means better
      cur_score = (cur_score * (1 + 1.0 * (freq[letter][i] - max_freq[i])) * (freq[letter][i] - max_freq[i]));
    }

    cur_score /= (freq_map[word]);
    word_score.insert({word, cur_score});
  }

  return word_score;
}

string BestWord(vector<string> candidates, map<char, array<int, 5>> freq) {
  long double max_score = 1e30;
  string best_word = "score";
  auto word_scores = ScoreOfWords(candidates, freq);

  for (string &word : candidates) {
    if (guessed_words.count(word) == 0 &&  word_scores[word] < max_score) {
      max_score = word_scores[word];
      best_word = word;
    }
  }
  if (guessed_words.size() == 0) return "score"; // First word is score
  else return best_word; // remaining words
}

void Solver(vector<string> candidates) {
  int counter = 1;
  for (string num : {"1st", "2nd", "3rd", "4th", "5th", "6th"}) {
    cout << "\n";

    cout << "The " << num << " suggested Word : " << BestWord(candidates, LetterFrequency(candidates)) << "\n";
    string guess;
    string response;

    cout << "Enter your " << num << " guess : ";
    cin >> guess;

    while (!isValidWord(guess)) {
      cout << "Please Enter correct guess : ";
      cin >> guess;
    }
    guessed_words.insert(guess);
    cout << "Enter your response ";
    if (num == "1st") {
      cout << "(eg. GGBYB) : ";
    } else {
      cout << ": ";
    }
    cin >> response;

    while (!isValidResponse(response)) {
      cout << "Please Enter correct response : ";
      cin >> response;
    }

    candidates = FilterWords(response, guess, candidates);

    if (candidates.size() == 0) {
      cout << "\nNo words left!!!\n";
      return;
    }

    if (response == "ggggg") {
      cout << "\nCongrats, We found the word in " << counter << " try! :)\n";
      return;
    }
    counter++;
  }

  if (counter > 6) {
    cout << "\nWe could not find the word! :(\n";
  }
}
int main() {
  vector<string> candidates;
  InitCandidates(candidates);
  InitFrequency();

  Solver(candidates);
}