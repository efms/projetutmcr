#include "LedsMng.h"
#include "ICalendarParser.h"
#include "CalConnector.h"
#include "Arduino_testing.h"
#include "util.h"
#include <stdint.h>
#include <time.h>
#include <string>
#include <fstream>
#include <iostream>
char ICS_FILE[] = "example.ics";

int main (void){
  /* test ICDate functionnality */
  // 1. Test icdate
  time_t testdate_time;
  time_t testdate_time2;
  struct tm *testdate_tm;
  char testdate[] = "20180927T120512Z";
  char testdate2short[] = "20180927T120512";
  testdate_time = ICDate::setFromICString(testdate);
  testdate_time2 = ICDate::setFromICString(testdate2short);
  testdate_tm = gmtime(&testdate_time);
  if(testdate_tm->tm_year == 2018 - 1900 &&
     testdate_tm->tm_mon  == 9 - 1 &&
     testdate_tm->tm_mday == 27   &&
     testdate_tm->tm_hour == 12   &&
     testdate_tm->tm_min  == 5    &&
     testdate_tm->tm_sec  == 12   &&
     testdate_time2 == 0)
    std::cout << "ICDate setFromICString test  passed\n";
  else
    std::cerr << "ICDate setFromICString test  FAILED\n";
  // 2. Test ICline
  ICline icline;
  char testdtstart[] = "DTSTART";
  icline.setName(testdtstart);
  icline.setValue(testdate);
  if (strcmp(icline.getName(),testdtstart) == 0 &&
      strcmp(icline.getValue(), testdate) == 0)
    std::cout << "ICline manual set test  passed\n";
  else
    std::cerr << "ICline manual set test  FAILED\n";
  char testicstr[] = "NAME;param1=42,param2=\"\\\"\":VALUE";
  icline.setFromICString(testicstr);
  if (strcmp(icline.getName(),"NAME") == 0 &&
      strcmp(icline.getValue(),"VALUE") == 0)
    std::cout << "ICline setFromICString test  passed\n";
  else
    std::cerr << "ICline setFromICString test  FAILED\n";
  // 3. test ICVevent
  ICVevent vevent = ICVevent();
  char testlocation[] = "TD9";
  char testsummary[] = "Maths-Physique-Electromag TD";
  struct tm testbeg = {0,0,16,26,10 - 1,2018 - 1900,0,0,0};
  struct tm testend = {0,0,18,26,10 - 1,2019 - 1900,0,0,0};
  time_t testbeg_t = timegm(&testbeg);
  time_t testend_t = timegm(&testend);
  vevent.setDtstart(testbeg_t);
  vevent.setDtend(testend_t);
  vevent.setLocation(testlocation);
  vevent.setSummary(testsummary);
  if (ICDate::getUtcYear(vevent.getDtstart())  == 2018 &&
      ICDate::getUtcMonth(vevent.getDtstart()) == 10 &&
      ICDate::getUtcDay(vevent.getDtstart())   == 26 &&
      ICDate::getUtcYear(vevent.getDtend())    == 2019 &&
      ICDate::getUtcMonth(vevent.getDtend())   == 10 &&
      ICDate::getUtcDay(vevent.getDtend())     == 26 &&
      strcmp(vevent.getLocation(),testlocation) == 0 &&
      strcmp(vevent.getSummary(),testsummary) == 0)
    std::cout << "ICVevent test  passed\n";
  else
    std::cout << "ICVevent test  FAILED\n";
  #ifdef WFCLIENTDMO
  // Client/WiFiClient demo run
  WiFi::begin("SSID");
  HTTPClient http;
  WiFiClient *stream = http.getStreamPtr();
  ICalClientParser icc_parser;
  ICVevent *icvev;
  time_t tmptime2;
  icc_parser.begin(stream);
  std::cout << "Now simulating network interaction ...\n";
  while((icvev = icc_parser.getNext())!=NULL){
    std::cout << "summary:" << icvev->getSummary() << "\n";
    std::cout << "location:" << icvev->getLocation() << "\n";
    tmptime2 = icvev->getDtstart();
    std::cout << "dtstart:" << asctime(localtime(&tmptime2));
    tmptime2 = icvev->getDtend();
    std::cout << "dtend:" << asctime(localtime(&tmptime2)) << "\n";
  }
  #endif
  #ifdef ICALBUFPARSDMO
  // ICalBufferParser demo run
  ICalBufferParser icparser = ICalBufferParser();
  char *icsbuf = _file2mem(ICS_FILE);
  icparser.begin(icsbuf);
  ICVevent *icobj;
  time_t tmptime;
  std::cout << "Now parsing " << ICS_FILE << " ...\n";
  while((icobj = icparser.getNext())!=NULL){
    std::cout << "summary:" << icobj->getSummary() << "\n";
    std::cout << "location:" << icobj->getLocation() << "\n";
    tmptime = icobj->getDtstart();
    std::cout << "dtstart:" << asctime(localtime(&tmptime));
    tmptime = icobj->getDtend();
    std::cout << "dtend:" << asctime(localtime(&tmptime)) << "\n";
  }
  free(icsbuf);
  #endif
  #ifndef ICALBUFPARSDMO
  #ifndef WFCLIENTDMO
  WiFi::begin("SSID");
  time_t prev_midnight = ICDate::setFromICString("20180122T000000Z");
  time_t next_midnight = ICDate::setFromICString("20180123T000000Z");
  ICVevent icvevs[8];
  char onlineresource[] = "http://example.com/example.ics";
  char ourlocation[] = "Salle Télécom. 1";
  int eventsnum =
  CalCo::events4loc_from_url(onlineresource,ourlocation, prev_midnight, next_midnight, icvevs, c_array_len(icvevs));
  std::cout << "found " << eventsnum << " event(s) between " << ctime(&prev_midnight) <<  " and " << ctime(&next_midnight) << " in " << ourlocation << "\n";
  for (int i=0; i<=eventsnum-1; i++){
    std::cout << icvevs[i].getSummary() << " in " << icvevs[i].getLocation() << "\n";
  }
  #else
  #warning "events filter testing disabled because of demo(s) !"
  #endif
  #else
  #warning "events filter testing disabled because of demo(s) !"
  #endif
  // testing LedMng features
  // init by constructor
  CourseSlot ledmng_test_slots[] = {
    CourseSlot(8,0,    10,0), // course slots expressed in localtime
    CourseSlot(10,15,  12,15),
    CourseSlot(13,45,  15,45),
    CourseSlot(16,0,   18,0)
  };
  ICVevent ledmng_vevents[4]; // no init by constructor (Arduino-style, why do I follow this ?)
  // sorry for this very long initialisation
  ledmng_vevents[0].setDtstart(ICDate::setFromICString("20181130T070000Z"));
  ledmng_vevents[0].setDtend(  ICDate::setFromICString("20181130T090000Z"));
  ledmng_vevents[0].setLocation("TD0");
  ledmng_vevents[0].setSummary("Summary1");
  ledmng_vevents[1].setDtstart(ICDate::setFromICString("20181130T091500Z"));
  ledmng_vevents[1].setDtend(  ICDate::setFromICString("20181130T111500Z"));
  ledmng_vevents[1].setLocation("TD0");
  ledmng_vevents[1].setSummary("Summary2");
  ledmng_vevents[2].setDtstart(ICDate::setFromICString("20181130T124500Z"));
  ledmng_vevents[2].setDtend(  ICDate::setFromICString("20181130T144500Z"));
  ledmng_vevents[2].setLocation("TD9");
  ledmng_vevents[2].setSummary("Summary3");
  // shouldn't fit, out of range
  ledmng_vevents[3].setDtstart(ICDate::setFromICString("20181130T190000Z"));
  ledmng_vevents[3].setDtend(  ICDate::setFromICString("20181130T200000Z"));
  ledmng_vevents[3].setLocation("TD0");
  ledmng_vevents[3].setSummary("Summary4");
  time_t ledmng_now = ICDate::setFromICString("20181130T093253Z");
  struct tm *ledmng_utcnow = gmtime(&ledmng_now);
  /* doesn't care about locations, cares only about time */
  cslots_set (ledmng_test_slots, c_array_len(ledmng_test_slots), ledmng_vevents, c_array_len(ledmng_vevents), ledmng_utcnow, 1);
  if (ledmng_test_slots[0].whichState() == CourseState::PLANNED &&
      ledmng_test_slots[1].whichState() == CourseState::PLANNED &&
      ledmng_test_slots[2].whichState() == CourseState::PLANNED &&
      ledmng_test_slots[3].whichState() == CourseState::NOCOURSE_NOACT)
    std::cout << "LedMng test passed\n";
  else
    std::cout << "LedMng test FAILED\n";
}
