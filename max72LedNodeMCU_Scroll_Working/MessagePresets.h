#pragma once

#include <stddef.h>

struct MessageEntry {
  const char* date;
  const char* message;
};

struct MessagePreset {
  const char* name;
  const MessageEntry* entries;
  size_t count;
};

// Family preset (original message list).
static const MessageEntry kFamilyEntries[] = {
  { "Jan  1", "New Years Day" },
  { "Jan 7", "10 days to Manuela's Bday" },
  { "Jan 17", "Happy Birthday Manuela" },
  { "Jan 25", "Burns Night" },
  { "Feb 12", "Chinese New Year" },
  { "Feb 14", "Valentines Day" },
  { "Apr  1", "April Fools Day" },
  { "Apr  15", "Good Friday" },
  { "May 12", "10 days to Dylan's Bday" },
  { "May 22", "Happy Birthday Dylan" },
  { "May 28", "10 days to Maureen's Bday" },
  { "Jun 2", "10 days to Mam's Bday" },
  { "Jun 7", "Happy Birthday Maureen" },
  { "Jun 12", "Happy Birthday Mam" },
  { "Jun 20", "10 days to Mel's Bday" },
  { "Jun 30", "Happy Birthday Mel" },
  { "Jul 17", "10 days to Yvonne's Bday" },
  { "Jul 27", "Happy Birthday Yvonne" },
  { "Aug 18", "10 days to Dad's Bday" },
  { "Aug 28", "Happy Birthday Dad" },
  { "Oct 31", "Happy Halloween" },
  { "Nov 2", "10 days to Harrys's Bday" },
  { "Nov 12", "Happy Birthday Harry" },
  { "Nov 29", "10 days to Robert's Bday" },
  { "Dec  9", "Happy Birthday Robert" },
  { "Dec 15", "10 days to Xmas" },
  { "Dec 24", "Christmas Eve" },
  { "Dec 25", "Christmas Day" },
  { "Dec 26", "Boxing Day" },
  { "Dec 31", "New Years Eve" }
};

static const MessageEntry kSeasonalEntries[] = {
  { "Jan  1", "Happy New Year" },
  { "Feb 14", "Happy Valentines Day" },
  { "Mar 20", "Spring is here" },
  { "Apr  1", "April Fools Day" },
  { "Jun 21", "Summer Solstice" },
  { "Oct 31", "Happy Halloween" },
  { "Nov  5", "Bonfire Night" },
  { "Dec 24", "Christmas Eve" },
  { "Dec 25", "Merry Christmas" },
  { "Dec 31", "New Years Eve" }
};

static const MessageEntry kDailyEntries[] = {
  { "Jan  1", "Make today awesome" },
  { "Feb  1", "Keep moving forward" },
  { "Mar  1", "Small steps count" },
  { "Apr  1", "Choose joy today" },
  { "May  1", "Stay curious" },
  { "Jun  1", "Be kind" },
  { "Jul  1", "Keep it simple" },
  { "Aug  1", "Enjoy the sunshine" },
  { "Sep  1", "Focus on what matters" },
  { "Oct  1", "Do the next right thing" },
  { "Nov  1", "You are capable" },
  { "Dec  1", "Finish strong" }
};

static const MessagePreset kMessagePresets[] = {
  { "family", kFamilyEntries, sizeof(kFamilyEntries) / sizeof(kFamilyEntries[0]) },
  { "seasonal", kSeasonalEntries, sizeof(kSeasonalEntries) / sizeof(kSeasonalEntries[0]) },
  { "daily", kDailyEntries, sizeof(kDailyEntries) / sizeof(kDailyEntries[0]) }
};

static const size_t kMessagePresetCount = sizeof(kMessagePresets) / sizeof(kMessagePresets[0]);
