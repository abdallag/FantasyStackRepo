#pragma once
#include <string>

bool MatchToken(const std::wstring& name, const std::wstring& other, int s1, int e1, int s2, int e2);

int MatchNames(const std::wstring& name, const std::wstring& other);

