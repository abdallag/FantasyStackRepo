#include "Utils.h"

bool MatchToken(const std::wstring& name, const std::wstring& other, int s1, int e1, int s2, int e2) {
    if (e1 - s1 != e2 - s2) {
        return false;
    }
    if (e1 - s1 == 2) {
        return false;
    }
    if (e1 - s1 == 3 &&
        tolower(name[s1]) == 'd' &&
        tolower(name[s1 + 1]) == 'o' &&
        tolower(name[s1 + 2]) == 's') {
        return false;
    }

    if (e1 - s1 == 3 &&
        tolower(name[s1]) == 'r' &&
        tolower(name[s1 + 1]) == 'u' &&
        tolower(name[s1 + 2]) == 'i') {
        return false;
    }

    int k = s2;
    for (int j = s1; j < e1; j++) {
        int x = unsigned char(int(name[j]));
        
        for (; k < e2; k++) {
            int y = unsigned char(int(other[k]));
            if (isalpha(x) && isalpha(y) && tolower(x) != tolower(y)) {
                return false;
            }
            else {
                k++;
                break;
            }
        }
    }

    return true;
}

int MatchNames(const std::wstring& name, const std::wstring& other) {
    std::wstring ename = name + L"-";
    std::wstring eother = other + L"-";
    int matches = 0;
    int s1 = -1, s2 = -1, e1, e2;
    int k = 0;
    int ksave = 0;
    for (int j = 0; j < ename.size(); j++) {
        if (s1 == -1) {
            s1 = j;
        }
        int x = unsigned char(int(ename[j]));
        if (isdigit(x)) {
            break;
        }
        if (x == '-' || x == '_') {
            e1 = j;
        }
        else {
            continue;
        }
        ksave = k;
        for (; k < eother.size(); k++) {
            if (s2 == -1) {
                s2 = k;
            }
            int y = unsigned char(int(eother[k]));
            if (y == '-' || y == '_') {
                e2 = k;
                if (MatchToken(ename, eother, s1, e1, s2, e2)) {
                    matches++;
                    s2 = -1;
                    s1 = -1;
                    k++;
                    break;
                }
                else {
                    s2 = -1;
                }
            }
        }
        if (k == eother.size()) {
            k = ksave;
            s1 = -1;
            s2 = -1;
        }
    }

    return matches;
}
