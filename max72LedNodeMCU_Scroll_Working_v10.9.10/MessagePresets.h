#pragma once

#include <stddef.h>

struct MessagePreset {
  const char* name;
  const char* const* dates;
  const char* const* messages;
  size_t count;
};

// Family preset (original message list).
static const char* const kFamilyDates[] = {
  "Jan  1", "Jan 7", "Jan 17", "Jan 25", "Feb 12", "Feb 14", "Apr  1", "Apr  15", "May 12", "May 22",
  "May 28", "Jun 2", "Jun 7", "Jun 12", "Jun 20", "Jun 30", "Jul 17", "Jul 27", "Aug 18", "Aug 28",
  "Oct 31", "Nov 2", "Nov 12", "Nov 29", "Dec  9", "Dec 15", "Dec 24", "Dec 25", "Dec 26", "Dec 31"
};

static const char* const kFamilyMessages[] = {
  "New Years Day", "10 days to Manuela's Bday", "Happy Birthday Manuela", "Burns Night", "Chinese New Year",
  "Valentines Day", "April Fools Day", "Good Friday", "10 days to Dylan's Bday", "Happy Birthday Dylan",
  "10 days to Maureen's Bday", "10 days to Mam's Bday", "Happy Birthday Maureen", "Happy Birthday Mam",
  "10 days to Mel's Bday", "Happy Birthday Mel", "10 days to Yvonne's Bday", "Happy Birthday Yvonne",
  "10 days to Dad's Bday", "Happy Birthday Dad", "Happy Halloween", "10 days to Harrys's Bday",
  "Happy Birthday Harry", "10 days to Robert's Bday", "Happy Birthday Robert", "10 days to Xmas",
  "Christmas Eve", "Christmas Day", "Boxing Day", "New Years Eve"
};

static const char* const kSeasonalDates[] = {
  "Jan  1", "Feb 14", "Mar 20", "Apr  1", "Jun 21", "Oct 31", "Nov  5", "Dec 24", "Dec 25", "Dec 31"
};

static const char* const kSeasonalMessages[] = {
  "Happy New Year", "Happy Valentines Day", "Spring is here", "April Fools Day", "Summer Solstice",
  "Happy Halloween", "Bonfire Night", "Christmas Eve", "Merry Christmas", "New Years Eve"
};

static const char* const kDailyDates[] = {
  "Jan  1", "Feb  1", "Mar  1", "Apr  1", "May  1", "Jun  1", "Jul  1", "Aug  1", "Sep  1", "Oct  1",
  "Nov  1", "Dec  1"
};

static const char* const kDailyMessages[] = {
  "Make today awesome", "Keep moving forward", "Small steps count", "Choose joy today", "Stay curious",
  "Be kind", "Keep it simple", "Enjoy the sunshine", "Focus on what matters", "Do the next right thing",
  "You are capable", "Finish strong"
};

static const MessagePreset kMessagePresets[] = {
  { "family", kFamilyDates, kFamilyMessages, sizeof(kFamilyDates) / sizeof(kFamilyDates[0]) },
  { "seasonal", kSeasonalDates, kSeasonalMessages, sizeof(kSeasonalDates) / sizeof(kSeasonalDates[0]) },
  { "daily", kDailyDates, kDailyMessages, sizeof(kDailyDates) / sizeof(kDailyDates[0]) }
};

static const size_t kMessagePresetCount = sizeof(kMessagePresets) / sizeof(kMessagePresets[0]);
