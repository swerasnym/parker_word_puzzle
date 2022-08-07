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
const uint32_t NO_MATCH = ~0;
int count = 0;

std::vector<std::vector<uint32_t>> find_solutions(const uint32_t state, const std::vector<uint32_t>& strings,
                                                  const std::vector<uint32_t>& map,
                                                  const std::vector<uint32_t>& map_min, const uint32_t endm) {
  if (std::popcount(state) == 26) {
    ++count;
    return std::vector<std::vector<uint32_t>>{{}};
  }

  if (std::popcount(state) == 25) {
    ++count;
    return std::vector<std::vector<uint32_t>>{{strings[map[state]]}};
  }
  std::vector<std::vector<uint32_t>> ret;

  const uint32_t end = std::min(map[state] + 1, endm);
  const uint32_t begin = map_min[state];

  for (const auto p : std::span(strings.begin() + begin, strings.begin() + end)) {
    if ((p & state) == 0) {
      const uint32_t pos = state | p;
      if (pos < (1 << 26) && map[pos] != NO_MATCH) {
        auto paths = find_solutions(pos, strings, map, map_min, endm);
        for (auto& path : paths) {
          path.emplace_back(p);
          ret.emplace_back(path);
        }
      }
    }
  }

  return ret;
}

std::vector<std::vector<uint32_t>> find_solutions(const std::vector<uint32_t>& strings,
                                                  const std::vector<uint32_t>& map,
                                                  const std::vector<uint32_t>& map_min) {
  const uint32_t endm = strings.size();
  auto solutions = find_solutions(0, strings, map, map_min, endm);
  for (auto& s : solutions) {
    std::sort(s.begin(), s.end(), std::greater<>());
  }
  std::sort(solutions.begin(), solutions.end());
  solutions.erase(std::unique(solutions.begin(), solutions.end()), solutions.end());

  return solutions;
}

void print_solution(const std::span<const uint32_t>& solution,
                    const std::multimap<uint32_t, std::string>& string_mapping, std::string str) {
  if (solution.size() == 0) {
    str.back() = '\n';
    std::cout << str;
    return;
  }
  const auto& range = string_mapping.equal_range(solution.front());
  for (auto i = range.first; i != range.second; ++i) {
    print_solution(solution.subspan(1), string_mapping, str + i->second + " ");
  }
}

void find_strings(const std::vector<uint32_t>& strings, const std::multimap<uint32_t, std::string>& string_mapping) {
  std::vector<uint32_t> map(1 << 26, NO_MATCH);
  std::vector<uint32_t> map_min(1 << 26, 0);
  std::vector<uint32_t> states;
  states.reserve(1 << 25);

  states.push_back(WIN);
  map[WIN] = 26;

  for (uint32_t i = 0; i < 26; ++i) {
    const uint32_t pos = WIN ^ (1 << i);
    states.push_back(pos);
    map[pos] = i;
  }

  std::vector<uint32_t> possibleStrings;

  for (std::size_t i = 0; i != states.size(); ++i) {
    const uint32_t state = states[i];
    if (i % 100000 == 0) {
      std::cerr << "\ri: " << i << " states: " << states.size() << " state " << std::bitset<26>(state) << " count "
                << std::popcount(state) << "        " << std::flush;
    }
    for (std::size_t si = map[state]; si != strings.size(); ++si) {
      const auto string = strings.at(si);
      if (string == (state & string)) {
        const uint32_t pos = state ^ string;
        if (pos < (1 << 26)) {
          if (map[pos] == NO_MATCH) {
            map[pos] = si;
            map_min[pos] = map[state];
            states.emplace_back(pos);
          } else {
            if (map[pos] < si) {
              map[pos] = si;
            }
            if (map_min[pos] > map[state]) {
              map_min[pos] = map[state];
            }
          }
        }
        if (state == string) {
          possibleStrings.emplace_back(string);
        }
      }
    }
  }

  std::cout << "size: " << states.size() << "\n";
  std::cout << "map[0]: " << std::bitset<26>(map[0]) << "\n";
  std::cout << "possible: " << possibleStrings.size() << "\n";
  std::cout << "strings: " << strings.size() << "\n";
  const auto& solutions = find_solutions(strings, map, map_min);
  std::cout << "solutions: " << solutions.size() << "\n";
  for (const auto& s : solutions) {
    print_solution(s, string_mapping, "");
  }
  std::cout << "solutions: " << solutions.size() << "\n";
  std::cout << "count: " << count << "\n";
}

}  // namespace

int main() {
  // Get the contents of file in a vector
  auto strings = getFiveLetterWordsFromFile("words_alpha.txt");
  std::multimap<uint32_t, std::string> string_mapping;
  std::vector<uint32_t> integer_strings;

  for (uint32_t i = 0; i < 26; ++i) {
    string_mapping.emplace((1 << i) | (1 << 26), std::string(1, 'a' + i));
    integer_strings.emplace_back((1 << i) | (1 << 26));
  }
  for (std::string& str : strings) {
    const auto integer = toInteger(str);
    if (std::cmp_equal(std::popcount(integer), str.size())) {
      string_mapping.emplace(integer, str);
      integer_strings.emplace_back(integer);
    }
  }

  std::sort(integer_strings.begin() + 26, integer_strings.end());
  integer_strings.erase(std::unique(integer_strings.begin() + 26, integer_strings.end()), integer_strings.end());

  find_strings(integer_strings, string_mapping);

  return 0;

}  // namespace
