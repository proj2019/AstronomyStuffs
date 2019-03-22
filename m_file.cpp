#include <curl/curl.h>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <fstream>
#include <numeric>
#include <map>
#include <stdio.h>
#include <set>
#include <string>
#include <vector> 

// адресс источника 
const std::string singe_obs = "ftp://ftp.nmh.ac.uk/wdc/obsdata/hourval/single_obs/";

// ошибки
std::vector<std::string> errors;


// БЛОК СКАЧИВАНИЯ ФАЙЛОВ
char * to_char(std::string s) {
    char * c = (char *) malloc(sizeof(*c) * (s.size() + 1));
    char * c1 = c;
    for (int i = 0; i < s.size(); ++i) {
        (*c1) = s[i];
        ++c1;
    }
    (*c1) = 0;
    return c;
}

// скачивание из файла
struct FtpFile {
    const char *filename;
    FILE *stream;
};
 
static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpFile *out = (struct FtpFile *)stream;
    if(out && !out->stream) {
        out->stream = fopen(out->filename, "wb");
        if(!out->stream)
        return -1;  
    }
    return fwrite(buffer, size, nmemb, out->stream);
}
 
void download_file(char * Url, char * name_file) {
    CURL *curl;
    CURLcode res;
    struct FtpFile ftpfile = {
        name_file,
        NULL
    };
 
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, Url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
    
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
        res = curl_easy_perform(curl);
    
        curl_easy_cleanup(curl);
        if(CURLE_OK != res) {
            fprintf(stderr, "curl told us %d\n", res);
        }
    }
    
    if(ftpfile.stream) {
        fclose(ftpfile.stream); 
    }
    
    curl_global_cleanup();
}

void download_file(std::string Url, std::string file_name) {
    char * u = to_char(Url);
    char * f = to_char(file_name);
    download_file(u, f);
    free(u); free(f);
}

void delete_file(char * file_name) {
    remove(file_name);
}

void delete_file(std::string file_name) {
    char * f = to_char(file_name);
    remove(f);
    free(f);
}
// БЛОК СКАЧИВАНИЯ ФАЙЛОВ 

// КЛАСС ИНТЕРВАЛА
// ОТВЕЧАЕТ ЗА ОБЪЕДИНЕНИЕ И ПЕРЕСЕЧЕНИЕ
template <class T>
class Interval {
 public:
    std::pair<T, T> inter;

    Interval () {
    }

    Interval (T a, T b) {
        inter.first = a;
        inter.second = b;
    }

    T first() const {
        return inter.first;
    }

    T second() const {
        return inter.second;
    }
};

template <class T>
void print(std::map<T, int>& x) {
    for (auto it = x.begin(); it != x.end(); ++it) {
        std::cout << it->first << " " << it->second << "\n";
    }
}

template <class T>
class Intervals {
 public:
    std::vector<Interval<T>> parts;

    Intervals() {
    }

    Intervals(std::vector<Interval<T>>& v) {
        parts = v;
    }

    Intervals(std::vector<T>& v) {
        for (int i = 0; i * 2 + 1 < v.size(); ++i) {
            parts.push_back(Interval<T> (v[i * 2], v[i * 2 + 1]));
        }
    }

    Interval<T> * begin() {
        parts.begin();
    }

    Interval<T> * end() {
        parts.end();
    }

    bool check() const {
        if (parts.size() == 0) {
            return true;
        }
        if (parts[0].first() >= parts[0].second()) {
            return false;
        }
        for (int i = 0; i < parts.size(); ++i) {
            if (parts[i].first() >= parts[i].second()) {
                return false;
            }
            if (parts[i - 1].second() >= parts[i].first()) {
                return false;
            }
        }
        return true;
    }

    void fit() {
        // 1 - открфвается 
        // -1 - если закрытвается
        std::map<T, int> points;
        for (int i = 0; i < parts.size(); ++i) {
            auto it = points.find(parts[i].first());
            if (it != points.end()) {
                ++(it->second);
            } else {
                points.insert(std::pair<T, int> (parts[i].first(), 1));
            }
            it = points.find(parts[i].second());
            if (it != points.end()) {
                --(it->second);
            } else {
                points.insert(std::pair<T, int> (parts[i].second(), -1));
            }
        }
        T current = points.begin()->first; 
        parts.clear(); 
        int opened = points.begin()->second;
        for (auto it = (++points.begin()); it != points.end(); ++it) {
            //std::cout << current;
            opened += it->second;
            if (opened == 0) {
                parts.push_back(Interval<T> (current, it->first));
                auto next = ++it; --it;
                if (next != points.end()) {
                    current = next->first;
                }
            }
        }
    }

    void push_back(const Interval<T>& pushed) {
        parts.push_back(pushed);
    }

    Intervals unit(const Intervals& other) {
        for (int i = 0; i < other.parts.size(); ++i) {
            parts.push_back(other.parts[i]);
        }
        fit();
        return *this;
    }

    Intervals intersection(const Intervals& other) {
        std::map<T, int> points;
        for (int i = 0; i < other.parts.size(); ++i) {
            parts.push_back(other.parts[i]);
        }
        for (int i = 0; i < parts.size(); ++i) {
            auto it = points.find(parts[i].first());
            if (it != points.end()) {
                ++(it->second);
            } else {
                points.insert(std::pair<T, int> (parts[i].first(), 1));
            }
            it = points.find(parts[i].second());
            if (it != points.end()) {
                --(it->second);
            } else {
                points.insert(std::pair<T, int> (parts[i].second(), -1));
            }
        }
        //print(points);
        parts.clear(); 
        int opened = 0;
        for (auto it = points.begin(); it != points.end(); ++it) {
            opened += it->second;
            //std::cout << it->first << " ";
            if (opened >= 2) {
                auto next = ++it;
                --it;
                parts.push_back(Interval<T> (it->first, next->first));
            }
        }
        return (*this);
    }
};
// КЛАСС ИНТЕРВАЛОВ 

template <typename T>
std::ostream& operator << (std::ostream& s, const Intervals<T>& x) {
    for (int i = 0; i < x.parts.size(); ++i) {
        s << x.parts[i].first() << " " << x.parts[i].second();
        s << "\n";
    }
    return s;
}

// КЛАССЫ ДАТЫ
class Date {
 public:
    // информация
    int year;
    int month;
    int day;
    int hour;
    // информация

    bool bissextile(int year) const {
        if (year % 4 == 0 && year % 100 != 0) {
            return true;
        }
        if (year % 400 == 0) {
            return true;
        }
        return false;
    }

    int day_in_month() const {
        if (month == 2) {
            if (bissextile(year)) {
                return 29;
            } else {
                return 28;
            }
        }
        if (month == 4 || month == 6 || month == 8 || month == 10) {
            return 30;
        }
        return 31;
    }

    bool operator == (const Date& other) const {
        if (hour != other.hour) {
            return false;
        }
        if (year != other.year) {
            return false;
        }
        if (day != other.day) {
            return false;
        }
        if (month != other.month) {
            return false;
        }
        return true;
    }

    bool operator != (const Date& other) const {
        return !((*this) == other);
    }

    Date next() const 
    {
        Date ans = (*this);
        if (hour < 23) {
            ++ans.hour;
            return ans;
        }
        if (day < day_in_month()) {
            ans.hour = 0;
            ++ans.day;
            return ans;
        }
        if (month < 12) {
            ans.hour = 0;
            ans.day = 1;
            ++ans.month;
            return ans;
        }
        ans.hour = 0;
        ans.day = 1;
        ans.month = 1;
        ++ans.year;
        return ans;
    }

    Date operator ++ () {
        if (hour < 23) {
            ++hour;
            return (*this);
        }
        if (day < day_in_month()) {
            hour = 0;
            ++day;
            return (*this);
        }
        if (month < 12) {
            hour = 0;
            day = 1;
            ++month;
            return (*this);
        }
        hour = 0;
        day = 1;
        month = 1;
        ++year;
        return (*this);
    }

    Date operator ++ (int) {
        Date ans = (*this);
        if (hour < 23) {
            ++hour;
            return ans;
        }
        if (day < day_in_month()) {
            hour = 0;
            ++day;
            return ans;
        }
        if (month < 12) {
            hour = 0;
            day = 1;
            ++month;
            return ans;
        }
        hour = 0;
        day = 1;
        month = 1;
        ++year;
        return ans;
    }
};

const bool operator < (const Date& d1, const Date& d2) {
    if (d1.year < d2.year) {
        return true;
    }
    if (d1.year > d2.year) {
        return false;
    }
    if (d1.month < d2.month) {
        return true;
    }
    if (d1.month > d2.month) {
        return false;
    }
    if (d1.day < d2.day) {
        return true;
    }
    if (d1.day > d2.day) {
        return false;
    }
    if (d1.hour < d2.hour) {
        return true;
    }
    return false;
}

std::ostream& operator << (std::ostream& s, const Date d) {
    s  << d.hour << ":" << d.day << ":" << d.month << ":" << d.year << "\n";
    return s;
}
// КЛАССЫ ДАТЫ 

// КЛАСС ОБСЕРВАТОРИИ 
class Observatory {
 public:
    // DATA
    std::string full_name;
    std::string short_name;
    double o_lat;
    double o_long;
    int first_year = 3000; 
    int last_year = 0;
    Intervals<Date> answer;
    Intervals<Date> X;
    Intervals<Date> Y;
    Intervals<Date> H;

    std::string site;
    // DATA

    Observatory() {
    }

    Observatory(const std::string& short_n) {
        short_name = short_n;
        site = singe_obs + short_n + "/";
    }

    void get_ack() {
        std::ifstream fin;
        fin.open(short_name + ".ack");
        std::string s;
        std::getline(fin, s);
        full_name = s.substr(0, s.size() - 2);
        std::getline(fin, s); 
        fin >> s >> s >> s >> s;
        fin >> o_lat;
        fin >> s;
        fin >> o_long;
        fin.close();
    }

    bool get_years(const std::string& s) {
        if (s == "hour.fmt") {
            return false;
        }
        if (s == (short_name + ".ack")) {
            return false;
        }
        try {
            int n = stoi(s.substr(3, 4));
            if (n < first_year) {
                first_year = n;
            } 
            if (n > last_year) {
                last_year = n;
            }
        } catch(std::exception &e) {
            std::cout << short_name << " file_name_error: " << e.what() << "\n";
        }
        return true;
    }

    Date get_data(std::string& s) {
        Date ans;
        ans.month = stoi(s.substr(5, 2));
        ans.year = stoi(s.substr(3, 2));
        ans.day = stoi(s.substr(8, 2));
        ans.year += 100 * stoi(s.substr(14, 2));
        ans.hour = 0;
        return ans;
    }

    Date last_X;
    bool add_X(std::string& s, Date& opened, bool& is_opened) {
        Date last_X;
        Date cur = get_data(s);
        if ((last_X.next()) != cur) {
            if (last_X.year != 0) {
                X.push_back(Interval<Date> (opened, ++last_X));
                is_opened = false;
            } 
        }
        for (int j = 0; j < 24; ++j) {
            cur.hour = j;
            if (s.substr(20 + 4 * j, 4) == "9999") {
                if (is_opened) {
                    X.push_back(Interval<Date> (opened, cur));
                    is_opened = false;
                }
            } else {
                if (!is_opened) {
                    is_opened = true;
                    opened = cur;
                }
            }
        }
        last_X = cur;
        return true;
    }
    

    Date last_Y;
    bool add_Y(std::string& s, Date& opened, bool& is_opened) {
        Date cur = get_data(s);
        if ((last_Y.next()) != cur) {
            if (last_Y.year != 0) {
                Y.push_back(Interval<Date> (opened, ++last_Y));
                is_opened = false;
            } 
        }
        for (int j = 0; j < 24; ++j) {
            cur.hour = j;
            if (s.substr(20 + 4 * j, 4) == "9999") {
                if (is_opened) {
                    Y.push_back(Interval<Date> (opened, cur));
                    is_opened = false;
                }
            } else {
                if (!is_opened) {
                    is_opened = true;
                    opened = cur;
                }
            }
        }
        last_Y = cur;
        return true;
    }

    Date last_H;
    bool add_H(std::string& s, Date& opened, bool& is_opened) {
        Date cur = get_data(s);
        if ((last_H.next()) != cur) {
            if (last_H.year != 0) {
                H.push_back(Interval<Date> (opened, ++last_H));
                is_opened = false;
            } 
        }
        for (int j = 0; j < 24; ++j) {
            cur.hour = j;
            if (s.substr(20 + 4 * j, 4) == "9999") {
                if (is_opened) {
                    H.push_back(Interval<Date> (opened, cur));
                    is_opened = false;
                }
            } else {
                if (!is_opened) {
                    is_opened = true;
                    opened = cur;
                }
            }
        }
        last_H = cur;
        return true;
    }

    void writing() {
        std::ofstream fout;
        fout.open(short_name + ".txt");
        fout << full_name << "\n";
        fout << short_name << "\n";
        fout << o_lat << " " << o_long << "\n";
        for (auto it = answer.begin(); it != answer.end(); ++it) {
            fout << it->first() << " " << it->second() << "\n";
        }
        fout.close();
    }

    bool get_periods() {
        std::ifstream fin;
        Date opened_X;
        bool if_opened_X = false;
        Date opened_Y;
        bool if_opened_Y = false;
        Date opened_H;
        bool if_opened_H = false;
        for (int i = first_year; i <= last_year; ++i) {
            try {
                fin.open(short_name + std::to_string(i) + ".wdc");
                std::string s;
                Date current;
                while (getline(fin, s)) {
                     std::cout << "length is" << s.size() << "\n";
                    if (s.size() != 120) {
                        errors.push_back(short_name);
                        std::cout << "form_error in " << short_name << "\n";
                    } else {
                        if (s[7] == 'X') {
                            add_X(s, opened_X, if_opened_X);
                        }
                        if (s[7] == 'Y') {
                            add_Y(s, opened_Y, if_opened_Y);
                        }
                        if (s[7] == 'H') {
                            add_H(s, opened_H, if_opened_H);
                        }
                    }
                }
                fin.close();
            } catch(...) {} 
        } 
        if (if_opened_X) {
            X.push_back(Interval<Date> (opened_X, ++last_X));
        }
        if (if_opened_Y) {
            X.push_back(Interval<Date> (opened_Y, ++last_Y));
        }
        if (if_opened_H) {
            X.push_back(Interval<Date> (opened_X, ++last_X));
        }
        answer = X;
        answer.intersection(Y);
        answer.unit(H);
        return true;
    }

    bool get_periods_1() {
        Date opened_date;
        bool is_open = false;
        for (int i = first_year; i <= last_year; ++i) {
            std::ifstream fin;
            fin.open(short_name + std::to_string(i) + ".wdc");
            std::string s;
            Date current;
            while (getline(fin, s)) {
                if (s.size() != 120) {
                    errors.push_back(short_name);
                    std::cout << "form_error in " << short_name << "\n";
                    return false;
                }
                if (s[7] == 'X' || s[7] == 'x') {
                    //add_X(s);
                }
                if (s[7] == 'Y' || s[7] == 'y') {
                    //add_Y(s);
                }
                if (s[7] == 'Z' || s[7] == 'z') {
                    //add_Z(s);
                }
                if (s[7] == 'H' || s[7] == 'h') {
                    //add_H(s);
                }
                if (s[7] == 'F' || s[7] == 'f') {
                    //add_F(s);
                }
                current.month = stoi(s.substr(5, 2));
                current.year = stoi(s.substr(3, 2));
                current.day = stoi(s.substr(8, 2));
                current.year += 100 * stoi(s.substr(14, 2));
                for (int j = 0; j < 24; ++j) {
                    if (s.substr(20 + 4 * j, 4) == "9999") {
                        if (is_open) {
                            current.hour = j;
                            //Period per(opened_date, current);
                            //periods.push_back(per);
                            is_open = false;
                        }
                    } else {
                        if (!is_open) {
                            is_open = true;
                            current.hour = j;
                            opened_date = current;
                        }
                    }
                }

            } 
            fin.close();
            return true;
        }

        //close_all();

        if (is_open) {
                //current.hour = 23;
                //Period per(opened_date, current);
                //periods.push_back(per);
            }
        return true;
    }

    void down_load_files() {
        download_file(site, "files.txt");
        std::ifstream fin;
        std::string line;
        fin.open("files.txt");
        std::string s;
        while (fin >> s >> s >> s >> s >> s >> s >> s >> s >> s) {
            get_years(s);
            download_file(site + s, s);
            //file_inf();
        }
        fin.close();
    }

    void get_inf() {
        //down_load_files();
        get_ack();
        get_periods(); 
        //delete_files();
        writing();
    }

    void delete_files() {
        std::ifstream fin;
        std::string line;
        fin.open("files.txt");
        std::string s;
        while (fin >> s >> s >> s >> s >> s >> s >> s >> s >> s) {
            delete_file(s);
        }
        fin.close();
        delete_file("files.txt");
    }

    ~Observatory() {
        delete_files();
    }
};

void obs_to_file(const Observatory& o, std::string file_name) {
    std::ofstream fout(file_name);
    fout << o.full_name << "\n" << o.short_name << "\n";
    fout << o.o_lat << " " << o.o_long << "\n";
    //for (int i = 0; i < o.periods.size(); ++i) {
    //    fout << o.periods[i].d1 << " " << o.periods[i].d2 << "\n";
    //}
    fout.close();
}
// КЛАСС ОБСЕРВАТОРИИ 

std::vector<std::string> get_obs_names(std::string file_name) {
    std::ifstream fin;
    fin.open(file_name);
    std::vector<std::string> obser;
    std::string s;
    while (std::getline(fin, s)) {
        s = s.substr(s.size() - 3, 3);
        obser.push_back(s);
    }
    return obser;
}

//int main(void) {
//    std::vector<std::string> obs_names = get_obs_names("obs.txt");
//    std::cout << obs_names.size() << "\n";
//    Observatory ex(obs_names[0]);
//    ex.get_inf();
//    obs_to_file(ex, "ex.txt");
//    return 0;
//}

int main() {
    Observatory tr("wia");
    tr.get_inf();
}
