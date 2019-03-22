#include <curl/curl.h>
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <numeric>
#include <map>
#include <stdio.h>
#include <set>
#include <string>
#include <vector> 


// адресс источника 
const std::string singe_obs = "ftp://ftp.nmh.ac.uk/wdc/obsdata/hourval/single_obs/";

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

#define FTPBODY "ftp-list"
#define FTPHEADERS "ftp-responses"
static size_t write_response(void *ptr, size_t size, size_t nmemb, void *data) {
    FILE *writehere = (FILE *)data;
    return fwrite(ptr, size, nmemb, writehere);
}


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
 
bool download_file(char * Url, char * name_file) {
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
            return false;
        }
    }
    
    if(ftpfile.stream) {
        fclose(ftpfile.stream); 
    }
    
    curl_global_cleanup();
    return true;
}

bool download_file(std::string Url, std::string file_name) {
    char * u = to_char(Url);
    char * f = to_char(file_name);
    bool ans = download_file(u, f);
    free(u); free(f);
    return ans;
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

// КЛАССЫ ДАТЫ
class Date {
 public:
    // информация
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
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
        if (month == 4 || month == 6 || month == 9 || month == 11) {
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

const bool operator == (const Date& d1, const Date& d2) {
    if (d1.year != d2.year) {
        return false;
    }
    if (d1.month != d2.month) {
        return false;
    }
    if (d1.day != d2.day) {
        return false;
    }
    if (d1.hour != d2.hour) {
        return true;
    }
    return true;
}

std::ostream& operator << (std::ostream& s, const Date d) {
    s  << d.hour << ":" << d.day << ":" << d.month << ":" << d.year;
    return s;
}

Date str_to_date(std::string& s) {
    Date answer;
    if (s[1] == ':') {
        answer.hour = std::stoi(s.substr(0, 1));
        s = s.substr(2, s.size());
    } else {
        answer.hour = std::stoi(s.substr(0, 2));
        s = s.substr(3, s.size());
    }
    if (s[1] == ':') {
        answer.day = std::stoi(s.substr(0, 1));
        s = s.substr(2, s.size());
    } else {
        answer.day = std::stoi(s.substr(0, 2));
        s = s.substr(3, s.size());
    }
    if (s[1] == ':') {
        answer.month = std::stoi(s.substr(0, 1));
        s = s.substr(2, s.size());
    } else {
        answer.month = std::stoi(s.substr(0, 2));
        s = s.substr(3, s.size());
    }
    answer.year = std::stoi(s.substr(0, 4));
    if (s.size() > 4) {
        s = s.substr(4, s.size());
    }
    return answer;
}

bool compare(const Date& d1, const Date& d2, const Date& gap) {
    int diff = (d2.year - d1.year) * 12 * 30 * 24;
    diff += (d2.month - d1.month)  * 30 * 24;
    diff += (d2.day - d1.day) * 24;
    diff += (d2.hour - d1.hour);
    if (diff < 0) {
        diff = 0 - diff;
    }
    int int_gap = ((((gap.year * 12) + gap.month) * 30 + gap.day) * 24) + gap.hour;
    if (int_gap <= diff) {
        return false;
    } 
    return true;
}

// КЛАССЫ ДАТЫ 

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

double make_standard_lat(double lat) {
    if (lat >= -180 && lat <= 180) {
        return lat;
    }
    if (lat > 180) {
        return lat - 360;
    }
    if (lat < -180) {
        return lat + 360;
    }

}

bool ob_fit(std::string name, 
            Date first_date, Date last_date, 
            Date gap, 
            double latitude_1,
            double latitude_2, 
            double longtitude_1,
            double longtitude_2) {
    std::ifstream fin;
    fin.open(name + "_result.txt");
    std::ofstream debug;
    debug.open("DEBUG.txt", std::ofstream::app);
    std::string s;
    std::getline(fin, s);
    std::getline(fin, s);
    double o_lat, o_long;
    fin >> o_lat >> o_long;
    std::getline(fin, s);
    longtitude_1 = make_standard_lat(longtitude_1);
    longtitude_2 = make_standard_lat(longtitude_2);
    o_long = make_standard_lat(o_long);
    if (o_lat > latitude_2 || o_lat < latitude_1) {
        debug << name << " " << "latitude = " << o_lat << "\n";
        return false;
    }
    if (o_long > longtitude_2 || o_long < longtitude_1) {
        debug << name << " " << "longtitude = " << o_long << "\n";
        return false;
    }
    Date prev;
    bool opened = false;
    std::vector<std::pair<Date, int>> inf;
    // 1 - если начало
    // 0 - если конец
    while (std::getline(fin, s)) {
        Date cur_1 = str_to_date(s);
        s = s.substr(1, s.size());
        inf.push_back(std::pair<Date, int> (cur_1, 1));
        Date cur_2 = str_to_date(s);
        inf.push_back(std::pair<Date, int> (cur_2, 0));
    }
    if (inf.size() == 0) {
        return false;
    }
    int i = 0;
    while (i < inf.size() && inf[i].first < first_date) {
        ++i;
    } 
    if (i == inf.size()) {
        debug << name << " Data ends at " << inf[i - 1].first << "\n";
        return false;
    }
    if (inf[i].second == 1) {
        if (!compare(first_date, inf[i].first, gap)) {
            debug << name << " No data from " << first_date << " to " << inf[i].first << "\n";
            return false;
        }
    } 
    if (inf[i].second == 0) {
        if (last_date < inf[i].first) {
            return true;
        }
    } 
    ++i;
    while (i < inf.size() && inf[i].first < last_date) {
        if (inf[i].second == 1) {
            if (i - 1 >= 0) {
                if (!compare(inf[i - 1].first, inf[i].first, gap)) {
                    debug << name << " No data from " << inf[i - 1].first << " to " << inf[i].first << "\n";
                    return false;
                }
            }
        }
        ++i;
    }
    if (i == inf.size()) {
        if (!compare(inf[inf.size() - 1].first, last_date, gap)) {
            debug << name << " No data from " << inf[inf.size() - 1].first << " to " << last_date << "\n";
            return false;
        }
    } else {
        if (inf[i].second == 1) {
            if (!compare(inf[i - 1].first, last_date, gap)) {
                debug << name << " No data from " << inf[i - 1].first << " to " << last_date << "\n";
                return false;
            }
        }
    }
    debug.close();
    return true;
}

int get_year(std::string short_name, std::string s) {
    if (s == "hour.fmt") {
        return 0;
    }
    if (s == (short_name + ".ack")) {
        return 0;
    }
    try {
        int n = stoi(s.substr(3, 4));
        return n;    
    } catch(std::exception &e) {
        std::cout << short_name << " file_name_error: " << e.what() << "\n";
    }
    return 0;
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

std::string make_H(std::string x, std::string y) {
    std::string h = x.substr(0, 20);
    for (int j = 0; j < 24; ++j) {
        if (x.substr(20 + 4 * j, 4) == "9999" || y.substr(20 + 4 * j, 4) == "9999") {
            h += "9999";
        } else {
            int x_coord = std::stoi(x.substr(20 + 4 * j, 4));
            int y_coord = std::stoi(y.substr(20 + 4 * j, 4));
            int h_coord = std::sqrt(x_coord * x_coord + y_coord * y_coord);
            std::string h_c = std::to_string(h_coord);
            while (h_c.size() < 4) {
                h_c = " " + h_c;
            }
            h += h_c;
        }
    }
    return h;
}

void file_work(std::string name, int year, std::string file_name, std::ofstream& files_to_delete) {
    std::ifstream fin;
    std::ofstream fout;
    fin.open(file_name);
    fout.open(name + std::to_string(year) + "_H.txt");
    files_to_delete << (name + std::to_string(year) + "_H.txt" + "\n");
    std::string s;
    std::map<Date, std::string> X;
    std::map<Date, std::string> H;
    std::map<Date, std::string> Y;
    while (getline(fin, s)) {
        //std::cout << s.size() << " ";
        //break;
        Date s_day = get_data(s);
        if (s[7] == 'X') {
            auto it = Y.find(s_day);
            if (it == Y.end()) {
                X.insert(std::pair<Date, std::string> (s_day, s));
            } else {
                std::string h = make_H(s, it->second);
                h[7] = 'H';
                H.insert(std::pair<Date, std::string> (s_day, h));
                Y.erase(it);
            }
        }
        if (s[7] == 'Y') {
            auto it = X.find(s_day);
            if (it == X.end()) {
                Y.insert(std::pair<Date, std::string> (s_day, s));
            } else {
                std::string h = make_H(it->second, s);
                h[7] = 'H';
                H.insert(std::pair<Date, std::string> (s_day, h));
                X.erase(it);
            }
        }
        if (s[7] == 'H') {
            H.insert(std::pair<Date, std::string> (s_day, s));
        }
    }
    for (auto it = H.begin(); it != H.end(); ++it) {
        fout << it->second << "\n";
    }
    fout.close();
    fin.close();
    delete_file(file_name);
}

bool download_ob(std::string name, int first_year, int last_year, std::ofstream& fout) {
    std::string site = singe_obs + name + "/";
    bool if_exist = download_file(site, "files.txt");
    if (!if_exist) {
        return false;
    }
    std::ifstream fin;
    std::string line;
    fin.open("files.txt");
    std::string s;
    while (fin >> s >> s >> s >> s >> s >> s >> s >> s >> s) {
        int y = get_year(name, s);
        if (y >= first_year && y <= last_year) {
            download_file(site + s, s.substr(0, 8) + "to_delete.txt");
            file_work(name, y, s.substr(0, 8) + "to_delete.txt", fout);
        }
    }
    fin.close();
    return true;
}

void downloading(std::vector<std::string>& obs, int first_year, int last_year, std::ofstream& fout) {
    for (int i = 0; i < obs.size(); ++i) {
        download_ob(obs[i], first_year, last_year, fout);
    }
}

bool process() {
    std::cout << "Enter first Date if format hour:day:month:year\n";
    std::cout << "Example: 0:01:2:1999\n";
    std::cout << "February 1999\n";
    std::string s;
    std::getline(std::cin, s);
    //std::cout << s << "\n";
    Date first_date = str_to_date(s);
    std::cout << "Enter end Date if format hour:day:month:year\n";
    std::getline(std::cin, s);
    Date end_date = str_to_date(s);
    std::cout << "Enter gap in format hours:days:months:year\n";
    std::getline(std::cin, s);
    Date gap = str_to_date(s);
    double latitude_1, latitude_2, longtitude_1, longtitude_2;
    std::cout << "Enter smallest latitude\n";
    std::cin >> latitude_1;
    std::cout << "Enter biggest latitude\n";
    std::cin >> latitude_2;
    std::cout << "Enter smallest longitude\n";
    std::cin >> longtitude_1;
    std::cout << "Enter biggest longitude\n";
    std::cin >> longtitude_2;
    std::vector<std::string> obs_names = get_obs_names("obs.txt"); 
    std::vector<std::string> obs_to_download;
    for (int i = 0; i < obs_names.size(); ++i) {
        if (ob_fit(
            obs_names[i],
            first_date,
            end_date,
            gap,
            latitude_1, 
            latitude_2,
            longtitude_1, 
            longtitude_2
        )) {
            obs_to_download.push_back(obs_names[i]);
            std::cout << obs_names[i] << "\n";
        }
    }
    std::cout << "Do you want to download (Enter YES or NO)?\n";
    getline(std::cin, s);
    getline(std::cin, s);
    if (s.substr(0, 3) == "YES") {
        //std::cout << "OK";
        std::ofstream fout;
        fout.open("obs_inf_need_to_delete.txt");
        downloading(obs_to_download, first_date.year, end_date.year, fout);
        fout.close();
        std::cout << "Do you want to delete (Enter YES or NO)?\n";
        getline(std::cin, s);
        if (s.substr(0, 3) == "YES") {
            std::ifstream fin;
            fin.open("obs_inf_need_to_delete.txt");
            while (getline(fin, s)) {
                delete_file(s);
            }
        }
    } 
    return true;
}

int main() {
    std::ofstream fout;
    fout.open("DEBUG.txt");
    fout << "\n";
    fout.close();
    process();
    //std::ofstream files_to_delete;
    //files_to_delete.open("Files_to_delete.txt");
    //download_file("ftp://ftp.nmh.ac.uk/wdc/obsdata/hourval/single_obs/cnb/cnb2013.wdc", "example_cnb_data.txt");
    //file_work("example", 2500, "example_cnb_data.txt", files_to_delete);
}


//int main() {
    //process();
    //Date d1;
    //d1.year = 1940;
    //d1.month = 1;
    //d1.day = 1;
    //Date d2;
    //d2.year = 2018;
    //d2.month = 1;
    //d2.day = 1;
    //Date gap;
    //gap.year = 0;
    //gap.month = 3;
    //gap.day = 0;
    //std::vector<std::string> obs_names = get_obs_names("obs.txt"); 
    //for (int i = 0; i < obs_names.size(); ++i) {
    //for (int i = 0; i < 3; ++i) {
    //    if (ob_fit(
    //        obs_names[i], d1, d2, gap, -1000, 1000, -1000, 1000
    //    )) {
    //        std::cout << obs_names[i] << "\n";
    //    }
    //}
    //std::cout << ob_fit(obs_names[0], d1, d2, gap, -1000, 1000, -1000, 1000) << "\n";
//}