#include <algorithm>
#include <bit>
#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <utility>

namespace {

std::vector<std::string> getFiveLetterWordsFromFile(const std::string& fileName) {
  std::ifstream in(fileName.c_str());
  if (!in) {
    std::cerr << "Cannot open the File : " << fileName << std::endl;
    throw std::runtime_error("Cant open file");
  }

  std::vector<std::string> strings;
  for (std::string str; std::getline(in, str);) {
    if (str.size() == 5) {
      strings.push_back(str);
    }
  }

  in.close();
  return strings;
}

uint32_t toInteger(const std::string& str) {
  uint32_t out = 0;

  // Compress string to only its characters as bits
  for (const char& c : str) {
    if ('a' <= c && c <= 'z') {
      out |= 1 << (c - 'a');
    } else if ('A' <= c && c <= 'Z') {
      out |= 1 << (c - 'A');
    }
  }
  return out;
}

const uint32_t WIN = (1 << 26) - 1;

std::vector<std::vector<uint32_t>> find_solutions(const uint32_t state, const std::vector<uint32_t>& possible,
                                                  const std::vector<uint32_t>& map) {
  if (std::popcount(state) == 25 && map[state] != 0) {
    return std::vector<std::vector<uint32_t>>{{map[state]}};
  }
  std::vector<std::vector<uint32_t>> ret;

  for (const uint32_t p : possible) {
    if ((p & state) == 0) {
      const uint32_t pos = state | p;

      // if (pos != state && map[pos] != 0) {
      auto paths = find_solutions(pos, possible, map);
      for (auto& path : paths) {
        path.emplace_back(p);
        ret.emplace_back(path);
      }
    }
  }

  return ret;
}

std::vector<std::vector<uint32_t>> find_solutions(const std::vector<uint32_t>& possible,
                                                  const std::vector<uint32_t>& map) {
  auto solutions = find_solutions(0, possible, map);
  for (auto& s : solutions) {
    std::sort(s.begin(), s.end(), std::greater<>());
  }
  std::sort(solutions.begin(), solutions.end());
  solutions.erase(std::unique(solutions.begin(), solutions.end()), solutions.end());

  return solutions;
}

void print_solution(const std::span<const uint32_t>& solution,
                    const std::multimap<uint32_t, std::string>& string_mapping,
		    std::string str) {
  if (solution.size() == 0) {
    str.back()='\n';
    std::cout << str;
    return;
  }
  const auto& range = string_mapping.equal_range(solution.front());
  for (auto i = range.first; i != range.second; ++i) {
    print_solution(solution.subspan(1), string_mapping, str + i->second + " ");
  }
}

void find_strings(const std::vector<uint32_t>& strings, const std::multimap<uint32_t, std::string>& string_mapping) {
  std::vector<uint32_t> map(1 << 26);
  std::vector<uint32_t> states;
  states.reserve(1 << 25);

  states.push_back(WIN);
  map[WIN] = WIN;

  for (uint32_t i = 0; i < 26; ++i) {
    const uint32_t pos = WIN ^ (1 << i);

    states.push_back(pos);
    map[pos] = (1 << i) | (1 << 26);
  }

  std::vector<uint32_t> possibleStrings;

  for (std::size_t i = 1; i != states.size(); ++i) {
    const uint32_t state = states[i];
    if (i % 100000 == 0) {
      std::cout << "\ri: " << i << " states: " << states.size() << " state " << std::bitset<26>(state) << " count "
                << std::popcount(state) << "        " << std::flush;
    }

    for (const auto string : strings) {
      if (string == (state & string)) {
        const uint32_t pos = state ^ string;
        if (map[pos] == 0) {
          map[pos] = string;
          states.emplace_back(pos);
        }
        if (state == string) {
          possibleStrings.emplace_back(string);
        }
      }
    }
  }

  // // Print solvable words...
  // for (const uint32_t p : possibleStrings) {
  //   const auto& range = string_mapping.equal_range(p);
  //   for (auto i = range.first; i != range.second; ++i) {
  //     std::cout << std::bitset<26>(i->first) << ": " << i->second << '\n';
  //   }
  // }

  std::cout << "size: " << states.size() << "\n";
  std::cout << "map[0]: " << std::bitset<26>(map[0]) << "\n";
  const auto& solutions = find_solutions(possibleStrings, map);
  std::cout << "solutions: " << solutions.size() << "\n";
  for (const auto& s : solutions) {
    print_solution(s, string_mapping, "");
  }
}
}  // namespace

int main() {
  // Get the contents of file in a vector
  auto strings = getFiveLetterWordsFromFile("words_alpha.txt");
  std::multimap<uint32_t, std::string> string_mapping;
  std::vector<uint32_t> integer_strings;

  for (std::string& str : strings) {
    const auto integer = toInteger(str);
    if (std::cmp_equal(std::popcount(integer), str.size())) {
      string_mapping.emplace(integer, str);
      integer_strings.emplace_back(integer);
    }
  }

  for (uint32_t i = 0; i < 26; ++i) {
    string_mapping.emplace((1 << i) | (1 << 26), std::string(1, 'a' + i));
  }

  std::sort(integer_strings.begin(), integer_strings.end());
  integer_strings.erase(std::unique(integer_strings.begin(), integer_strings.end()), integer_strings.end());

  find_strings(integer_strings, string_mapping);

  return 0;

}  // namespace
