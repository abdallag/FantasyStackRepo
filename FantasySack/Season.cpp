#include "Season.h"

Season::Season(const std::string season_name) : seasonName(season_name) {
    char envvar[100];
    size_t l;
    if (getenv_s(&l, envvar, "FILEOUT") == 0 && std::string(envvar, l) != "") {
        pout = &filestream;
        filestream.open((season_name + "_" + envvar).c_str());
        filestream.imbue(std::locale(filestream.getloc(), new std::ctype<wchar_t>()));
    }
}

int Season::FindPlayer(const std::wstring& player_name) {
    int maxMatch = 0;
    int maxi = -1;
    for (int i = 0; i < pcount; i++) {
        const std::wstring& other_name = player[i].name;
        int match = MatchNames(player_name, other_name);
        if (match > 1 && match > maxMatch) {
            maxMatch = match;
            maxi = i;
        }
    }
    return maxi;
}

void Season::ResetPlayers(int gw) {
    std::wostream& fout = *pout;
    for (int i = 0; i < pcount; i++) {
        Player& p = player[i];
        p.price = p.gwprice[gw];
        p.effective_points = 0;
        int eval_weeks = gw;
        if (max_reval_weeks)
            eval_weeks = max_reval_weeks;
        int nbadweeks = 0;
        int max_points = 0;
        for (int j = gw; j > 0 && eval_weeks > 0; j--) {
            if (skipped_week[j])
                continue;

            auto gwp = p.gwpoints[j];
            p.effective_points += gwp;
            eval_weeks--;
            if (max_points < p.gwpoints[j])
                max_points = p.gwpoints[j];
        }


        int counted_weeks = 0;
        for (int j = gw; j > 0 && counted_weeks < bad_form_weeks; j--) {
            if (skipped_week[j] || !p.gwgames[j])
                continue;

            counted_weeks++;

            if (p.gwpoints[j] < 3) {
                nbadweeks++;
            }
        }
        if (bad_form_weeks && nbadweeks >= bad_form_weeks && p.pos != 1) {
            p.skip = true;
        }
        else {
            p.skip = false;
        }
    }
}

int Season::Load(bool xpts) {
    std::wostream& fout = *pout;
    std::map<std::wstring, int> name_to_index;

    std::string name = seasonName + ".txt";
    if (xpts) {
        name = seasonName + "_opta.txt";
    }
    std::wifstream fin(name.c_str());
    if (!fin.is_open()) {
        fout << "Error reading season file" << std::endl;
        return 10;
    }

    fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    int i;

    while (fin >> i)
    {
        Player& p = player[i];
        p.skip = false;

        fin >> p.name >> p.pos >> p.points >> p.price >> p.prev_points >> p.team;
        if (p.points > 200 && p.team == LIVERPOOL) {
            fout << " " << p.points << " " << p.prev_points << " " << p.price << " " << p.team << std::endl;
        }
        name_to_index[p.name] = pcount++;
        for (int j = 0; j < 50; j++) {
            p.gwprice[j] = 0;
            p.gwpoints[j] = 0;
            p.gwxpts[j] = 0;
            p.gwgames[j] = false;
        }
    }
    fout << "loading GW data" << std::endl;
    name = seasonName + "_gw.txt";
    if (xpts)
        name = seasonName + "_gw_opta.txt";
    fin.close();
    fin.open(name.c_str());
    if (!fin.is_open()) {
        fout << "Error reading gameweeks file" << std::endl;
        max_gw = 38;

        return 0;
    }
    fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    int gw;
    max_gw = 0;
    while (fin >> gw)
    {
        std::wstring name;
        fin >> name;
        auto it = name_to_index.find(name);
        if (it == name_to_index.end()) {
            fout << "Player not found " << gw << " " << name << "\n";
            int x, y, z;
            fin >> x >> y;
            if (xpts) {
                fin >> z;
            }
            continue;
        }
        Player& p = player[it->second];
        int points, price;
        fin >> points >> price;
        p.gwpoints[gw] += points;
        p.gwprice[gw] = price;
        if (xpts) {
            int xp;
            fin >> xp;
            p.gwxpts[gw] += xp;
        }
        if (gw > max_gw) {
            max_gw = gw;
        }
        p.gwgames[gw] = true;
    }

    if (xpts) {
        name = seasonName + "_gw.txt";
        fin.close();
        fin.open(name.c_str());
        if (!fin.is_open()) {
            fout << "Error reading gameweeks file" << std::endl;
            max_gw = 38;

            return 0;
        }
        std::map<std::wstring, int> lookup;
        fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
        int gw;
        while (fin >> gw)
        {
            std::wstring name;
            int points, price;
            fin >> name >> points >> price;
            auto it = lookup.find(name);
            if (it == lookup.end()) {
                int ind = FindPlayer(name);

                if (ind < 0 && points > 0) {
                    fout << "Player not found " << gw << " " << name << "\n";
                    continue;
                }
                Player& p = player[ind];
                if (points != p.gwpoints[gw]) {
                    fout << "Player points doesn't match: " << gw << " " << name << "\n";
                    continue;
                }
                lookup.emplace(name, ind);
                it = lookup.find(name);
            }

            player[it->second].gwprice[gw] = price;
        }
    }

    fout << "Price highlights\n**************\n\n";
    for (int i = 0; i < pcount; i++) {
        Player& p = player[i];
        // Fill missing prices
        int last_price = p.price;
        for (int j = 0; j < 50; j++) {
            if (p.gwprice[j] == 0) {
                p.gwprice[j] = last_price;
            }
            else {
                if (p.price == 0) {
                    p.price = p.gwprice[j];
                }
            }
            last_price = p.gwprice[j];
        }

        // update points to the sum of all points
        player[i].points = 0;
        for (int j = 0; j < 50; j++) {
            player[i].points += player[i].gwpoints[j];
        }

        if (player[i].team == LIVERPOOL) {
            if (player[i].points > 200) {
                fout << player[i].name << " " << player[i].points << std::endl;
            }
        }
    }
    fout << std::endl;

    for (int j = 0; j < 50; j++) {
        skipped_week[j] = false;
    }

    if (seasonName == "2019-20") {
        max_wildcards = 3;
        fixed_wcweek = 39;
        for (int i = 30; i < 39; i++) {
            skipped_week[i] = true;
        }
    }

    return 0;
}

void Season::ApplyXpts(int gw, int weeks) {
    for (int i = 0; i < pcount; i++) {
        player[i].effective_points = 0;
        for (int j = 0; j < weeks; j++) {
            player[i].effective_points += player[i].gwxpts[gw + j];
        }
        player[i].skip = false;
    }
}
