#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <unistd.h>
#include <csignal>
#include <signal.h>
#include <bits/stdc++.h>
//#define afname "/home/sarthak/Documents/alarms.txt"
#define ll long long
using namespace std;

string afname;

struct Date_details {
   int d, m, y;
};

const int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int countLeapYears(Date_details d) {
   int years = d.y;

   if (d.m <= 2)
      years--;

   return years / 4 - years / 100 + years / 400;
}

int getDifference(Date_details dt1, Date_details dt2) {

   long int n1 = dt1.y * 365 + dt1.d;

   for (int i = 0; i < dt1.m - 1; i++)
      n1 += monthDays[i];

   n1 += countLeapYears(dt1);

   long int n2 = dt2.y * 365 + dt2.d;
   for (int i = 0; i < dt2.m - 1; i++)
      n2 += monthDays[i];
   n2 += countLeapYears(dt2);

   return (n2 - n1);
}

class alarm_details {
public:
   int year, month, day, hour, minutes, seconds;
   string message;

   ll set_alarm_values(string alarm_time) {
      year = (alarm_time[6] - '0') * 1000 + (alarm_time[7] - '0') * 100 + (alarm_time[8] - '0') * 10 + (alarm_time[9] - '0');
      month = (alarm_time[3] - '0') * 10 + (alarm_time[4] - '0');
      day = (alarm_time[0] - '0') * 10 + (alarm_time[1] - '0');
      hour = (alarm_time[12] - '0') * 10 + (alarm_time[13] - '0');
      minutes = (alarm_time[15] - '0') * 10 + (alarm_time[16] - '0');
      seconds = (alarm_time[18] - '0') * 10 + (alarm_time[19] - '0');
      message = alarm_time.substr(22);

      // Returns difference in seconds from current date/time
      time_t now = time(0);
      tm *ltm = localtime(&now);
      int curr_day = ltm->tm_mday;
      int curr_year = 1900 + ltm->tm_year;
      int curr_month = 1 + ltm->tm_mon;
      int curr_hour = ltm->tm_hour;
      int curr_min = ltm->tm_min;
      int curr_sec = ltm->tm_sec;

      Date_details d1 = {curr_day, curr_month, curr_year};
      Date_details d2 = {day, month, year};

      int day_diff = getDifference(d1, d2);

      ll sec_diff;
      if (curr_hour > hour || (curr_hour == hour && curr_min > minutes) || (curr_hour == hour && curr_min == minutes && curr_sec > seconds)) {
         day_diff = day_diff - 1;
         sec_diff = 86400 - curr_hour * 3600 - curr_min * 60 - curr_sec;
         sec_diff = sec_diff + hour * 3600 + minutes * 60 + seconds;
         sec_diff = sec_diff + 86400 * day_diff;
      }
      else {
         sec_diff = (hour - curr_hour) * 3600 + (minutes - curr_min) * 60 + seconds - curr_sec;
         sec_diff = sec_diff + 86400 * day_diff;
      }
      return sec_diff;
   }

   void display_details() {
      string hr, min, sec, da, mon, yr;
      ostringstream s1, s2, s3, s4, s5, s6;

      s1 << hour;
      if (hour < 10)
         hr = "0" + s1.str();
      else
         hr = s1.str();

      s2 << minutes;
      if (minutes < 10)
         min = "0" + s2.str();
      else
         min = s2.str();

      s3 << seconds;
      if (seconds < 10)
         sec = "0" + s3.str();
      else
         sec = s3.str();

      s4 << day;
      if (day < 10)
         da = "0" + s4.str();
      else
         da = s4.str();

      s5 << month;
      if (month < 10)
         mon = "0" + s5.str();
      else
         mon = s5.str();

      s6 << year;
      if (year < 1000)
         yr = "0" + s6.str();
      else
         yr = s6.str();

      cout << "Date: " << da << "/" << mon << "/" << yr << endl;
      cout << "Time: " << hr << ":" << min << ":" << sec << endl;
      cout << "Message: " << message << endl;
   }
};

alarm_details curr_alarm;
unordered_map<pid_t, alarm_details> alarm_map;
vector<string> alarm_vec;

void populate_alarm_map() {
   fstream alarmfile(afname);

   string alarm_dat;
   while (getline(alarmfile, alarm_dat)) {
      // cout << alarm_dat << endl;
      ll seconds_diff = curr_alarm.set_alarm_values(alarm_dat);
      if (seconds_diff <= 0) {
         cout << "Alarm missed:- " << endl;
         curr_alarm.display_details();
         cout << endl;
      }
      else {
         //fork here
         alarm_vec.push_back(alarm_dat);
      }
      // cout << curr_alarm.message << endl << endl;
   }
   alarmfile.close();
}

void update_alarm_file() {
   int f = remove(afname.c_str());
   ofstream updatefile(afname);

   for (auto x : alarm_map) {
      string hr, min, sec, day, mon, yr;
      ostringstream s1, s2, s3, s4, s5, s6;

      s1 << x.second.hour;
      if (x.second.hour < 10)
         hr = "0" + s1.str();
      else
         hr = s1.str();

      s2 << x.second.minutes;
      if (x.second.minutes < 10)
         min = "0" + s2.str();
      else
         min = s2.str();

      s3 << x.second.seconds;
      if (x.second.seconds < 10)
         sec = "0" + s3.str();
      else
         sec = s3.str();

      s4 << x.second.day;
      if (x.second.day < 10)
         day = "0" + s4.str();
      else
         day = s4.str();

      s5 << x.second.month;
      if (x.second.month < 10)
         mon = "0" + s5.str();
      else
         mon = s5.str();

      s6 << x.second.year;
      if (x.second.year < 1000)
         yr = "0" + s6.str();
      else
         yr = s6.str();

      string update = day + "/" + mon + "/" + yr + "::" + hr + ":" + min + ":" + sec + "::" + x.second.message;
      updatefile << update << "\n";
   }

   updatefile.close();
}
