#pragma once

class GitInfoStore {
  public:
    static const char* branch();
    static const char* describe();
    static const char* date();
    static int commitCount();
    static bool dirty();
};
